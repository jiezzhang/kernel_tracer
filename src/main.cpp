#include "launch.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv, char *env[]) {
  if (argc < 2)
    std::cerr
        << "usage: kernel_launcher <target executable> <program arguments>..."
        << std::endl;

  std::string Executable = std::string(argv[1]);
  std::vector<std::string> Args;
  for (size_t i = 1; i < argc; ++i) {
    Args.emplace_back(argv[i]);
  }

  std::vector<std::string> NewEnv;
  {
    size_t I = 0;
    while (env[I] != nullptr)
      NewEnv.push_back(env[I++]);
  }
  NewEnv.push_back("XPTI_FRAMEWORK_DISPATCHER=libxptifw.so");
  NewEnv.push_back("XPTI_SUBSCRIBERS=libkernel_collector.so");
  NewEnv.push_back("XPTI_TRACE_ENABLE=1");

  int Err = launch(Executable, Args, NewEnv);

  if (Err) {
    std::cerr << "Failed to launch target application. Error code " << Err
              << "\n";
    return Err;
  }

  return 0;
}
