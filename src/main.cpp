#include <cstdio>
#include "pico/stdlib.h"
#include <cstdlib>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

int main() {
    setup_default_uart();
    printf("Hello, world!\n");

    lua_State* lua = luaL_newstate();

    const char lua_script[] = "print('Hello, world (but cooler)!')";
    int load_stat = luaL_loadbuffer(lua, lua_script, sizeof(lua_script) - 1, lua_script);
    lua_pcall(lua, 0, 0, 0);

    lua_close(lua);
}