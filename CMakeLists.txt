cmake_minimum_required(VERSION 3.13)

# FIXME: This is currently broken when using the secondary memory pool because OOM situations in Lua are turned into UB
#        Maybe this should be scrapped altogether
# set(YOCTO8_YOLO_RELEASE OFF CACHE BOOL "Enables highly aggressive optimizations that turn some error conditions into undefined behavior")

set(Y8_ARCH "desktop" CACHE STRING "Selects the architecture (desktop/pico)")
set(Y8_PLATFORM "" CACHE STRING "Selects the platform (i.e. subarch), if relevant (none/asupico/picosystem). Must be consistent with architecture.")

if (Y8_ARCH STREQUAL "desktop")
    enable_testing()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
elseif(Y8_ARCH STREQUAL "pico")
    include(vendor/pico-sdk/pico_sdk_init.cmake)

    # FIXME: this is a horrible workaround for clangd in vscode
    set(CLANGD_WORKAROUND_FLAGS "-isystem /usr/arm-none-eabi/include/c++/12.1.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/12.1.0/ -isystem /usr/arm-none-eabi/include/")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")

    # Higher frequencies mean we need a higher SPI clock divisor.
    # Past 200MHz, we are way out of spec for the Flash and everything crashes.
    # This is not as much of a big deal as you may think;
    # the XIP cache works well if you pay attention to things (e.g. multicore).
    add_compile_definitions(PICO_FLASH_SPI_CLKDIV=4)
else()
    message(FATAL_ERROR "Y8_ARCH not specified or illegal")
endif()

project(yocto-8)

# FIXME: this really shouldn't be duplicated :)
add_link_options(
    "-fno-exceptions"
    "-Os" "-ffast-math"
    "-flto"
)

add_compile_options(
    "-fno-exceptions"
    "-Os" "-ffast-math"
    "-flto"
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

set(ALLOC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/umm_malloc/src/")
set(ALLOC_SOURCES
    ${ALLOC_ROOT}/umm_info.c
    ${ALLOC_ROOT}/umm_integrity.c
    ${ALLOC_ROOT}/umm_malloc.c
    ${ALLOC_ROOT}/umm_poison.c)
target_sources(y8-core PUBLIC ${ALLOC_SOURCES})
target_include_directories(y8-core PRIVATE ${ALLOC_ROOT})

if (Y8_ARCH STREQUAL "desktop")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
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

    function(y8_test testname)
        add_test(${testname} y8-headless "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/${testname}.p8")
    endfunction()

    add_compile_definitions(FAST_FUNC_TARGET_SECTION=".text")

    y8_test(basic-assert)
    y8_test(lua-if)
elseif (Y8_ARCH STREQUAL "pico")
    pico_sdk_init()

    set(ARCH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/pico")
    set(ARCH_SOURCES
        ${ARCH_ROOT}/extmem/paging.cpp
        ${ARCH_ROOT}/extmem/faulthandler.cpp
        ${ARCH_ROOT}/extmem/spiram.cpp
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

    add_executable(y8 ${ARCH_SOURCES} ${PLATFORM_SOURCES})
    target_link_libraries(y8 PRIVATE y8-core)
    
    # Performs some checksum verification on SPI RAM when swapping pages in and out.
    # Use only for debugging; increases memory and CPU workload.
    #target_compile_definitions(y8 PRIVATE YOCTO8_EXTMEM_CHECKSUM)
    
    pico_add_extra_outputs(y8)

    if (Y8_PLATFORM STREQUAL "asupico")
        target_link_libraries(y8-core PUBLIC hardware_spi hardware_dma)
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
