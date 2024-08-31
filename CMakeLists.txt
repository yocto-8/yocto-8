cmake_minimum_required(VERSION 3.13)

# FIXME: This is currently broken when using the secondary memory pool because OOM situations in Lua are turned into UB
#        Maybe this should be scrapped altogether
# set(YOCTO8_YOLO_RELEASE OFF CACHE BOOL "Enables highly aggressive optimizations that turn some error conditions into undefined behavior")

set(Y8_ARCH "desktop" CACHE STRING "Selects the architecture (desktop/pico)")
set(Y8_PLATFORM "" CACHE STRING "Selects the platform (i.e. subarch), if relevant (none/asupico/picosystem). Must be consistent with architecture.")

if (Y8_ARCH STREQUAL "desktop")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold -Wl,--thinlto-cache-dir=./llvmcache")
elseif(Y8_ARCH STREQUAL "pico")
    set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/pico/platform/boards/)
    include(vendor/pico-sdk/pico_sdk_init.cmake)
else()
    message(FATAL_ERROR "Y8_ARCH not specified or illegal")
endif()

project(yocto-8)

set(PERF_FLAGS
    "-Os"

    # is a net gain, for some reason
    # TODO: recheck performance, maybe it's not relevant
    "-ffast-math")

if (Y8_ARCH STREQUAL "pico")
    set(PERF_FLAGS ${PERF_FLAGS}) # "-flto")  # todo: detect clang and use thinlto
endif()

# FIXME: this really shouldn't be duplicated :)
add_link_options(
    "-fno-exceptions"
    ${PERF_FLAGS}
)

add_compile_options(
    "-fno-exceptions"
    ${PERF_FLAGS}
)

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
target_sources(y8-core PUBLIC ${CORE_SOURCES})

set(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/")
set(LUA_SOURCES
    ${LUA_ROOT}/lapi.c
    ${LUA_ROOT}/lauxlib.c
    ${LUA_ROOT}/lbaselib.c
    # ${LUA_ROOT}/lbitlib.c
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
    # ${LUA_ROOT}/liolib.c
    ${LUA_ROOT}/llex.c
    #${LUA_ROOT}/lmathlib.c
    ${LUA_ROOT}/lmem.c
    ${LUA_ROOT}/loadlib.c
    ${LUA_ROOT}/lobject.c
    ${LUA_ROOT}/lopcodes.c
    # ${LUA_ROOT}/loslib.c
    ${LUA_ROOT}/lparser.c
    ${LUA_ROOT}/lstate.c
    ${LUA_ROOT}/lstring.c
    # ${LUA_ROOT}/lstrlib.c
    ${LUA_ROOT}/ltable.c
    # ${LUA_ROOT}/ltablib.c
    # ${LUA_ROOT}/ltests.c
    ${LUA_ROOT}/ltm.c
    ${LUA_ROOT}/lundump.c
    ${LUA_ROOT}/lvm.c
    ${LUA_ROOT}/lzio.c
)
# compile Lua as C++ - we need this for the fixmath black magic
set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE CXX)

# we care more about code size than about performance for some functionality.
# for these, use lighter optimization levels
set_source_files_properties(
    ${LUA_ROOT}/llex.c
    ${LUA_ROOT}/lparser.c
    ${LUA_ROOT}/lcode.c
    ${LUA_ROOT}/lundump.c
    ${LUA_ROOT}/ldebug.c

    PROPERTIES COMPILE_FLAGS "-Os")
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
set_source_files_properties(${FIXMATH_SOURCES} PROPERTIES LANGUAGE CXX)
target_sources(y8-core PUBLIC ${FIXMATH_SOURCES})

