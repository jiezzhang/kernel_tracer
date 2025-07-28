file(GLOB xptilib_src "*.cpp")
set(CMAKE_CXX_FLAGS "-fsycl -Wno-deprecated-declarations")

add_executable(usm usm.cpp)