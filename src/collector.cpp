#include "kernel_collector.hpp"
#include "xpti/xpti_trace_framework.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>

std::mutex GMutex;
KernelCollector *collector = nullptr;

XPTI_CALLBACK_API void urCallback(uint16_t, xpti::trace_event_data_t *,
                                  xpti::trace_event_data_t *,
                                  uint64_t /*Instance*/, const void *);

XPTI_CALLBACK_API
void xptiTraceInit(unsigned int MajorVersion, unsigned int MinorVersion,
                   const char *VersionStr, const char *StreamName) {
  std::string_view NameView{StreamName};
  if (std::string_view(StreamName) == "ur.call") {
    uint8_t StreamID = xptiRegisterStream(StreamName);
    // Must register args_begin because https://github.com/intel/llvm/pull/11651
    xptiRegisterCallback(StreamID, xpti::trace_function_with_args_begin,
                         urCallback);
    xptiRegisterCallback(StreamID, xpti::trace_function_with_args_end,
                         urCallback);
  }
}

XPTI_CALLBACK_API void xptiTraceFinish(const char *StreamName) {
  std::string_view NameView{StreamName};
}

XPTI_CALLBACK_API void urCallback(uint16_t TraceType,
                                  xpti::trace_event_data_t *,
                                  xpti::trace_event_data_t *,
                                  uint64_t /*Instance*/, const void *UserData) {
  auto &G = KernelCollector::getInstance();
  const auto *Data = static_cast<const xpti::function_with_args_t *>(UserData);

  if (TraceType == xpti::trace_function_with_args_begin) {

  } else if (TraceType == xpti::trace_function_with_args_end) {
    switch (static_cast<ur_function_t>(Data->function_id)) {
    case UR_FUNCTION_KERNEL_CREATE:
      G.handle_urKernelCreate(static_cast<ur_kernel_create_params_t *>(Data->args_data));
      return;
    // ur_kernel_handle_t: ZeCache<std::string> ZeKernelName;
    // case UR_FUNCTION_KERNEL_CREATE_WITH_NATIVE_HANDLE:

    case UR_FUNCTION_ENQUEUE_KERNEL_LAUNCH:
      G.handle_urEnqueueKernelLaunch(static_cast<ur_enqueue_kernel_launch_params_t *>(Data->args_data));
      return;
    case UR_FUNCTION_EVENT_WAIT:
      G.handle_urEventWait(static_cast<ur_event_wait_params_t *>(Data->args_data));
      return;
    case UR_FUNCTION_QUEUE_FINISH:
      G.handle_urQueueFinish(static_cast<ur_queue_finish_params_t *>(Data->args_data));

    // case UR_FUNCTION_ENQUEUE_EVENTS_WAIT:
    //   G.handle_urEnqueueEventsWait(static_cast<ur_enqueue_events_wait_params_t *>(Data->args_data));
    //   return;
    // case UR_FUNCTION_ENQUEUE_EVENTS_WAIT_WITH_BARRIER:
    // case UR_FUNCTION_ENQUEUE_EVENTS_WAIT_WITH_BARRIER_EXT:

    default:
      return;
    }
  }
}