pico_sdk_init()

add_library(yocto-8-pico-core
    extmem/cachedinterface.cpp
    extmem/faulthandler.cpp
)

target_link_libraries(yocto-8-pico-core yocto-8-core pico_stdlib hardware_spi)

target_include_directories(yocto-8-pico-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

function(add_rom NAME SOURCES)
    add_executable(${NAME} ${SOURCES})
    target_link_libraries(${NAME} yocto-8-pico-core)
    pico_add_extra_outputs(${NAME})
endfunction()

add_rom(yocto-8 main/main.cpp)
add_rom(psram-test main/extmem.cpp)

# FIXME: this is a horrible workaround for clangd in vscode
set(CLANGD_WORKAROUND_FLAGS "-isystem /usr/arm-none-eabi/include/c++/11.1.0/arm-none-eabi -isystem /usr/arm-none-eabi/include/c++/11.1.0/ -isystem /usr/arm-none-eabi/include/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANGD_WORKAROUND_FLAGS}")