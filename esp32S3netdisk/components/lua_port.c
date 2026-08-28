#include "lua_port.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define LUA_OUTPUT_BUF_SIZE 4096
static char lua_output_buf[LUA_OUTPUT_BUF_SIZE];
static int lua_output_len = 0;

// 🔥 全局 Lua 虚拟机（用于 long 持久模式）
static lua_State *global_L = NULL;

// ===================== 修复：支持多参数 print =====================
int __lua_print(lua_State *L)
{
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++)
    {
        const char *str = lua_tostring(L, i);
        if (str)
        {
            int len = strlen(str);
            if (lua_output_len + len < LUA_OUTPUT_BUF_SIZE - 2)
            {
                strcat(lua_output_buf, str);
                strcat(lua_output_buf, " ");
                lua_output_len += len + 1;
            }
        }
    }
    strcat(lua_output_buf, "\n");
    lua_output_len += 1;
    return 0;
}

// 清空输出
void lua_output_reset(void)
{
    memset(lua_output_buf, 0, LUA_OUTPUT_BUF_SIZE);
    lua_output_len = 0;
}

// 获取结果
const char *lua_output_get(void)
{
    return lua_output_buf;
}

// ===================== 普通模式（每次新建销毁）=====================
int lua_run_with_arg(const char *lua_code, const char *arg)
{
    if (!lua_code)
        return -1;
    lua_output_reset();

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "print", __lua_print);

    if (arg)
    {
        lua_pushstring(L, arg);
        lua_setglobal(L, "ARG");
    }
    else
    {
        lua_pushstring(L, "");
        lua_setglobal(L, "ARG");
    }

    int res = luaL_dostring(L, lua_code);

    if (res != LUA_OK)
    {
        snprintf(lua_output_buf, LUA_OUTPUT_BUF_SIZE - 1, "错误: %s", lua_tostring(L, -1));
    }

    lua_close(L);
    return 0;
}

// ===================== 🔥 全局持久模式（新增）=====================
int lua_run_global(const char *lua_code)
{
    if (!lua_code)
        return -1;
    lua_output_reset();

    // 第一次使用：创建全局虚拟机
    if (!global_L)
    {
        global_L = luaL_newstate();
        luaL_openlibs(global_L);
        lua_register(global_L, "print", __lua_print);
    }

    // 执行代码（不关闭虚拟机！）
    int res = luaL_dostring(global_L, lua_code);

    if (res != LUA_OK)
    {
        snprintf(lua_output_buf, LUA_OUTPUT_BUF_SIZE - 1, "错误: %s", lua_tostring(global_L, -1));
    }

    return 0;
}