file(GLOB xptilib_src "*.cpp")

add_executable(kernel_launcher main.cpp)
target_include_directories(kernel_launcher PRIVATE "${PROJECT_SOURCE_DIR}/external")

add_library(kernel_collector SHARED ${xptilib_src})
target_include_directories(kernel_collector PRIVATE "${PROJECT_SOURCE_DIR}/external")
target_compile_definitions(kernel_collector PRIVATE XPTI_CALLBACK_API_EXPORTS)
target_compile_definitions(kernel_collector PRIVATE TRACE)

target_link_libraries(kernel_collector PRIVATE xpti)