if (Y8_ARCH STREQUAL "desktop")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")

    set(ARCH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/desktop")
    set(ARCH_SOURCES
        ${ARCH_ROOT}/hal/hal.cpp
        ${ARCH_ROOT}/main.cpp
    )

    add_executable(y8
        ${ARCH_SOURCES}
        ${ARCH_ROOT}/window.cpp
    )
    target_link_libraries(y8 PRIVATE y8-core sfml-system sfml-window sfml-graphics)

    add_executable(y8-headless ${ARCH_SOURCES})
    target_compile_definitions(y8-headless PRIVATE YOCTO8_DESKTOP_HEADLESS)
    target_link_libraries(y8-headless PRIVATE y8-core)

    add_compile_definitions(FAST_FUNC_TARGET_SECTION=".text")
elseif (Y8_ARCH STREQUAL "pico")
    pico_sdk_init()

    set(ARCH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/pico")
    set(ARCH_SOURCES
        ${ARCH_ROOT}/hal/hal.cpp
        ${ARCH_ROOT}/hardwarestate.cpp
        ${ARCH_ROOT}/cmdthread.cpp
        ${ARCH_ROOT}/main/main.cpp
    )

    if (Y8_PLATFORM STREQUAL "asupico")
        set(PLATFORM_ROOT "${ARCH_ROOT}/platform/asupico")
        set(PLATFORM_SOURCES
            ${PLATFORM_ROOT}/asupico.cpp
            ${PLATFORM_ROOT}/hal.cpp
            ${PLATFORM_ROOT}/platform.cpp)

        # extmem size is autodetected at runtime
        add_compile_definitions(Y8_USE_EXTMEM Y8_EXTMEM_START=0x11000000)
    elseif (Y8_PLATFORM STREQUAL "picosystem")
        set(PLATFORM_ROOT "${ARCH_ROOT}/platform/picosystem")
        set(PLATFORM_SOURCES
            ${PLATFORM_ROOT}/picosystem.cpp
            ${PLATFORM_ROOT}/hal.cpp
            ${PLATFORM_ROOT}/platform.cpp)
    else()
        message(FATAL_ERROR "Y8_PLATFORM not specified, illegal or not supported on the \"pico\" arch")
    endif()

    target_link_libraries(y8-core PUBLIC pico_stdlib pico_multicore cmsis_core)
    
    # NOTE: we require PICO_MALLOC_PANIC=0 here for the following reasons:
    # 1. usage of the secondary memory pool requires us to handle failing allocs.
    # 2. dynamic allocation in non-desktop platforms is forbidden, so we can check
    #    malloc return values in the only place it can be used: the lua allocator.
    # 3. we rely on the lua emergency GC, which can save us in this usecase.
    target_compile_definitions(y8-core PUBLIC PICO_MALLOC_PANIC=0)

    # NOTE: we forbid malloc outsides of lua malloc; and that excludes any work
    # on the second core.
    target_compile_definitions(y8-core PUBLIC PICO_USE_MALLOC_MUTEX=0)

    add_executable(y8 ${ARCH_SOURCES} ${PLATFORM_SOURCES})
    target_link_libraries(y8 PUBLIC y8-core)
    
    pico_add_extra_outputs(y8)

    if (Y8_PLATFORM STREQUAL "asupico")
        target_link_libraries(y8-core PUBLIC hardware_spi hardware_dma)
        target_compile_definitions(y8-core PUBLIC PICO_CMSIS_RENAME_EXCEPTIONS=0)
    elseif (Y8_PLATFORM STREQUAL "picosystem")
        #target_link_libraries(y8-core PUBLIC hardware_spi hardware_pio hardware_dma)
        target_link_libraries(y8-core PUBLIC hardware_spi hardware_dma)
        #pico_generate_pio_header(y8-core ${PLATFORM_ROOT}/screen.pio)
        pico_set_linker_script(y8 "${PLATFORM_ROOT}/memmap_picosystem.ld")
        pico_enable_stdio_usb(y8 1)
    endif()

    set(SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/pico-sdk/")

    # NOTE: at the moment, usage of LTO on the pico has its shares of gotchas, see
    # https://github.com/raspberrypi/pico-sdk/issues/97
    # so we have to do a few silly things... overall it's a net gain on pico, though.
    set_source_files_properties(
        "${SDK_ROOT}/src/rp2_common/hardware_irq/irq.c"
        PROPERTIES COMPILE_FLAGS -fno-lto
    )

    add_compile_definitions(FAST_FUNC_TARGET_SECTION=".time_critical.y8")
endif()

target_include_directories(y8-core PUBLIC ${ARCH_ROOT})

target_compile_options(y8-core
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wno-ignored-qualifiers>
)

# Causes issues with lua; when lua excluded, no significant gains
# set_target_properties(y8 PROPERTIES
#     UNITY_BUILD ON
# )
# foreach(FILE IN LISTS LUA_SOURCES)
# 	set_property(SOURCE ${FILE} PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)
# endforeach()

target_include_directories(y8-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libfixmath/libfixmath"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/frozen/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
