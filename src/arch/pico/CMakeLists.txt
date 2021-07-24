pico_sdk_init()

function(add_rom NAME SOURCES)
    add_executable(
        ${NAME}
        extmem/cachedinterface.cpp
        extmem/faulthandler.cpp
        extmem/spiram.cpp
        hal/hal.cpp
        hardwarestate.cpp
        cmdthread.cpp
        ${SOURCES}
    )
    target_link_libraries(${NAME} yocto-8-core pico_stdlib pico_multicore hardware_spi hardware_dma)
    target_include_directories(${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # message(WARNING "AAA${PICO_DEFAULT_BOOT_STAGE2_FILE}AAA")
    # pico_define_boot_stage2(${NAME} ${PICO_DEFAULT_BOOT_STAGE2_FILE})
    # target_compile_definitions(${NAME} PRIVATE PICO_FLASH_SPI_CLKDIV=4)
    # pico_set_boot_stage2(TARGET ${NAME})    

    pico_add_extra_outputs(${NAME})
endfunction()

add_rom(yocto-8 main/main.cpp)
add_rom(psram-test main/extmem.cpp)

# FIXME: this is a horrible workaround for clangd in vscode
set(CLANGD_WORKAROUND_FLAGS "-isystem /usr/arm-none-eabi/include/c++/11.1.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/11.1.0/ -isystem /usr/arm-none-eabi/include/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")