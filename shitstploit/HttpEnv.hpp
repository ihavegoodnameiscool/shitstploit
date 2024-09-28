
#include "cpr/cpr.h"
#include <cpr/HttpStatus.hpp>
#include <lualib.h>
#include <lua.h>
#include <Offsets.hpp>

#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T




enum request_methods
{
	H_GET,
	H_HEAD,
	H_POST,
	H_PUT,
	H_DELETE,
	H_OPTIONS
};

std::map<std::string, request_methods> request_method_map =
{
	{ "get", H_GET },
	{ "head", H_HEAD },
	{ "post", H_POST },
	{ "put", H_PUT },
	{ "delete", H_DELETE },
	{ "options", H_OPTIONS }
};

std::map<uint32_t, std::string> http_cache = {};
#include <functional> // For std::hash

auto httpget(lua_State* L) -> int {
	// Argument check: expected 1 userdata, 1 string, and optionally 1 boolean
     // Get the base64-encoded string from Lua
    std::string url = luaL_checkstring(L, 1);

    // Use the decode_into template to decode the base64 string



    // Perform a GET request
    cpr::Response response = cpr::Get(cpr::Url{ url });
    // Push the decoded result to Lua
    lua_pushstring(L, response.text.c_str());
    return 1;  // Returning one value (decoded string)
	return 1;
}



auto httppost(lua_State* L) -> int {
	// Argument check: expected 1 userdata, 3 strings
	ARG_CHECK(L, 4, 4, luaL_checktype(L, 1, LUA_TUSERDATA););
	std::string url = luaL_checkstring(L, 2);
	std::string data = luaL_checkstring(L, 3);
	std::string content_type = luaL_checkstring(L, 4);

	// Validate URL protocol
	if (url.find("http") != 0) {
		luaL_error(L, "Invalid protocol specified (expected 'http://' or 'https://')");
		return 0;
	}

	// Perform HTTP POST request
	auto result = cpr::Post(cpr::Url{ url },
		cpr::Header{ {"User-Agent", "Roblox/WinInet"},
					  {"Content-Type", content_type} },
		cpr::Body{ data });

	// Check for HTTP errors
	if (HttpStatus::IsError(result.status_code)) {
		auto err = "Http Error " + std::to_string(result.status_code) + " - " + HttpStatus::ReasonPhrase(result.status_code);
		luaL_error(L, err.c_str());
		return 0;
	}

	// Push response text onto the Lua stack
	lua_pushlstring(L, result.text.c_str(), result.text.size());
	return 1;
}


auto request(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTABLE););

    lua_getfield(L, 1, "Url");
    if (lua_type(L, -1) != LUA_TSTRING) {
        luaL_error(L, "Invalid or no 'Url' field specified in request table");
        return 0;
    }

    std::string url = lua_tostring(L, -1);
    if (url.find("http") != 0) {
        luaL_error(L, "Invalid protocol specified (expected 'http://' or 'https://')");
        return 0;
    }
    lua_pop(L, 1);

    auto method = H_GET;
    lua_getfield(L, 1, "Method");
    if (lua_type(L, -1) == LUA_TSTRING) {
        std::string methods = luaL_checkstring(L, -1);
        std::transform(methods.begin(), methods.end(), methods.begin(), ::tolower);

        if (!request_method_map.count(methods)) {
            luaL_error(L, "Request type '%s' is not a valid HTTP request type.", methods.c_str());
            return 0;
        }
        method = request_method_map[methods];
    }
    lua_pop(L, 1);

    cpr::Header headers;
    lua_getfield(L, 1, "Headers");
    if (lua_type(L, -1) == LUA_TTABLE) {
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TSTRING) {
                luaL_error(L, "'Headers' table must contain string keys/values.");
                return 0;
            }
            std::string header_key = luaL_checkstring(L, -2);
            if (header_key == "Content-Length") {
                luaL_error(L, "Headers: 'Content-Length' header cannot be overwritten.");
                return 0;
            }
            std::string header_value = luaL_checkstring(L, -1);
            headers.insert({ header_key, header_value });
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    cpr::Cookies cookies;
    lua_getfield(L, 1, "Cookies");
    if (lua_type(L, -1) == LUA_TTABLE) {
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TSTRING) {
                luaL_error(L, "'Cookies' table must contain string keys/values.");
                return 0;
            }
            std::string cookie_key = luaL_checkstring(L, -2);
            std::string cookie_value = luaL_checkstring(L, -1);
            cookies[cookie_key] = cookie_value;
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    std::string body;
    lua_getfield(L, 1, "Body");
    if (lua_type(L, -1) == LUA_TTABLE) {
        if (method == H_GET || method == H_HEAD) {
            luaL_error(L, "'Body' cannot be present in GET or HEAD requests.");
            return 0;
        }
        size_t body_size;
        const char* body_cstr = luaL_checklstring(L, -1, &body_size);
        body.assign(body_cstr, body_size);
    }
    lua_pop(L, 1);

    // Execute the request
    cpr::Response response;
    switch (method) {
    case H_GET:
        response = cpr::Get(cpr::Url{ url }, cookies, headers);
        break;
    case H_HEAD:
        response = cpr::Head(cpr::Url{ url }, cookies, headers);
        break;
    case H_POST:
        response = cpr::Post(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
        break;
    case H_PUT:
        response = cpr::Put(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
        break;
    case H_DELETE:
        response = cpr::Delete(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
        break;
    case H_OPTIONS:
        response = cpr::Options(cpr::Url{ url }, cookies, headers);
        break;
    default:
        luaL_error(L, "Invalid request type.");
        return 0;
    }

    // Push the response back to Lua
    lua_newtable(L);
    lua_pushboolean(L, HttpStatus::IsSuccessful(response.status_code));
    lua_setfield(L, -2, "Success");
    lua_pushinteger(L, response.status_code);
    lua_setfield(L, -2, "StatusCode");
    lua_pushlstring(L, HttpStatus::ReasonPhrase(response.status_code).c_str(),
        HttpStatus::ReasonPhrase(response.status_code).size());
    lua_setfield(L, -2, "StatusMessage");

    // Push headers
    lua_newtable(L);
    for (const auto& header : response.header) {
        lua_pushlstring(L, header.first.c_str(), header.first.size());
        lua_pushlstring(L, header.second.c_str(), header.second.size());
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "Headers");

    // Push cookies
    lua_newtable(L);
    for (const auto& cookie : response.cookies.map_) {
        lua_pushlstring(L, cookie.first.c_str(), cookie.first.size());
        lua_pushlstring(L, cookie.second.c_str(), cookie.second.size());
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "Cookies");

    // Push response body
    lua_pushlstring(L, response.text.c_str(), response.text.size());
    lua_setfield(L, -2, "Body");

    return 1;
}


/* registering */

/* I have to fix request not resuming */
static const luaL_Reg env_funcs2[] = { // {"NAME", FUNC},
	{"httpget", httpget},
	{"httppost", httppost},
	{"httpgetasync", httpget},
	{"httppostasync", httppost},
	{"request", request},
	{"http_request", request},
	{NULL, NULL},
};

void register_http(lua_State* L)
{
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, env_funcs2);

    lua_pop(L, 1);

    lua_getglobal(L, "http");  // Push the "http" table to the stack

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);  // Pop the nil value
        lua_newtable(L);  // Create a new table for "http"
        lua_pushvalue(L, -1);  // Duplicate the "http" table
        lua_setglobal(L, "http");  // Set the global "http" = table
    }

    // Register the functions in the "http" table
    luaL_register(L, NULL, env_funcs2);  // Lua 5.2+ function to register them in "http"
}