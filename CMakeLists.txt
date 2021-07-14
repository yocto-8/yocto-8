cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(yocto-8)

add_library(yocto-8-core
    tinyalloc/tinyalloc.c
)

target_compile_options(yocto-8-core PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)

target_include_directories(yocto-8-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyalloc"
    "${CMAKE_CURRENT_SOURCE_DIR}/GSL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(yocto-8-core picolua)

add_subdirectory(src/arch/pico)

#pico_enable_stdio_usb(yocto-8 1)

# Do not relocate if possible - this should be after the C/C++ flag changes
# See https://stackoverflow.com/a/33834879/6261331 - add_subdirectory clones the current set of variables
add_subdirectory(lua)