#include "kernel_collector.hpp"
#include "pi_arguments_handler.hpp"
#include "xpti/xpti_trace_framework.h"

// Demangle
#ifdef __has_include
#if __has_include(<cxxabi.h>)
#define __SYCL_ENABLE_GNU_DEMANGLING
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif
#endif

#ifdef __SYCL_ENABLE_GNU_DEMANGLING
struct DemangleHandle {
  char *p;
  DemangleHandle(char *ptr) : p(ptr) {}
  ~DemangleHandle() { std::free(p); }
};
static std::string demangleKernelName(std::string Name) {
  int Status = -1; // some arbitrary value to eliminate the compiler warning
  DemangleHandle result(abi::__cxa_demangle(Name.c_str(), NULL, NULL, &Status));
  return (Status == 0) ? result.p : Name;
}
#else
static std::string demangleKernelName(std::string Name) { return Name; }
#endif

static std::string getResult(pi_result Res) {
  switch (Res) {
#define _PI_ERRC(NAME, VAL)                                                    \
  case NAME:                                                                   \
    return #NAME;
#define _PI_ERRC_WITH_MSG(NAME, VAL, MSG) _PI_ERRC(NAME, VAL)
#include <sycl/detail/pi_error.def>
#undef _PI_ERRC
#undef _PI_ERRC_WITH_MSG
  }

  return "UNKNOWN RESULT";
}

void KernelCollector::setupPiHandler() {
  // Register kernels
  argEndHandler.set_piKernelCreate(
      [&](const pi_plugin &, std::optional<pi_result> Res, pi_program program,
          const char *kernel_name, pi_kernel *ret_kernel) {
        auto demangleName = demangleKernelName(std::string(kernel_name));
        auto kernelRef = reinterpret_cast<uintptr_t>(*ret_kernel);
        kernelMap.emplace(kernelRef, Kernel(demangleName, kernelRef, 0));

        std::cout << ">>>>> Create kernel: " << demangleName << std::endl;
        std::cout << "<<<<< Creating finish with " << getResult(Res.value())
                  << std::endl;
      });
  // Register interop kernels
  argEndHandler.set_piextKernelCreateWithNativeHandle(
      [&](const pi_plugin &, std::optional<pi_result> Res,
          pi_native_handle NativeHandle, pi_context Context, pi_program Program,
          bool OwnNativeHandle, pi_kernel *ret_kernel) {
        auto kernelRef = reinterpret_cast<uintptr_t>(*ret_kernel);
        // TODO: use hex string?
        std::string name =
            "unknown interop kernel " + std::to_string(kernelRef);
        kernelMap.emplace(kernelRef, Kernel(name, kernelRef, 0));

        std::cout << ">>>>> Create kernel: " << name << std::endl;
        std::cout << "<<<<< Creating finish with " << getResult(Res.value())
                  << std::endl;
      });
  // Associate kernels with events and queue
  argBeginHandler.set_piEnqueueKernelLaunch(
      [&](const pi_plugin &, std::optional<pi_result>, pi_queue Queue,
          pi_kernel Kernel, pi_uint32, const size_t *, const size_t *,
          const size_t *, pi_uint32, const pi_event *, pi_event *OutEvent) {
        auto kernelRef = reinterpret_cast<uintptr_t>(Kernel);
        auto eventRef = reinterpret_cast<uintptr_t>(*OutEvent);
        if (kernelMap.find(kernelRef) != kernelMap.end()) {
          kernelMap.at(kernelRef).eventRef = eventRef;
          std::cout << ">>>>> Launch kernel: " << kernelMap.at(kernelRef).name
                    << std::endl;
        } else
          std::cout << ">>>>> Launch unknown kernel\n";
      });
  argEndHandler.set_piEnqueueKernelLaunch(
      [&](const pi_plugin &, std::optional<pi_result> Res, pi_queue,
          pi_kernel Kernel, pi_uint32, const size_t *, const size_t *,
          const size_t *, pi_uint32, const pi_event *, pi_event *) {
        auto kernelRef = reinterpret_cast<uintptr_t>(Kernel);
        if (kernelMap.find(kernelRef) != kernelMap.end()) {
          kernelMap.at(kernelRef).launched = true;
          std::cout << "<<<<< Launching finish with " << getResult(Res.value())
                    << std::endl;
        }
      });

  // TODO: query pi_event.CleanedUp or pi_event.Completed
}

void KernelCollector::handlePiBegin(const pi_plugin &Plugin,
                                    const xpti::function_with_args_t *Data) {
  argBeginHandler.handle(Data->function_id, Plugin, std::nullopt,
                         Data->args_data);
}

void KernelCollector::handlePiEnd(const pi_plugin &Plugin,
                                  const xpti::function_with_args_t *Data) {
  auto Res = *static_cast<pi_result *>(Data->ret_data);
  argEndHandler.handle(Data->function_id, Plugin, Res, Data->args_data);
}

void KernelCollector::printKernel() {
  std::cout << "----------------------------------------------------------\n";
  std::cout << "check launched kernels\n";
  std::cout << "----------------------------------------------------------\n";
  for (const auto &[p, k] : kernelMap) {
    // Kernel k = pair.second;
    if (k.launched)
      std::cout << k.name << "    launched\n";
    else
      std::cout << k.name << "    not launched\n";
  }
}

void KernelCollector::clear() {}