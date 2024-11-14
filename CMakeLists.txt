cmake_minimum_required(VERSION 3.13)

# FIXME: This is currently broken when using the secondary memory pool because OOM situations in Lua are turned into UB
# Maybe this should be scrapped altogether
# set(Y8_YOLO_RELEASE OFF CACHE BOOL "Enables highly aggressive optimizations that turn some error conditions into undefined behavior")
set(Y8_ARCH "desktop" CACHE STRING "Selects the architecture (desktop/pico)")
set(Y8_PLATFORM "" CACHE STRING "Selects the platform (i.e. subarch), if relevant (none/asupico/picosystem). Must be consistent with architecture.")

# macro definitions
set(Y8_DEFINITIONS "Y8_ARCH=${Y8_ARCH} Y8_PLATFORM=${Y8_PLATFORM}")

function(add_y8_definition_string OPTION_NAME OPTION_DEFAULT OPTION_DEFINITION)
    set(${OPTION_NAME} ${OPTION_DEFAULT} CACHE STRING ${OPTION_DEFINITION})
    set(Y8_DEFINITIONS "${Y8_DEFINITIONS};${OPTION_NAME}=${${OPTION_NAME}}" PARENT_SCOPE)
endfunction()

function(add_y8_definition_flag OPTION_NAME OPTION_DEFAULT OPTION_DEFINITION)
    set(${OPTION_NAME} ${OPTION_DEFAULT} CACHE BOOL ${OPTION_DEFINITION})

    if(${${OPTION_NAME}})
        set(Y8_DEFINITIONS "${Y8_DEFINITIONS};${OPTION_NAME}" PARENT_SCOPE)
    endif()
endfunction()

add_y8_definition_flag(Y8_DEBUG_MEMCHECK OFF "On PSRAM-enabled systems, run a quick memory test script to sanity check whether the PSRAM appears functional and stable. If the memcheck fails, an error will be logged and the PSRAM heap will be disabled")
add_y8_definition_flag(Y8_EXPERIMENTAL_GENGC ON "Enable the experimental Lua 5.2 generational GC, which is known to have bugs but can prove beneficial for performance")
add_y8_definition_flag(Y8_LUA_REPL ON "Enable the stdin Lua REPL")
add_y8_definition_string(Y8_RESERVED_FIRMWARE_SIZE 397312 "Reserved firmware size, in bytes. Must align to 4096. The rest may be used for the internal FAT storage. Modifying firmware size will break existing partitions. Defaults to 386KiB (up-aligned to 4K page)")
add_y8_definition_string(Y8_LED_PWM_SCALE 80 "Max PWM value (out of 255) for the main onboard LED, if relevant.")

# force disabling the use of ctype for lua so that we handle UTF-8 ourselves
add_compile_definitions(LUA_USE_CTYPE=0)

message("Following definitions set: [${Y8_DEFINITIONS}]")

if(Y8_ARCH STREQUAL "desktop")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
elseif(Y8_ARCH STREQUAL "pico")
    set(SKIP_PICO_MALLOC 1)
    set(PICO_CXX_ENABLE_EXCEPTIONS 1)
    set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/pico/platform/boards/)
    set(LTO_FLAGS "-flto=auto" "-O2" "-fpeel-loops")
    include(vendor/pico-sdk/pico_sdk_init.cmake)
else()
    message(FATAL_ERROR "Y8_ARCH not specified or illegal")
endif()

project(yocto-8)

