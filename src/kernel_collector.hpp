#pragma once
#include "xpti/xpti_trace_framework.h"

#include <assert.h>
#include <iostream>
#include <string>
#include <ur_api.h>
#include <vector>

using handler_t = uintptr_t;

struct Kernel {
  Kernel(std::string name_, uintptr_t kernelRef_, uintptr_t eventRef_)
      : name(name_), kernelRef(kernelRef_), eventRef(eventRef_) {}
  std::string name;
  uintptr_t kernelRef;
  uintptr_t eventRef;

  bool launched = false;
  bool finished = false;
};

class KernelCollector {
public:
  KernelCollector() {}

  void handle_urKernelCreate(const ur_kernel_create_params_t *Params);
  void
  handle_urEnqueueKernelLaunch(const ur_enqueue_kernel_launch_params_t *Params);
  void handle_urEventWait(const ur_event_wait_params_t *Params);
  void
  handle_urEnqueueEventsWait(const ur_enqueue_events_wait_params_t *Params);
  void handle_urQueueFinish(const ur_queue_finish_params_t *Params);

  void printKernel();
  void kernelLaunch(Kernel &k) {
    k.launched = true;
    std::cout << k.name << " [launched]\n";
  }
  void kernelFinish(Kernel &k) {
    k.finished = true;
    std::cout << k.name << " [finished]\n";
  }

  void clear();

  static KernelCollector *&getInstancePtr() {
    static KernelCollector *KernelCollectorPtr = new KernelCollector();
    return KernelCollectorPtr;
  }
  static KernelCollector &getInstance() {
    KernelCollector *KernelCollectorPtr = KernelCollector::getInstancePtr();
    assert(KernelCollectorPtr && "Instance must not be deallocated earlier");
    return *KernelCollectorPtr;
  }

private:
  std::unordered_map<handler_t, Kernel> kernelMap;
  std::unordered_map<handler_t, std::vector<handler_t>> queueMap;
};