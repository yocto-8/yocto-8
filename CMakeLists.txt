cmake_minimum_required(VERSION 3.13)

#set(Y8_DESKTOP_VM ON)

if (Y8_DESKTOP_VM)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
else()
    include(vendor/pico-sdk/pico_sdk_init.cmake)

    # FIXME: this is a horrible workaround for clangd in vscode
    set(CLANGD_WORKAROUND_FLAGS "-isystem /usr/arm-none-eabi/include/c++/11.2.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/11.2.0/ -isystem /usr/arm-none-eabi/include/")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
endif()

project(yocto-8)

add_compile_options("-fno-exceptions" $<$<CONFIG:RELWITHDEBINFO>:-Ofast>)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fno-rtti")

add_library(yocto-8-core
    src/devices/image.cpp
    src/emu/emulator.cpp
    src/emu/bindings/input.cpp
    src/emu/bindings/math.cpp
    src/emu/bindings/mmio.cpp
    src/emu/bindings/rng.cpp
    src/emu/bindings/time.cpp
    src/emu/bindings/video.cpp
    src/p8/parser.cpp
    vendor/tinyalloc/tinyalloc.c
)

target_compile_options(yocto-8-core
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wno-ignored-qualifiers>
)

set_property(TARGET yocto-8-core PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)

target_include_directories(yocto-8-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyalloc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GSL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(yocto-8-core picolua libfixmath)

if (Y8_DESKTOP_VM)
    add_subdirectory(src/arch/desktop)
else()
    add_subdirectory(src/arch/pico)
endif()

#pico_enable_stdio_usb(yocto-8 1)

# Do not relocate if possible - this should be after the C/C++ flag changes
# See https://stackoverflow.com/a/33834879/6261331 - add_subdirectory clones the current set of variables
add_subdirectory(vendor/libfixmath)
add_subdirectory(vendor/lua)