# flags common to all TUs
set(COMMON_FLAGS

    # default to -Os for all sources, but we override it just later
    "-Os"

    # ninja opens a pipe so the compiler disables ansi escape sequences by
    # default. let's force it for now
    "-fdiagnostics-color=always"

    # relative path
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/src/=src/"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMMON_FLAGS ${COMMON_FLAGS}
        "-fimplicit-constexpr"
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(Y8_ARCH STREQUAL "pico")
        set(COMMON_FLAGS ${COMMON_FLAGS}
            "-flto=thin"
        )
    endif()
endif()

if(Y8_ARCH STREQUAL "pico")
    set(COMMON_FLAGS
        ${COMMON_FLAGS}
        "-fno-threadsafe-statics"

        "-Wstack-usage=256"
        "-fstack-usage"

        # nano specs turn exception raise into terminate(?) but we do actually
        # use exceptions (mostly from the load trigger mechanism to interrupt
        # execution from anywhere in the VM)
        # "--specs=nano.specs"
    )
endif()

add_link_options(${COMMON_FLAGS} ${LTO_FLAGS})
add_compile_options(${COMMON_FLAGS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexceptions -fno-rtti")

# this is a weird hack that defines a core library on the desktop platform
# (which avoids rebuilding everything when doing headless/gui builds)
#
# the core library trick worked OK on pico, but it resulted in double the files
# being compiled for some reason; unsure why.
if(Y8_ARCH STREQUAL "desktop")
    set(CORE_LIBRARY_TARGET y8-core)
    add_library(${CORE_LIBRARY_TARGET} STATIC)
elseif(Y8_ARCH STREQUAL "pico")
    set(CORE_LIBRARY_TARGET y8)
endif()

set(CORE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(CORE_SOURCES
    ${CORE_ROOT}/emu/alloc.cpp
    ${CORE_ROOT}/emu/emulator.cpp
    ${CORE_ROOT}/emu/bindings/input.cpp
    ${CORE_ROOT}/emu/bindings/math.cpp
    ${CORE_ROOT}/emu/bindings/misc.cpp
    ${CORE_ROOT}/emu/bindings/mmio.cpp
    ${CORE_ROOT}/emu/bindings/rng.cpp
    ${CORE_ROOT}/emu/bindings/table.cpp
    ${CORE_ROOT}/emu/bindings/time.cpp
    ${CORE_ROOT}/emu/bindings/video.cpp
    ${CORE_ROOT}/p8/parser.cpp
)

set(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/")
set(LUA_SOURCES
    ${LUA_ROOT}/lapi.cpp
    ${LUA_ROOT}/lauxlib.cpp
    ${LUA_ROOT}/lbaselib.cpp

    # ${LUA_ROOT}/lbitlib.cpp
    ${LUA_ROOT}/lcode.cpp
    ${LUA_ROOT}/lcorolib.cpp
    ${LUA_ROOT}/lctype.cpp

    # ${LUA_ROOT}/ldblib.cpp
    ${LUA_ROOT}/ldebug.cpp
    ${LUA_ROOT}/ldo.cpp

    # ${LUA_ROOT}/ldump.cpp
    ${LUA_ROOT}/lfunc.cpp
    ${LUA_ROOT}/lgc.cpp
    ${LUA_ROOT}/linit.cpp

    # ${LUA_ROOT}/liolib.cpp
    ${LUA_ROOT}/llex.cpp

    # ${LUA_ROOT}/lmathlib.cpp
    ${LUA_ROOT}/lmem.cpp

    # ${LUA_ROOT}/loadlib.cpp
    ${LUA_ROOT}/lobject.cpp
    ${LUA_ROOT}/lopcodes.cpp

    # ${LUA_ROOT}/loslib.cpp
    ${LUA_ROOT}/lparser.cpp
    ${LUA_ROOT}/lstate.cpp
    ${LUA_ROOT}/lstring.cpp

    # ${LUA_ROOT}/lstrlib.cpp

    # ${LUA_ROOT}/ltablib.cpp
    # ${LUA_ROOT}/ltests.cpp
    ${LUA_ROOT}/ltm.cpp

    # ${LUA_ROOT}/lundump.cpp
    ${LUA_ROOT}/lvm.cpp
    ${LUA_ROOT}/ltable.cpp
    ${LUA_ROOT}/lzio.cpp
)

# Commented out: for now, use C++ exceptions, not longjmp.
# add_compile_definitions(LUA_USE_LONGJMP)
set(FIXMATH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libfixmath/libfixmath/")
set(FIXMATH_SOURCES
    ${FIXMATH_ROOT}/fix16_exp.cpp
    ${FIXMATH_ROOT}/fix16_sqrt.cpp
    ${FIXMATH_ROOT}/fix16_str.cpp
    ${FIXMATH_ROOT}/fix16_trig.cpp
)

set(FATFS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fatfs/")
set(FATFS_SOURCES
    ${FATFS_ROOT}/ff.c
    ${FATFS_ROOT}/ffunicode.c
)

set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/zlib/")
set(ZLIB_SOURCES
    ${ZLIB_ROOT}/adler32.c
    ${ZLIB_ROOT}/crc32.c
    ${ZLIB_ROOT}/deflate.c
    ${ZLIB_ROOT}/trees.c
    ${ZLIB_ROOT}/zutil.c
)

set(PNGENC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/pngenc/src/")
set(PNGENC_SOURCES
    ${PNGENC_ROOT}/PNGenc.cpp
)

set_property(
    SOURCE ${CORE_SOURCES} ${LUA_SOURCES} ${FIXMATH_SOURCES} ${FATFS_SOURCES} ${ZLIB_SOURCES} ${PNGENC_SOURCES}
    APPEND PROPERTY COMPILE_OPTIONS
    ${LTO_FLAGS}
)

# we care more about code size than about performance for some functionality.
# for these, use lighter optimization levels
set(SIZE_SENSITIVE_SOURCES
    ${LUA_ROOT}/llex.cpp
    ${LUA_ROOT}/lparser.cpp
    ${LUA_ROOT}/lcode.cpp
    ${LUA_ROOT}/ldebug.cpp

    ${CORE_ROOT}/p8/parser.cpp

    ${FIXMATH_ROOT}/fix16_str.cpp

    ${FATFS_SOURCES}

    ${ZLIB_SOURCES}

    ${PNGENC_SOURCES}
)

set_property(
    SOURCE ${SIZE_SENSITIVE_SOURCES}
    APPEND PROPERTY COMPILE_OPTIONS
    "-Os"
)
set_property(
    SOURCE ${LUA_ROOT}/lvm.cpp
    APPEND PROPERTY COMPILE_OPTIONS

    # increasing the inlining threshold and using -O3 adds a few KiB of code
    # size here (which goes to SRAM) but it seems to help execution time by ~5%
    #
    # it might be that a single inline is responsible for this; can we find
    # which?
    "-finline-limit=200"
    "-Ofast"
)

if(Y8_ARCH STREQUAL "desktop")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")

    set(ARCH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/desktop")
    set(ARCH_SOURCES
        ${ARCH_ROOT}/hal/hal.cpp
        ${ARCH_ROOT}/main.cpp
    )
    set(ARCH_HAL_ROOT "${ARCH_ROOT}/hal/")

    add_executable(y8
        ${ARCH_SOURCES}
        ${ARCH_ROOT}/window.cpp
    )
    target_link_libraries(y8 PRIVATE ${CORE_LIBRARY_TARGET} sfml-system sfml-window sfml-graphics)

    add_executable(y8-headless ${ARCH_SOURCES})
    target_compile_definitions(y8-headless PRIVATE Y8_DESKTOP_HEADLESS)
    target_link_libraries(y8-headless PRIVATE ${CORE_LIBRARY_TARGET})

    add_compile_definitions(Y8_SRAM_SECTION=".text")
    add_compile_definitions(Y8_PSRAM_SECTION=".text")
elseif(Y8_ARCH STREQUAL "pico")
    pico_sdk_init()

    set(ARCH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/pico")
    set(ARCH_SOURCES
        ${ARCH_ROOT}/hal/hal.cpp
        ${ARCH_ROOT}/hardwarestate.cpp
        ${ARCH_ROOT}/cmdthread.cpp
        ${ARCH_ROOT}/fs/fatfsimpl.cpp
        ${ARCH_ROOT}/fs/hwinit.cpp
        ${ARCH_ROOT}/usb/usb_descriptors.cpp
        ${ARCH_ROOT}/usb/hwinit.cpp
        ${ARCH_ROOT}/usb/msc_disk.cpp
        ${ARCH_ROOT}/main/main.cpp
        ${CORE_ROOT}/compilerhacks/gcc_stub_verbose_terminate_handler.cpp
        ${CORE_ROOT}/compilerhacks/gcc_exception_static_alloc.cpp
        ${FATFS_SOURCES}
    )
    set(ARCH_HAL_ROOT "${ARCH_ROOT}/hal/")

    if(Y8_PLATFORM STREQUAL "asupico")
        set(PLATFORM_ROOT "${ARCH_ROOT}/platform/asupico")
        set(PLATFORM_SOURCES
            ${PLATFORM_ROOT}/asupico.cpp
            ${PLATFORM_ROOT}/hal.cpp
            ${PLATFORM_ROOT}/platform.cpp
            ${ARCH_ROOT}/video/dwoqspi.cpp # FIXME: proper compile-time API for driver selection
            ${CORE_ROOT}/emu/tlsf.cpp # TODO: cleaner way to include this
        )

        # extmem size is autodetected at runtime
        add_compile_definitions(Y8_USE_EXTMEM)
    elseif(Y8_PLATFORM STREQUAL "picosystem")
        set(PLATFORM_ROOT "${ARCH_ROOT}/platform/picosystem")
        set(PLATFORM_SOURCES
            ${PLATFORM_ROOT}/picosystem.cpp
            ${PLATFORM_ROOT}/hal.cpp
            ${PLATFORM_ROOT}/platform.cpp)
    else()
        message(FATAL_ERROR "Y8_PLATFORM not specified, illegal or not supported on the \"pico\" arch")
    endif()

    set_property(
        SOURCE ${ARCH_SOURCES} ${PLATFORM_SOURCES}
        APPEND PROPERTY COMPILE_OPTIONS
        ${LTO_FLAGS}
    )

    add_executable(${CORE_LIBRARY_TARGET} ${ARCH_SOURCES} ${PLATFORM_SOURCES})

    target_include_directories(${CORE_LIBRARY_TARGET} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fatfs/"
        "${ARCH_ROOT}/usb/"
    )

    target_link_libraries(${CORE_LIBRARY_TARGET} PUBLIC
        pico_stdlib
        pico_multicore
        pico_rand
        pico_flash
        hardware_xip_cache
        hardware_pwm
        pico_unique_id
        tinyusb_device
        tinyusb_board
    )

    target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC Y8_INFINITE_LOOP_EXIT)

    # We use SKIP_PICO_MALLOC so that we wrap the C alloc functions ourselves
    target_link_options(${CORE_LIBRARY_TARGET} PUBLIC -Wl,--wrap=malloc,--wrap=free,--wrap=realloc)

    # Let the pico-sdk configure the MPU to protect the end of the stack, to
    # detect certain kinds of stack overflows (presumably not all if large
    # buffers aren't entirely written to, etc.)
    target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC PICO_USE_STACK_GUARDS=1)

    # 12KiB core0 stack because Lua stacks can get pretty deep
    # This requires the linker script to swap out SCRATCH_X and SCRATCH_Y:
    # The ARM stack grows downwards, so we let it overflow into main RAM (but we
    # restrict the heap not to ever overflow there).
    #
    # See memmap_asupico.ld for proper implementation
    # FIXME: memmap_picosystem.ld does not have this implemented
    target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC PICO_STACK_SIZE=0x3000)

    # 0.25KiB core1 worker stack because it never recurses much
    # Leaves plenty of space for stuff there.
    target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC PICO_CORE1_STACK_SIZE=0x200)

    # NOTE: as of 2.0.0, the pico-sdk is not including XIP setup code for the
    # flash, resulting in suboptimal timing for the stock flash
    # https://github.com/raspberrypi/pico-sdk/issues/1903
    # however, enabling it does mess up debugging for some reason
    # target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC PICO_EMBED_XIP_SETUP=1)

    # needed for a LTO hack
    # target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC PICO_STDIO_SHORT_CIRCUIT_CLIB_FUNCS=0)
    if(Y8_PLATFORM STREQUAL "asupico")
        target_link_libraries(${CORE_LIBRARY_TARGET} PUBLIC hardware_spi hardware_dma hardware_pio)
        pico_generate_pio_header(${CORE_LIBRARY_TARGET} ${ARCH_ROOT}/video/dwoqspipio.pio)

        pico_set_linker_script(${CORE_LIBRARY_TARGET} "${PLATFORM_ROOT}/memmap_asupico.ld")
    elseif(Y8_PLATFORM STREQUAL "picosystem")
        # target_link_libraries(${CORE_LIBRARY_TARGET} PUBLIC hardware_spi hardware_pio hardware_dma)
        target_link_libraries(${CORE_LIBRARY_TARGET} PUBLIC hardware_spi hardware_dma)

        # pico_generate_pio_header(${CORE_LIBRARY_TARGET} ${PLATFORM_ROOT}/screen.pio)
        pico_set_linker_script(${CORE_LIBRARY_TARGET} "${PLATFORM_ROOT}/memmap_picosystem.ld")
        pico_enable_stdio_usb(${CORE_LIBRARY_TARGET} 1)
    endif()

    set(SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/pico-sdk/")

    add_compile_definitions(Y8_SRAM_SECTION=".time_critical.y8")
    add_compile_definitions(Y8_PSRAM_SECTION=".psram_bss")
    add_compile_definitions(Y8_CORE1_SRAM_SECTION="${Y8_SRAM_SECTION}")

    pico_add_extra_outputs(${CORE_LIBRARY_TARGET})
endif()

target_sources(${CORE_LIBRARY_TARGET} PRIVATE ${CORE_SOURCES})
target_sources(${CORE_LIBRARY_TARGET} PRIVATE ${LUA_SOURCES})
target_sources(${CORE_LIBRARY_TARGET} PRIVATE ${FIXMATH_SOURCES})
target_sources(${CORE_LIBRARY_TARGET} PRIVATE ${ZLIB_SOURCES})
target_sources(${CORE_LIBRARY_TARGET} PRIVATE ${PNGENC_SOURCES})

target_include_directories(${CORE_LIBRARY_TARGET} PUBLIC ${ARCH_ROOT})

target_compile_options(${CORE_LIBRARY_TARGET}
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wno-ignored-qualifiers>
)

target_compile_definitions(${CORE_LIBRARY_TARGET} PUBLIC ${Y8_DEFINITIONS})

target_include_directories(${CORE_LIBRARY_TARGET} PUBLIC
    ${LUA_ROOT}
    ${FIXMATH_ROOT}
    ${PNGENC_ROOT}
    ${ZLIB_ROOT}
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
