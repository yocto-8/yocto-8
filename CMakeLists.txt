cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(yocto-8)

pico_sdk_init()

add_executable(yocto-8
    src/main.cpp
)

target_include_directories(yocto-8 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lua")
target_link_libraries(yocto-8 pico_stdlib picolua)

# FIXME: this is a horrible workaround for clangd in vscode
set(CLANGD_WORKAROUND_FLAGS "-isystem /usr/arm-none-eabi/include/c++/11.1.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/11.1.0/ -isystem /usr/arm-none-eabi/include/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")

pico_add_extra_outputs(yocto-8)

# Do not relocate if possible - this should be after the C/C++ flag changes
# See https://stackoverflow.com/a/33834879/6261331 - add_subdirectory clones the current set of variables
add_subdirectory(lua)