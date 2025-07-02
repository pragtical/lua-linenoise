/* vim:sts=4 sw=4 expandtab
 */

/*
* Copyright (c) 2011-2015 Rob Hoelz <rob@hoelz.ro>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is furnished
* to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise/linenoise.h"

#define LN_COMPLETION_TYPE "linenoiseCompletions*"

#ifdef _WIN32
#define LN_EXPORT __declspec(dllexport)
#else
#define LN_EXPORT extern
#endif

#ifndef LUA_OK
#define LUA_OK 0
#endif

static int completion_func_ref = LUA_NOREF;
static lua_State *completion_state;
static int callback_error_ref;

static int handle_ln_error(lua_State *L)
{
    lua_pushnil(L);
    return 1;
}

static int handle_ln_ok(lua_State *L)
{
    lua_pushboolean(L, 1);
    return 1;
}

static void completion_callback_wrapper(const char *line, linenoiseCompletions *completions)
{
    lua_State *L = completion_state;
    int status;

    lua_rawgeti(L, LUA_REGISTRYINDEX, completion_func_ref);
    *((linenoiseCompletions **) lua_newuserdata(L, sizeof(linenoiseCompletions *))) = completions;
    luaL_getmetatable(L, LN_COMPLETION_TYPE);
    lua_setmetatable(L, -2);

    lua_pushstring(L, line);

    status = lua_pcall(L, 2, 0, 0);

    if(status != LUA_OK) {
        lua_rawseti(L, LUA_REGISTRYINDEX, callback_error_ref);
    }
}

static int l_input(lua_State *L)
{
    const char *prompt = luaL_checkstring(L, 1);
    char *line;

    completion_state = L;
    lua_pushliteral(L, "");
    lua_rawseti(L, LUA_REGISTRYINDEX, callback_error_ref);
    line = linenoise(prompt);
    completion_state = NULL;

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_error_ref);
    if(strlen(lua_tostring(L, -1)) != 0) {
        lua_pushnil(L);
        lua_insert(L, -2);
        if(line) {
            free(line);
        }
        return 2;
    }

    if(! line) {
        return handle_ln_error(L);
    }
    lua_pushstring(L, line);
    free(line);
    return 1;
}

static int lines_next(lua_State *L)
{
    lua_pushcfunction(L, l_input);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_call(L, 1, 1);
    return 1;
}

static int l_lines(lua_State *L)
{
    luaL_checkstring(L, 1);
    lua_pushcclosure(L, lines_next, 1);
    return 1;
}

static int l_history_add(lua_State *L)
{
    const char *line = luaL_checkstring(L, 1);

    if(! linenoiseHistoryAdd(line)) {
        return handle_ln_error(L);
    }

    return handle_ln_ok(L);
}

static int l_history_set_max_len(lua_State *L)
{
    int len = luaL_checkinteger(L, 1);

    if(! linenoiseHistorySetMaxLen(len)) {
        return handle_ln_error(L);
    }

    return handle_ln_ok(L);
}

static int l_history_save(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);

    if(linenoiseHistorySave((char *) filename) < 0) {
        return handle_ln_error(L);
    }
    return handle_ln_ok(L);
}

static int l_history_load(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);

    if(linenoiseHistoryLoad((char *) filename) < 0) {
        return handle_ln_error(L);
    }
    return handle_ln_ok(L);
}

static int l_clear_screen(lua_State *L)
{
    linenoiseClearScreen();
    return handle_ln_ok(L);
}

static int l_set_completion(lua_State *L)
{
    if(lua_isnoneornil(L, 1)) {
        luaL_unref(L, LUA_REGISTRYINDEX, completion_func_ref);
        completion_func_ref = LUA_NOREF;
        linenoiseSetCompletionCallback(NULL);
    } else {
        luaL_checktype(L, 1, LUA_TFUNCTION);

        lua_pushvalue(L, 1);
        if(completion_func_ref == LUA_NOREF) {
            completion_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        } else {
            lua_rawseti(L, LUA_REGISTRYINDEX, completion_func_ref);
        }
        linenoiseSetCompletionCallback(completion_callback_wrapper);
    }

    return handle_ln_ok(L);
}

static int l_add_completion(lua_State *L)
{
    linenoiseCompletions *completions = *((linenoiseCompletions **) luaL_checkudata(L, 1, LN_COMPLETION_TYPE));
    const char *entry                 = luaL_checkstring(L, 2);

    linenoiseAddCompletion(completions, (char *) entry);

    return handle_ln_ok(L);
}

static int
l_set_multiline(lua_State *L)
{
    int is_multi_line = lua_toboolean(L, 1);

    linenoiseSetMultiLine(is_multi_line);

    return handle_ln_ok(L);
}

static int
l_print_keycodes(lua_State *L)
{
    linenoisePrintKeyCodes();
    return handle_ln_ok(L);
}

luaL_Reg linenoise_funcs[] = {
    { "input", l_input },
    { "add_history", l_history_add },
    { "set_history_max_len", l_history_set_max_len },
    { "save_history", l_history_save },
    { "load_history", l_history_load },
    { "clear_screen", l_clear_screen },
    { "set_completion", l_set_completion},
    { "add_completion", l_add_completion },
    { "set_multiline", l_set_multiline },
    { "print_keycodes", l_print_keycodes },
    { "lines", l_lines },
    { NULL, NULL }
};

luaL_Reg linenoise_methods[] = {
    { "add", l_add_completion },
    { NULL, NULL }
};

LN_EXPORT int luaopen_linenoise(lua_State *L)
{
    lua_pushliteral(L, "");
    callback_error_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_newtable(L);

    luaL_newmetatable(L, LN_COMPLETION_TYPE);
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

#if LUA_VERSION_NUM > 501
    luaL_setfuncs(L, linenoise_methods, 0);
    lua_pop(L, 1);
    luaL_setfuncs(L,linenoise_funcs,0);
#else
    luaL_register(L, NULL, linenoise_methods);
    lua_pop(L, 1);
    luaL_register(L, NULL, linenoise_funcs);
#endif
    return 1;
}
