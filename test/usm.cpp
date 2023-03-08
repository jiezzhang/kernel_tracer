#include <array>
#include <iostream>
#include <sycl/sycl.hpp>

#if FPGA || FPGA_EMULATOR
#include <sycl/ext/intel/fpga_extensions.hpp>
#endif

using namespace sycl;
using namespace std;

static auto exception_handler = [](sycl::exception_list e_list) {
  for (std::exception_ptr const &e : e_list) {
    try {
      std::rethrow_exception(e);
    } catch (std::exception const &e) {
#if _DEBUG
      std::cout << "Failure" << std::endl;
#endif
      std::terminate();
    }
  }
};

constexpr size_t array_size = 10000;
void IotaParallel(queue &q, int *a, size_t size, int value) {
  range num_items{size};
  auto e = q.parallel_for(num_items, [=](auto i) { a[i] = value + i; });
  q.parallel_for<class aaa>(num_items, [=](auto i) { a[i] = value + i; });
  q.parallel_for<class bbb>(num_items, [=](auto i) { a[i] = value + i; });
  e.wait();
}

int main() {
  auto selector{default_selector_v};

  constexpr int value = 100000;

  try {
    queue q(selector, exception_handler);
    int *sequential = malloc_shared<int>(array_size, q);
    int *parallel = malloc_shared<int>(array_size, q);

    if ((sequential == nullptr) || (parallel == nullptr)) {
      if (sequential != nullptr)
        free(sequential, q);
      if (parallel != nullptr)
        free(parallel, q);

      return -1;
    }

    // Sequential iota.
    for (size_t i = 0; i < array_size; i++)
      sequential[i] = value + i;

    // Parallel iota in SYCL.
    IotaParallel(q, parallel, array_size, value);

    // Verify two results are equal.
    for (size_t i = 0; i < array_size; i++) {
      if (parallel[i] != sequential[i]) {
        cout << "Failed on device.\n";
        return -1;
      }
    }

    int indices[]{0, 1, 2, (array_size - 1)};
    constexpr size_t indices_size = sizeof(indices) / sizeof(int);

    // Print out iota result.
    for (int i = 0; i < indices_size; i++) {
      int j = indices[i];
      if (i == indices_size - 1)
        cout << "...\n";
      cout << "[" << j << "]: " << j << " + " << value << " = " << sequential[j]
           << "\n";
    }

    free(sequential, q);
    free(parallel, q);
  } catch (std::exception const &e) {
    cout << "An exception is caught while computing on device.\n";
    terminate();
  }

  cout << "Successfully completed on device.\n";
  return 0;
}