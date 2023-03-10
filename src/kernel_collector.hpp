#pragma once
#include "pi_arguments_handler.hpp"
#include "xpti/xpti_trace_framework.h"

#include <iostream>
#include <string>
#include <vector>

using kernelptr_t = uintptr_t;

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
  void setupPiHandler();
  void handlePiBegin(const pi_plugin &Plugin,
                     const xpti::function_with_args_t *Data);
  void handlePiEnd(const pi_plugin &Plugin,
                   const xpti::function_with_args_t *Data);
  void printKernel();
  void clear();

private:
  sycl::xpti_helpers::PiArgumentsHandler argBeginHandler;
  sycl::xpti_helpers::PiArgumentsHandler argEndHandler;

  std::unordered_map<kernelptr_t, Kernel> kernelMap;
};