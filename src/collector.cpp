#include "kernel_collector.hpp"
#include "xpti/xpti_trace_framework.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>

std::mutex GMutex;
KernelCollector *collector = nullptr;

XPTI_CALLBACK_API void piCallback(uint16_t, xpti::trace_event_data_t *,
                                  xpti::trace_event_data_t *,
                                  uint64_t /*Instance*/, const void *);

XPTI_CALLBACK_API
void xptiTraceInit(unsigned int MajorVersion, unsigned int MinorVersion,
                   const char *VersionStr, const char *StreamName) {
  std::string_view NameView{StreamName};

  if (NameView == "sycl.pi.debug") {
    collector = new KernelCollector;
    collector->setupPiHandler();
    uint8_t StreamID = xptiRegisterStream(StreamName);
    // Register PI and XPTI callbacks
    xptiRegisterCallback(StreamID, xpti::trace_function_with_args_begin,
                         piCallback);
    xptiRegisterCallback(StreamID, xpti::trace_function_with_args_end,
                         piCallback);
  }
}

XPTI_CALLBACK_API void xptiTraceFinish(const char *StreamName) {
  std::string_view NameView{StreamName};

  if (NameView == "sycl.pi.debug") {
    collector->printKernel();
    delete collector;
  }
}

XPTI_CALLBACK_API void piCallback(uint16_t TraceType,
                                  xpti::trace_event_data_t *,
                                  xpti::trace_event_data_t *,
                                  uint64_t /*Instance*/, const void *UserData) {
  // TODO: move mutex into global handler

  auto *Payload = xptiQueryPayloadByUID(xptiGetUniversalId());
  std::lock_guard<std::mutex> Lock(GMutex);
  const auto *Data = static_cast<const xpti::function_with_args_t *>(UserData);
  const auto *Plugin = static_cast<pi_plugin *>(Data->user_data);
  if (TraceType == xpti::trace_function_with_args_begin) {
    collector->handlePiBegin(*Plugin, Data);
  } else if (TraceType == xpti::trace_function_with_args_end) {
    collector->handlePiEnd(*Plugin, Data);
  }
}