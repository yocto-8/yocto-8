cmake_minimum_required(VERSION 3.13)

# FIXME: This is currently broken when using the secondary memory pool because OOM situations in Lua are turned into UB
#        Maybe this should be scrapped altogether
# set(YOCTO8_YOLO_RELEASE OFF CACHE BOOL "Enables highly aggressive optimizations that turn some error conditions into undefined behavior")

set(Y8_DESKTOP_VM OFF CACHE BOOL "Makes a desktop build instead of a build for the Raspberry Pi Pico")

if (Y8_DESKTOP_VM)
    enable_testing()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
else()
    include(vendor/pico-sdk/pico_sdk_init.cmake)

    # FIXME: this is a horrible workaround for clangd in vscode
    set(CLANGD_WORKAROUND_FLAGS "-isystem /usr/arm-none-eabi/include/c++/11.2.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/11.2.0/ -isystem /usr/arm-none-eabi/include/")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")

    add_compile_definitions(PICO_FLASH_SPI_CLKDIV=4)
endif()

project(yocto-8)

add_compile_options("-fno-exceptions" $<$<CONFIG:RELWITHDEBINFO>:-Ofast>)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fno-rtti")

add_library(y8-core)

set(CORE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(CORE_SOURCES
    ${CORE_ROOT}/devices/image.cpp
    ${CORE_ROOT}/emu/emulator.cpp
    ${CORE_ROOT}/emu/bindings/input.cpp
    ${CORE_ROOT}/emu/bindings/math.cpp
    ${CORE_ROOT}/emu/bindings/misc.cpp
    ${CORE_ROOT}/emu/bindings/mmio.cpp
    ${CORE_ROOT}/emu/bindings/rng.cpp
    ${CORE_ROOT}/emu/bindings/time.cpp
    ${CORE_ROOT}/emu/bindings/video.cpp
    ${CORE_ROOT}/p8/parser.cpp
)
target_sources(y8-core PRIVATE ${CORE_SOURCES})

set(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/")
set(LUA_SOURCES
    ${LUA_ROOT}/lapi.c
    ${LUA_ROOT}/lauxlib.c
    ${LUA_ROOT}/lbaselib.c
    ${LUA_ROOT}/lbitlib.c
    ${LUA_ROOT}/lcode.c
    ${LUA_ROOT}/lcorolib.c
    ${LUA_ROOT}/lctype.c
    ${LUA_ROOT}/ldblib.c
    ${LUA_ROOT}/ldebug.c
    ${LUA_ROOT}/ldo.c
    ${LUA_ROOT}/ldump.c
    ${LUA_ROOT}/lfunc.c
    ${LUA_ROOT}/lgc.c
    ${LUA_ROOT}/linit.c
    ${LUA_ROOT}/liolib.c
    ${LUA_ROOT}/llex.c
    #${LUA_ROOT}/lmathlib.c
    ${LUA_ROOT}/lmem.c
    ${LUA_ROOT}/loadlib.c
    ${LUA_ROOT}/lobject.c
    ${LUA_ROOT}/lopcodes.c
    ${LUA_ROOT}/loslib.c
    ${LUA_ROOT}/lparser.c
    ${LUA_ROOT}/lstate.c
    ${LUA_ROOT}/lstring.c
    ${LUA_ROOT}/lstrlib.c
    ${LUA_ROOT}/ltable.c
    ${LUA_ROOT}/ltablib.c
    ${LUA_ROOT}/ltests.c
    ${LUA_ROOT}/ltm.c
    ${LUA_ROOT}/lundump.c
    ${LUA_ROOT}/lvm.c
    ${LUA_ROOT}/lzio.c
)
# compile Lua as C++ - we need this for the fixmath black magic
set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE CXX)
add_compile_definitions(LUA_USE_LONGJMP)
target_sources(y8-core PUBLIC ${LUA_SOURCES})

set(FIXMATH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libfixmath/libfixmath/")
set(FIXMATH_SOURCES
    ${FIXMATH_ROOT}/fix16_exp.c
    ${FIXMATH_ROOT}/fix16_sqrt.c
    ${FIXMATH_ROOT}/fix16_str.c
    ${FIXMATH_ROOT}/fix16_trig.c
    ${FIXMATH_ROOT}/fix16.c
)
target_sources(y8-core PUBLIC ${FIXMATH_SOURCES})

set(ALLOC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyalloc/")
set(ALLOC_SOURCES
    ${ALLOC_ROOT}/tinyalloc.c
)
target_sources(y8-core PUBLIC ${ALLOC_SOURCES})

if (Y8_DESKTOP_VM)
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")

    set(PLATFORM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/desktop")
    set(PLATFORM_SOURCES
        ${PLATFORM_ROOT}/hal/hal.cpp
        ${PLATFORM_ROOT}/main.cpp
        
    )

    add_executable(y8
        ${PLATFORM_SOURCES}
        ${PLATFORM_ROOT}/window.cpp
    )
    target_link_libraries(y8 PRIVATE y8-core sfml-system sfml-window sfml-graphics)

    add_executable(y8-headless ${PLATFORM_SOURCES})
    target_compile_definitions(y8-headless PRIVATE YOCTO8_DESKTOP_HEADLESS)
    target_link_libraries(y8-headless PRIVATE y8-core)

    function(y8_test testname)
        add_test(${testname} y8-headless "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/${testname}.p8")
    endfunction()

    y8_test(basic-assert)
else()
    pico_sdk_init()

    set(PLATFORM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/pico")
    set(PLATFORM_SOURCES
        ${PLATFORM_ROOT}/extmem/cachedinterface.cpp
        ${PLATFORM_ROOT}/extmem/faulthandler.cpp
        ${PLATFORM_ROOT}/extmem/spiram.cpp
        ${PLATFORM_ROOT}/hal/hal.cpp
        ${PLATFORM_ROOT}/hardwarestate.cpp
        ${PLATFORM_ROOT}/cmdthread.cpp
        ${PLATFORM_ROOT}/main/main.cpp
    )
    target_link_libraries(y8-core PUBLIC pico_stdlib pico_multicore hardware_spi hardware_dma)
    target_compile_definitions(y8-core PUBLIC PICO_MALLOC_PANIC=0)

    add_executable(y8 ${PLATFORM_SOURCES})
    target_link_libraries(y8 PRIVATE y8-core)

    set_property(TARGET y8 y8-core PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
    
    # Performs some checksum verification on SPI RAM when swapping pages in and out.
    # Use only for debugging; increases memory and CPU workload.
    #target_compile_definitions(y8 PRIVATE YOCTO8_EXTMEM_CHECKSUM)
    
    pico_add_extra_outputs(y8)

    set(SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/pico-sdk/")
    set_source_files_properties(
        "${SDK_ROOT}/src/rp2_common/hardware_irq/irq.c"
        PROPERTIES COMPILE_FLAGS -fno-lto
    )
endif()

target_include_directories(y8-core PUBLIC ${PLATFORM_ROOT})

target_compile_options(y8-core
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wno-ignored-qualifiers>
)

target_include_directories(y8-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyalloc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libfixmath/libfixmath"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

#pico_enable_stdio_usb(yocto-8 1)
