cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(yocto-8)

pico_sdk_init()

add_executable(yocto-8
    src/main.cpp
)

target_link_libraries(yocto-8 pico_stdlib)

# FIXME: this is a horrible workaround for clangd in vscode
set(CMAKE_CXX_FLAGS "-isystem /usr/arm-none-eabi/include/c++/11.1.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/11.1.0/ -isystem /usr/arm-none-eabi/include/")

pico_add_extra_outputs(yocto-8)