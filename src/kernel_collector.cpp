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
  argHandler.set_piKernelCreate([&](const pi_plugin &, std::optional<pi_result>,
                                    pi_program program, const char *kernel_name,
                                    pi_kernel *ret_kernel) {
    auto demangleName = demangleKernelName(std::string(kernel_name));
    auto kernelRef = reinterpret_cast<uintptr_t>(*ret_kernel);
    kernelMap.emplace(kernelRef, Kernel(demangleName, kernelRef, 0));

    std::cout << ">>>>> Create kernel: " << demangleName << std::endl;
  });
  // Associate kernels with events and queue
  argHandler.set_piEnqueueKernelLaunch(
      [&](const pi_plugin &, std::optional<pi_result>, pi_queue Queue,
          pi_kernel Kernel, pi_uint32, const size_t *, const size_t *,
          const size_t *, pi_uint32, const pi_event *, pi_event *OutEvent) {
        auto kernelRef = reinterpret_cast<uintptr_t>(Kernel);
        auto eventRef = reinterpret_cast<uintptr_t>(*OutEvent);

        kernelMap.at(kernelRef).eventRef = eventRef;
        std::cout << ">>>>> Launch kernel: " << kernelMap.at(kernelRef).name
                  << std::endl;
      });
  /*
    // How many events will be executed?
    // Or query pi_event.CleanedUp or pi_event.Completed
    argHandler.set_piEventsWait([&](const pi_plugin &, std::optional<pi_result>,
                                    pi_uint32 num_events,
                                    const pi_event *event_list) {

    });
    // Clear all kernels associate with this queue
    argHandler.set_piQueueFinish(
        [&](const pi_plugin &, std::optional<pi_result>, pi_queue Queue) {

        });
  */
}

void KernelCollector::handlePiBegin(const pi_plugin &Plugin,
                                    const xpti::function_with_args_t *Data) {
  argHandler.handle(Data->function_id, Plugin, std::nullopt, Data->args_data);
}

void KernelCollector::handlePiEnd(pi_result Res) {
  std::cout << "<<<<< Finish with " << getResult(Res) << std::endl;
}