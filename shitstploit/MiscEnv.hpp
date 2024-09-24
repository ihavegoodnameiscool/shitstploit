#include <lualib.h>
#include <lua.h>
#include <Offsets.hpp>

#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T
double cap = 60;
auto setfpscap(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER););
    double fps = lua_tonumber(L, 1);
    cap = fps;
//    task_scheduler->setfps(fps);

    return 0;
}

auto queue_on_teleport(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
    std::string code = lua_tostring(L, 1);
  //  cherry::environment::queue_on_tp(code);
    return 0;
}

int lua_setclipboard(lua_State* L) {
    // Get the string argument from Lua
    const char* text = luaL_checkstring(L, 1);

    // Open the clipboard
    if (OpenClipboard(NULL)) {
        // Empty the clipboard
        EmptyClipboard();

        // Allocate global memory for the clipboard text
        size_t len = strlen(text) + 1;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

        // Copy the string to the global memory
        if (hMem) {
            memcpy(GlobalLock(hMem), text, len);
            GlobalUnlock(hMem);

            // Set the clipboard data with the CF_TEXT format (ANSI text)
            SetClipboardData(CF_TEXT, hMem);
        }

        // Close the clipboard
        CloseClipboard();
    }

    // Return nothing, as this function is a void return in Lua
    return 0;
}

int lua_messagebox(lua_State* L) {
    // Check and get the arguments from Lua
    const char* text = luaL_checkstring(L, 1);      // First argument: text
    const char* caption = luaL_checkstring(L, 2);   // Second argument: caption
    int flags = luaL_checkinteger(L, 3);            // Third argument: flags

    // Call the Windows API MessageBoxA function
    int result = MessageBoxA(NULL, text, caption, flags);

    // Return the result of MessageBoxA to Lua
    lua_pushinteger(L, result);

    return 1;  // Returning one result (the result of MessageBoxA)
}

int lua_identifyexecutor(lua_State* L) {
    // Push the first string onto the Lua stack
    lua_pushstring(L, "FentanylSploit");

    // Push the second string onto the Lua stack
    lua_pushstring(L, "1.1");

    // Return the number of results
    return 2;
}

auto getfpscap(lua_State* L) -> int {
    ARG_CHECK(L, 0, 0);
    lua_pushnumber(L, cap);
    return 1;
}

auto lua_loadstring(lua_State* L) -> int {
    ARG_CHECK(L, 1, 2, luaL_checktype(L, 1, LUA_TSTRING););
    const char* code = lua_tostring(L, 1);
    const char* chunk_name = luaL_optstring(L, 2, "ihavegoodexecutor");
    StateTools::Execute(code, (uintptr_t)L);
    return 1;
}


static const luaL_Reg env_funcs34[] = {
    {"setclipboard", lua_setclipboard},
    {"toclipboard", lua_setclipboard},

    //{"lz4compress", lz4compress},
  //  {"lz4decompress", lz4decompress},

    {"identifyexecutor", lua_identifyexecutor},
    {"getexecutorname", lua_identifyexecutor},

    {"setfpscap", setfpscap},
    {"getfpscap", getfpscap},

    {"queue_on_teleport", queue_on_teleport},
    {"queueonteleport", queue_on_teleport},

    {"loadstring", lua_loadstring},

    {NULL, NULL}
};

auto register_misc(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, env_funcs34);

    lua_pop(L, 1);
}