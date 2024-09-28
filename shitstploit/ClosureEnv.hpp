#include <Offsets.hpp>
#include <lua.h>
#include <lualib.h>
#include <map>
#include <VM/src/lapi.h>
#include <VM/src/lgc.h>

std::map<int, int> CLOSURE_MAP = {};
#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T
/* custom functions */


std::map<Closure*, Closure*> newcclosure_map;
auto newcclosure_handler(lua_State* L) -> int {
    const auto nargs = lua_gettop(L);

    void* real_closure = reinterpret_cast<void*>(newcclosure_map.find(clvalue(L->ci->func))->second);

    if (real_closure == nullptr)
        return 0;

    L->top->value.p = real_closure;
    L->top->tt = LUA_TFUNCTION;
    L->top++;



    lua_insert(L, 1);

    const char* error;
    const auto res = lua_pcall(L, nargs, LUA_MULTRET, 0);

    if (res && res != LUA_YIELD && (error = lua_tostring(L, -1), !std::strcmp(error, "attempt to yield across metamethod/C-call boundary")))
    {
        return lua_yield(L, 0);
    }

    return lua_gettop(L);
}

int closure_count = 0;
auto newcclosure(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););

    if (lua_iscfunction(L, 1)) {
        lua_pushvalue(L, 1);
        return 1;
    }

    lua_ref(L, 1);
    lua_pushcclosure(L, newcclosure_handler, 0, 0);
    newcclosure_map[&luaA_toobject(L, -1)->value.gc->cl] = &luaA_toobject(L, 1)->value.gc->cl;

    return 1;
}

auto blank_func(lua_State* L) -> int {
    return 0;
}
auto random_string(int len) -> std::string {
    static const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZbcdefghijklmnopqrstuvwxyz";

    std::string str;
    str.reserve(len);

    for (int i = 0; i < len; ++i) {
        str += chars[rand() % (strlen(chars) - 1)];
    }

    return str;
}




auto getcallingscript(lua_State* L) -> int { /* this is for testing and it's temp */
    ARG_CHECK(L, 0, 0);

    lua_pushvalue(L, -10002);
    lua_getfield(L, -1, "script");

    return 1;
}

auto islclosure(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    lua_pushboolean(L, !lua_iscfunction(L, 1));
    return 1;
}

auto iscclosure(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    lua_pushboolean(L, lua_iscfunction(L, 1));
    return 1;
}
std::map<Closure*, Closure*> closureMap;
Closure* FindWrappedClosure(Closure* wrapper) { return closureMap.find(wrapper)->second; }
int NewCClosureHandler(lua_State* L) {
    const auto argc = lua_gettop(L);
 

    // Get og closure out of map from this newcclosure closure object.
    // then get the Value and thats the closure we gotta invoke.
    Closure* realClosure = FindWrappedClosure(clvalue(L->ci->func));

    if (realClosure == nullptr) {
        printf("Failed to execute newcclosure handler: Real closure not found.\r\n");
        return 0; // Failed to map, shit.
    }

    luaC_threadbarrier(L);
    L->top->value.p = realClosure;
    L->top->tt = LUA_TFUNCTION;
    L->top++; // Increase top

    lua_insert(L, 1);

    const auto callResult = lua_pcall(L, argc, LUA_MULTRET, 0);
    if (callResult != LUA_OK && callResult != LUA_YIELD &&
        std::strcmp(luaL_optstring(L, -1, ""), "attempt to yield across metamethod/C-call boundary") == 0) {
        printf("\r\n[Closures::NewCClosureHandler] The wrapped lua closure needs to yield!\r\n");
        return lua_yield(L, LUA_MULTRET);
    }

    if (callResult == LUA_ERRRUN)
        lua_error(
            L); // If we don't error, we will leave it as a string at the stack top. This will cause us to get dtc.

    return lua_gettop(L);
}
auto checkclosure(lua_State* L) -> int {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    if (const auto* cl = static_cast<const Closure*>(lua_topointer(L, 1)); cl->isC) {
        lua_pushboolean(L, NewCClosureHandler == cl->c.f || (cl->c.debugname == nullptr || cl->c.debugname == nullptr));
    }
    else {
        lua_pushboolean(L, cl->l.p->linedefined == -1);
    }
    return 1;
}

auto clonefunction(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    Closure* cl = (Closure*)lua_topointer(L, 1);

    lua_clonefunction(L, 1);

    return 1;
}

/* registering */
static const luaL_Reg env_funcs23[] = {
    {"newcclosure", newcclosure},

    {"getcallingscript", getcallingscript},
    {"islclosure", islclosure},
    {"iscclosure", iscclosure},

    {"checkclosure", checkclosure},
    {"isourclosure", checkclosure},
    {"isexecutorclosure", checkclosure},

    {"clonefunction", clonefunction},
    {"clonefunc", clonefunction},

    {NULL, NULL}
};

auto register_closure(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, env_funcs23);

    lua_pop(L, 1);
}