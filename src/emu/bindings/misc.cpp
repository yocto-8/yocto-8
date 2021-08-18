#include "input.hpp"

#include <lua.h>
#include <lauxlib.h>

namespace emu::bindings
{

int y8_printh(lua_State* state)
{
    // based on lua standard print()
    // FIXME: i don't think standard printh in pico-8 uses tostring.

    int n = lua_gettop(state);  /* number of arguments */
    int i;
    lua_getglobal(state, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(state, -1);  /* function to be called */
        lua_pushvalue(state, i);   /* value to print */
        lua_call(state, 1, 1);
        s = lua_tolstring(state, -1, &l);  /* get result */
        if (s == nullptr)
        {
            return luaL_error(state,
            LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i>1)
        {
            printf("\t");
        }
        
        printf("%s", s);
        lua_pop(state, 1);  /* pop result */
    }
    printf("\n");
    return 0;
}

}