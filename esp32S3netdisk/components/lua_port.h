#ifndef LUA_PORT_H
#define LUA_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

    void lua_output_reset(void);
    const char *lua_output_get(void);
    int lua_run_with_arg(const char *lua_code, const char *arg);
    int lua_run_global(const char *lua_code);

#ifdef __cplusplus
}
#endif

#endif