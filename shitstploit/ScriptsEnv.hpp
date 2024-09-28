#include <lualib.h>
#include <lua.h>
#include <Offsets.hpp>
#include <VM/src/lmem.h>
#include <VM/src/ltable.h>

#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T

int getgc(lua_State * L) {

    const bool addTables = luaL_optboolean(L, 1, false);

    lua_createtable(L, 0, 0); // getgc table, prealloc sum space, because yes.

    typedef struct {
        lua_State* pLua;
        bool accessTables;
        int itemsFound;
    } GCOContext;

    auto gcCtx = GCOContext{ L, addTables, 0 };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
    const auto ullOldThreshold = L->global->GCthreshold;
    L->global->GCthreshold = SIZE_MAX;
    // Never return true. We aren't deleting shit.
    luaM_visitgco(L, &gcCtx, [](void* ctx, lua_Page* pPage, GCObject* pGcObj) -> bool {
        const auto pCtx = static_cast<GCOContext*>(ctx);
        const auto ctxL = pCtx->pLua;

        if (iswhite(pGcObj))
            return false; // The object is being collected/checked. Skip it.

        if (const auto gcObjType = pGcObj->gch.tt;
            gcObjType == LUA_TFUNCTION || gcObjType == LUA_TUSERDATA || gcObjType == LUA_TTABLE && pCtx->accessTables) {
            // Push copy to top of stack.
            ctxL->top->value.gc = pGcObj;
            ctxL->top->tt = gcObjType;
            ctxL->top++;

            // Store onto GC table.
            const auto tIndx = pCtx->itemsFound++;
            lua_rawseti(ctxL, -2, tIndx + 1);
        }
        return false;
        });
    L->global->GCthreshold = ullOldThreshold;
#pragma clang diagnostic pop

    return 1;
}

int setrawmetatable(lua_State* L) {
    luaL_argexpected(L,
        lua_istable(L, 1) || lua_islightuserdata(L, 1) || lua_isuserdata(L, 1) || lua_isbuffer(L, 1) ||
        lua_isvector(L, 1),
        1, "table or userdata or vector or buffer");

    luaL_argexpected(L, lua_istable(L, 2) || lua_isnil(L, 2), 2, "table or nil");

    lua_setmetatable(L, 1);
    return 0;
}



int gettenv(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTHREAD);
    const auto* otherL = static_cast<const lua_State*>(lua_topointer(L, 1));

    auto nT = luaH_clone(L, otherL->gt);
    L->top->tt = LUA_TTABLE;
    L->top->value.p = static_cast<void*>(nT);
    L->top++;
    return 1;
}
int gethui(lua_State* L) {

    lua_getglobal(L, "game");
    lua_getfield(L, -1, "CoreGui");
    return 1;
}

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


int compareinstances(lua_State* L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TUSERDATA);
  
    lua_pushboolean(L, *static_cast<const std::uintptr_t*>(lua_touserdata(L, 1)) ==
        *static_cast<const std::uintptr_t*>(lua_touserdata(L, 2)));

    return 1;
}
int setreadonly(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    lua_setreadonly(L, 1, lua_toboolean(L, 2));
    return 1;
}
int isreadonly(lua_State* L) {

    luaL_checktype(L, 1, LUA_TTABLE);
    lua_pushboolean(L, lua_getreadonly(L, 1));
    return 1;
}
auto checkcaller(lua_State* L) -> int {
    ARG_CHECK(L, 0, 0);
    const auto ExtraSpace = (uintptr_t)L->userdata;
    bool hawkTuah = *(__int64*)(ExtraSpace + Update::Userdata::Identity) > 3;
    lua_pushboolean(L, hawkTuah);
    return 1;
}
int getrawmetatable(lua_State* L) {
    luaL_checkany(L, 1);

    if (!lua_getmetatable(L, 1))
        lua_pushnil(L);

    return 1;
}
int getnamecallmethod(lua_State* L) {
    const auto szNamecall = lua_namecallatom(L, nullptr);

    if (szNamecall == nullptr) {
        lua_pushnil(L);
    }
    else {
        lua_pushstring(L, szNamecall);
    }

    return 1;
}
int isluau(lua_State* L) {
    lua_pushboolean(L, true);
    return 1;
}
int setnamecallmethod(lua_State* L) {
    luaL_checkstring(L, 1);
    if (L->namecall != nullptr)
        L->namecall = &L->top->value.gc->ts;

    return 0;
}

static const luaL_Reg script_envfuncs[] = {
    {"getthreadidentity", lua_getthreadidentity},
    {"getidentity", lua_getthreadidentity},
    {"getthreadcontext", lua_getthreadidentity},

    {"getrawmetatable", getrawmetatable},
     {"getnamecallmethod", getnamecallmethod},
     {"setnamecallmethod", setnamecallmethod},

    {"getgenv", getgenv},
    {"getreg", getreg},
    {"getsenv", getsenv},
    {"getgc", getgc},
    {"gettenv", gettenv},
    {"gethui", gethui},

    {"isluau", isluau},

    {"isreadonly", isreadonly},
    {"setreadonly", setreadonly},

     {"checkcaller", checkcaller},





    {"setrawmetatable", setrawmetatable},


    {"sethiddenproperty", sethiddenproperty},
    {"gethiddenproperties", gethiddenproperties},

    {"setthreadidentity", lua_setthreadidentity},
    {"setidentity", lua_setthreadidentity},
    {"setthreadcontext", lua_setthreadidentity},

     {"compareinstances", compareinstances},

    {"getobjects", getobjects},


    {NULL, NULL}
};

auto register_script(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, script_envfuncs);

    lua_pop(L, 1);
}