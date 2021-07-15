cmake_minimum_required(VERSION 3.13)

set(Y8_DESKTOP_VM ON)

if(NOT Y8_DESKTOP_VM)
    include(pico-sdk/pico_sdk_init.cmake)
endif()

project(yocto-8)

add_library(yocto-8-core
    src/emu/emulator.cpp
    src/p8/parser.cpp
    vendor/tinyalloc/tinyalloc.c
)

target_compile_options(yocto-8-core
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -Wall -Wextra -Wpedantic>)

target_include_directories(yocto-8-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyalloc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GSL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(yocto-8-core picolua)

if (Y8_DESKTOP_VM)
    add_subdirectory(src/arch/desktop)
else()
    add_subdirectory(src/arch/pico)
endif()

#pico_enable_stdio_usb(yocto-8 1)

# Do not relocate if possible - this should be after the C/C++ flag changes
# See https://stackoverflow.com/a/33834879/6261331 - add_subdirectory clones the current set of variables
add_subdirectory(vendor/lua)