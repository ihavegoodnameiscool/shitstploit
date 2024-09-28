#include <lualib.h>
#include <lua.h>
#include <Offsets.hpp>
#include "lz4/lz4.h"

#define dereference_pointer(ptr) (*(int*)ptr)
#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T
double cap = 60;
auto setfpscap(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER););
    double fps = lua_tonumber(L, 1);
    cap = fps;
//    task_scheduler->setfps(fps);

    return 0;
}
auto checkinstance(lua_State* L, int idx) -> bool {
    const char* t = luaL_typename(L, idx);
    return (strcmp(t, "Instance") == 0);
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
const wchar_t* to_wchar(const char* szConvert) {
    auto charLen = strlen(szConvert) + 1;
    auto* wText = new wchar_t[charLen];
    size_t outSize = 0;
    mbstowcs_s(&outSize, wText, charLen, szConvert, charLen - 1);
    return wText;
}

auto lua_loadstring(lua_State* L) -> int {


    const char* str = luaL_checkstring(L, 1);

    if (std::string(str).empty() || lua_type(L, 1) != LUA_TSTRING) {
        lua_pushnil(L);
        return 1;
    }

    return loadstring(L, str);


}


int lz4compress(lua_State* L) {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
    const char* data = lua_tostring(L, 1);
    int nMaxCompressedSize = LZ4_compressBound(strlen(data));
    char* out_buffer = new char[nMaxCompressedSize];

    LZ4_compress(data, out_buffer, strlen(data));
    lua_pushlstring(L, out_buffer, nMaxCompressedSize);
    return 1;
}
int lz4decompress(lua_State* L) {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TSTRING););
    luaL_checktype(L, 2, LUA_TNUMBER);
    const char* data = lua_tostring(L, 1);
    int data_size = lua_tointeger(L, 2);

    char* pszUnCompressedFile = new char[data_size];

    LZ4_uncompress(data, pszUnCompressedFile, data_size);
    lua_pushlstring(L, pszUnCompressedFile, data_size);
    return 1;
}
auto getpropvalue(lua_State* L) -> int {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
    luaL_checktype(L, 2, LUA_TSTRING);

    if (!checkinstance(L, 1))
    {
        luaL_argerror(L, 1, "userdata<instance> expected");
        return 0;
    }

    lua_getfield(L, 1, lua_tolstring(L, 2, NULL));
    const char* data = lua_tolstring(L, -1, NULL); lua_pop(L, 2);


    lua_pushstring(L, data);
    return 1;
}

auto setpropvalue(lua_State* L) -> int {
    ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checkany(L, 3);

    if (!checkinstance(L, 1))
    {
        luaL_argerror(L, 1, "userdata<instance> expected");
        return 0;
    }

    lua_setfield(L, 1, lua_tolstring(L, 2, NULL));
    return 0;
}
auto compare_instances(lua_State* L) -> int {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
    luaL_checktype(L, 2, LUA_TUSERDATA);

    if (!checkinstance(L, 1))
    {
        luaL_argerror(L, 1, "userdata<instance> expected");
        return 0;
    }

    if (!checkinstance(L, 2))
    {
        luaL_argerror(L, 1, "userdata<instance> expected");
        return 0;
    }

    int instance_1 = dereference_pointer(lua_touserdata(L, 1));
    int instance_2 = dereference_pointer(lua_touserdata(L, 2));

    lua_pushboolean(L, (instance_1 == instance_2));
    return 1;
}
static const luaL_Reg env_funcs34[] = {
    {"getpropvalue", getpropvalue},
    {"checkinst", compare_instances},
     {"compareinstances", compare_instances},
    {"getpropvalue", setpropvalue},
    {"loadstring2", lua_loadstring},
    {"setclipboard", lua_setclipboard},
    {"toclipboard", lua_setclipboard},

    //{"lz4compress", lz4compress},
  //  {"lz4decompress", lz4decompress},

    {"identifyexecutor", lua_identifyexecutor},
    {"getexecutorname", lua_identifyexecutor},

    {"lz4compress", lz4compress},
    {"lz4decompress", lz4decompress},

    {"setfpscap", setfpscap},
    {"getfpscap", getfpscap},

    {"queue_on_teleport", queue_on_teleport},
    {"queueonteleport", queue_on_teleport},


    {NULL, NULL}
};

auto register_misc(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, env_funcs34);

    lua_pop(L, 1);
}