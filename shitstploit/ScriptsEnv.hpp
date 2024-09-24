#include <lualib.h>
#include <lua.h>
#include <Offsets.hpp>
#include <VM/src/lmem.h>

#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T



int lua_getthreadidentity(lua_State * L) {
    // Get the thread identity from your implementation
    const auto ExtraSpace = (uintptr_t)L->userdata;

    // Push the thread identity to the Lua stack as a number
    lua_pushnumber(L, *(__int64*)(ExtraSpace + Update::Userdata::Identity));

    // Return 1 to indicate one value is being returned
    return 1;
}

int lua_setthreadidentity(lua_State* L) {
    // Check if the first argument is a number
    if (!lua_isnumber(L, 1)) {
        luaL_error(L, "Expected a number as the argument");
        return 0;
    }

    // Get the thread identity from the Lua stack
    int identity = lua_tonumber(L, 1);

    // Set the thread identity using your implementation
    const auto ExtraSpace = (uintptr_t)L->userdata;
    *(__int64*)(ExtraSpace + Update::Userdata::Identity) = identity;
    // No return value, so return 0
    return 0;
}

auto getgenv(lua_State* L) -> int {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    return 1;
}



auto getreg(lua_State* L) -> int {
    ARG_CHECK(L, 0, 0);
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    return 1;
}

auto getsenv(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1);
    return 1;
}

auto sethiddenproperty(lua_State* L) -> int {
    ARG_CHECK(L, 0, 0);
    return 0;
}

auto gethiddenproperties(lua_State* L) -> int {
    ARG_CHECK(L, 0, 0);
    return 0;
}

auto getobjects(lua_State* L) -> int {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
    luaL_checktype(L, 2, LUA_TSTRING);

    //std::string rbx_id = lua_tostring(L, 2);
    //rbx_id = utilities::replace(rbx_id, "rbxassetid://", "");

    lua_pushvalue(L, 1);
    lua_getfield(L, -1, "GetService");
    lua_pushvalue(L, 1);
    lua_pushstring(L, "InsertService");

    lua_pcall(L, 2, 1, 0);

    lua_getfield(L, -1, "LoadLocalAsset");
    lua_pushvalue(L, -2);
    lua_pushvalue(L, 2);

    lua_pcall(L, 2, 1, 0);

    lua_newtable(L);
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, 1);

    return 1;
}



static const luaL_Reg script_envfuncs[] = {
    {"getthreadidentity", lua_getthreadidentity},
    {"getidentity", lua_getthreadidentity},
    {"getthreadcontext", lua_getthreadidentity},

    {"getgenv", getgenv},
    {"getreg", getreg},
    {"getsenv", getsenv},

    {"sethiddenproperty", sethiddenproperty},
    {"gethiddenproperties", gethiddenproperties},

    {"setthreadidentity", lua_setthreadidentity},
    {"setidentity", lua_setthreadidentity},
    {"setthreadcontext", lua_setthreadidentity},

    {"getobjects", getobjects},


    {NULL, NULL}
};

auto register_script(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, script_envfuncs);

    lua_pop(L, 1);
}