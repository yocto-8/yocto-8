#include "pico/stdlib.h"
#include "pico/time.h"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
/*
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}*/

void __attribute__((optimize("O0"))) foo(int i) {}

int main() {
    set_sys_clock_khz(250000, true);

    stdio_init_all();

    /*lua_State* lua = luaL_newstate();
    luaL_openlibs(lua);

    const char lua_script[] = "print('Hello, world (but cooler)!\n')";
    int load_stat = luaL_loadbuffer(lua, lua_script, sizeof(lua_script) - 1, lua_script);
    lua_pcall(lua, 0, 0, 0);*/
    
    //lua_close(lua);
}