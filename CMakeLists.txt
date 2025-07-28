cmake_minimum_required(VERSION 3.15)

# Set SYCL compiler and options
find_program(DPCPP_CXX_EXECUTABLE NAMES icpx clang++ dpcpp
    PATH_SUFFIXES bin)
message(${DPCPP_CXX_EXECUTABLE})
set(CMAKE_CXX_COMPILER ${DPCPP_CXX_EXECUTABLE})
set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations")
project(kernel_collector LANGUAGES CXX)


add_subdirectory(src)
add_subdirectory(test)
