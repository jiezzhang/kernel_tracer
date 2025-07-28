#include "kernel_collector.hpp"
#include "xpti/xpti_trace_framework.h"

void KernelCollector::handle_urKernelCreate(
    const ur_kernel_create_params_t *Params) {
  handler_t kernelRef = reinterpret_cast<uintptr_t>(**Params->pphKernel);
  std::string name = *(Params->ppKernelName);
  kernelMap.emplace(kernelRef, Kernel(name, kernelRef, 0));
}

void KernelCollector::handle_urEnqueueKernelLaunch(
    const ur_enqueue_kernel_launch_params_t *Params) {
  handler_t queueRef = reinterpret_cast<uintptr_t>(*Params->phQueue);
  handler_t kernelRef = reinterpret_cast<uintptr_t>(*Params->phKernel);
  handler_t eventRef = reinterpret_cast<uintptr_t>(**Params->pphEvent);
  assert(kernelMap.find(kernelRef) != kernelMap.end());
  kernelMap.at(kernelRef).eventRef = eventRef;
  kernelLaunch(kernelMap.at(kernelRef));
  if (queueMap.find(queueRef) == queueMap.end())
    queueMap[queueRef] = {kernelRef};
  else
    queueMap.at(queueRef).push_back(kernelRef);
}

void KernelCollector::handle_urEventWait(const ur_event_wait_params_t *Params) {
  handler_t eventRef = reinterpret_cast<uintptr_t>(**Params->pphEventWaitList);
  for (auto &[h, k] : kernelMap) {
    if (k.eventRef == eventRef)
      kernelFinish(k);
  }
  // TODO: throw error for unexisting?
}

void KernelCollector::handle_urEnqueueEventsWait(
    const ur_enqueue_events_wait_params_t *Params) {
  // handler_t queueRef = Params->phQueue;
  // if (queueMap.find(queueRef) != queueMap.end()) {
  //   for (auto h: queueMap.at(queueRef)) {
  //     assert(kernelMap.find(h) != kernelMap.end());
  //     kernelMap.at(h).finished = true;
  //   }
  // }
}

void KernelCollector::handle_urQueueFinish(
    const ur_queue_finish_params_t *Params) {
  handler_t queueRef = reinterpret_cast<uintptr_t>(*Params->phQueue);
  if (queueMap.find(queueRef) != queueMap.end()) {
    auto handlers = queueMap.at(queueRef);
    for (auto h : handlers) {
      assert(kernelMap.find(h) != kernelMap.end());
      kernelFinish(kernelMap.at(h));
    }
  }
}

void KernelCollector::printKernel() {}

void KernelCollector::clear() {}