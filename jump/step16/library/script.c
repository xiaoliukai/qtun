#ifndef WIN32
#include <netinet/tcp.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <string.h>

#include <lualib.h>

#include "library.h"
#include "proto.h"
#include "script.h"

static void load_lib(lua_State* lua, const char* name, lua_CFunction func)
{
    luaL_requiref(lua, name, func, 1);
    lua_pop(lua, 1);
}

static int state_get(lua_State* lua)
{
    size_t len;
    const char* str = luaL_checklstring(lua, -1, &len);

    if (strcmp(str, "this_path") == 0)
        lua_pushstring(lua, qtun->this_path);
    else if (strcmp(str, "msg_ident") == 0)
        lua_pushinteger(lua, qtun->msg_ident);
    else if (strcmp(str, "msg_ttl") == 0)
        lua_pushinteger(lua, qtun->msg_ttl);
    else if (strcmp(str, "localip") == 0) {
        struct in_addr a = {qtun->localip};
        lua_pushstring(lua, inet_ntoa(a));
    } else if (strcmp(str, "netmask") == 0)
        lua_pushinteger(lua, qtun->netmask);
    else if (strcmp(str, "log_level") == 0)
        lua_pushinteger(lua, qtun->log_level);
    else if (strcmp(str, "little_endian") == 0)
        lua_pushboolean(lua, qtun->little_endian);
    else if (strcmp(str, "internal_mtu") == 0)
        lua_pushinteger(lua, qtun->internal_mtu);
    else if (strcmp(str, "use_udp") == 0)
        lua_pushboolean(lua, qtun->use_udp);
    else if (strcmp(str, "use_dhcp") == 0) {
        if (qtun->is_server)
            lua_pushboolean(lua, qtun->use_dhcp);
    } else if (strcmp(str, "aes_key") == 0) {
        if (qtun->aes_key_len)
            lua_pushlstring(lua, (const char*)qtun->aes_key, qtun->aes_key_len);
        else
            lua_pushnil(lua);
    } else if (strcmp(str, "aes_key_len") == 0)
        lua_pushinteger(lua, qtun->aes_key_len);
    else if (strcmp(str, "aes_iv") == 0) {
        if (qtun->aes_key_len)
            lua_pushlstring(lua, (const char*)qtun->aes_iv, sizeof(qtun->aes_iv));
        else
            lua_pushnil(lua);
    } else if (strcmp(str, "des_key") == 0) {
        if (qtun->des_key_len)
            lua_pushlstring(lua, (const char*)qtun->des_key, qtun->des_key_len);
        else
            lua_pushnil(lua);
    } else if (strcmp(str, "des_key_len") == 0)
        lua_pushinteger(lua, qtun->des_key_len);
    else if (strcmp(str, "des_iv") == 0) {
        if (qtun->des_key_len)
            lua_pushlstring(lua, (const char*)qtun->des_iv, sizeof(qtun->des_iv));
        else
            lua_pushnil(lua);
    } else if (strcmp(str, "compress") == 0)
        lua_pushinteger(lua, qtun->compress);
    else if (strcmp(str, "encrypt") == 0)
        lua_pushinteger(lua, qtun->encrypt);
    else if (strcmp(str, "is_server") == 0)
        lua_pushboolean(lua, qtun->is_server);
    else
        lua_pushnil(lua);
    return 1;
}

static int conf_get(lua_State* lua)
{
    library_conf_t* conf;
    const char* str;
    
    lua_getglobal(lua, "__conf__");
    conf = lua_touserdata(lua, -1);
    lua_pop(lua, 1);
    
    str = luaL_checkstring(lua, -1);
    if (strcmp(str, "conf_file") == 0)
        lua_pushstring(lua, conf->conf_file);
    else if (strcmp(str, "localip") == 0) {
        struct in_addr a = {conf->localip};
        lua_pushstring(lua, inet_ntoa(a));
    } else if (strcmp(str, "netmask") == 0)
        lua_pushnumber(lua, conf->netmask);
    else if (strcmp(str, "log_level") == 0)
        lua_pushnumber(lua, conf->log_level);
    else if (strcmp(str, "internal_mtu") == 0)
        lua_pushnumber(lua, conf->internal_mtu);
#ifdef WIN32
    else if (strcmp(str, "dev_symbol") == 0)
        lua_pushstring(lua, conf->dev_symbol);
    else if (strcmp(str, "dev_name") == 0)
        lua_pushstring(lua, conf->dev_name);
#endif
    else if (strcmp(str, "use_udp") == 0)
        lua_pushboolean(lua, conf->use_udp);
    else if (strcmp(str, "use_gzip") == 0)
        lua_pushboolean(lua, conf->use_gzip);
    else if (strcmp(str, "aes_file") == 0) {
        if (conf->use_aes)
            lua_pushstring(lua, conf->aes_key_file);
        else
            lua_pushnil(lua);
    } else if (strcmp(str, "des_file") == 0) {
        if (conf->use_des)
            lua_pushstring(lua, conf->des_key_file);
        else
            lua_pushnil(lua);
    } else if (strcmp(str, "use_dhcp") == 0) {
        lua_pushboolean(lua, conf->use_dhcp);
    } else if (strcmp(str, "signature_file") == 0) {
        lua_pushlstring(lua, (char*)conf->signature_file, sizeof(conf->signature_file));
    } else if (strcmp(str, "multi_pipe") == 0) {
        lua_pushnumber(lua, conf->multi_pipe);
    } else {
        lua_pushnil(lua);
    }
    return 1;
}

static int conf_set_and_check(lua_State* lua, library_conf_t* conf, const char* key, int input_type)
{
    int rc = 0;
    if (strcmp(key, "server") == 0) {
        rc = input_type == LUA_TSTRING;
        if (!rc) goto end;
        strncpy(conf->server, lua_tostring(lua, 3), sizeof(conf->server));
    } else if (strcmp(key, "server_port") == 0) {
        rc = input_type == LUA_TNUMBER;
        if (!rc) goto end;
        conf->server_port = lua_tonumber(lua, 3);
    } else if (strcmp(key, "localip") == 0) {
        rc = input_type == LUA_TSTRING;
        if (!rc) goto end;
        conf->localip = inet_addr(lua_tostring(lua, 3));
    } else if (strcmp(key, "netmask") == 0) {
        rc = input_type == LUA_TNUMBER;
        if (!rc) goto end;
        conf->netmask = lua_tonumber(lua, 3);
    } else if (strcmp(key, "log_level") == 0) {
        rc = input_type == LUA_TNUMBER;
        if (!rc) goto end;
        conf->log_level = lua_tonumber(lua, 3);
    } else if (strcmp(key, "internal_mtu") == 0) {
        rc = input_type == LUA_TNUMBER;
        if (!rc) goto end;
        conf->internal_mtu = lua_tonumber(lua, 3);
    } else if (strcmp(key, "use_udp") == 0) {
        rc = input_type == LUA_TBOOLEAN;
        if (!rc) goto end;
        conf->use_udp = lua_toboolean(lua, 3);
    } else if (strcmp(key, "use_gzip") == 0) {
        rc = input_type == LUA_TBOOLEAN;
        if (!rc) goto end;
        conf->use_gzip = lua_toboolean(lua, 3);
    } else if (strcmp(key, "aes_file") == 0) {
        size_t len;
        const char* str;
        rc = input_type == LUA_TSTRING;
        if (!rc) goto end;
        str = lua_tolstring(lua, 3, &len);
        if (len) {
            conf->use_aes = 1;
            strncpy(conf->aes_key_file, str, sizeof(conf->aes_key_file));
        }
    } else if (strcmp(key, "des_file") == 0) {
        size_t len;
        const char* str;
        rc = input_type == LUA_TSTRING;
        if (!rc) goto end;
        str = lua_tolstring(lua, 3, &len);
        if (len) {
            conf->use_des = 1;
            strncpy(conf->des_key_file, str, sizeof(conf->des_key_file));
        }
    } else if (strcmp(key, "use_dhcp") == 0) {
        rc = input_type == LUA_TBOOLEAN;
        if (!rc) goto end;
        conf->use_dhcp = lua_toboolean(lua, 3);
    } else if (strcmp(key, "signature_file") == 0) {
        size_t len;
        const char* str;
        rc = input_type == LUA_TSTRING;
        if (!rc) goto end;
        str = lua_tolstring(lua, 3, &len);
        if (len)
            strncpy((char*)conf->signature_file, str, sizeof(conf->signature_file));
    } else if (strcmp(key, "multi_pipe") == 0) {
        rc = input_type == LUA_TNUMBER;
        if (!rc) goto end;
        conf->multi_pipe = lua_tonumber(lua, 3);
        if (conf->multi_pipe < 1 || conf->multi_pipe > 10) {
            SYSLOG(LOG_ERR, "multi_pipe must equal or greater than 1 and equal or less than 10");
            rc = 0;
        }
    }
end:
    return rc;
}

static int conf_set(lua_State* lua)
{
    library_conf_t* conf;
    int type;
    
    const char* str = luaL_checkstring(lua, 2);
    
    lua_getglobal(lua, "__conf__");
    conf = lua_touserdata(lua, -1);
    lua_pop(lua, 1);
    
    type = lua_type(lua, 3);
    if (!conf_set_and_check(lua, conf, str, type))
    {
        const char* msg = lua_pushfstring(lua, "the param [%s] is not support, or type [%s] is not supported", str, lua_typename(lua, type));
        return luaL_argerror(lua, 1, msg);
    }
    return 0;
}

static int _syslog(lua_State* lua)
{
    lua_Number level = luaL_checknumber(lua, 1);
    const char* str = luaL_checkstring(lua, 2);
    SYSLOG(level, str, "");
    return 0;
}

static void get_qtun_obj(lua_State* lua)
{
    lua_getglobal(lua, "qtun");
    if (lua_isnil(lua, -1))
    {
        lua_pop(lua, 1);
        lua_newtable(lua);
        lua_setglobal(lua, "qtun");
        lua_getglobal(lua, "qtun");
    }
}

static void push_log_level(lua_State* lua, const char* name, lua_Number value)
{
    lua_pushstring(lua, name);
    lua_pushinteger(lua, value);
    lua_settable(lua, -3);
}

static void init_qtun_state(lua_State* lua)
{
    get_qtun_obj(lua);
    lua_pushstring(lua, "state");
    lua_newtable(lua);

    lua_newtable(lua);
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, state_get);
    lua_settable(lua, -3);
    lua_setmetatable(lua, -2);
    
    lua_settable(lua, -3);
    lua_pop(lua, 1);
}

static void init_qtun_conf(lua_State* lua)
{
    get_qtun_obj(lua);
    lua_pushstring(lua, "conf");
    lua_newtable(lua);
    
    lua_newtable(lua);
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, conf_get);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__newindex");
    lua_pushcfunction(lua, conf_set);
    lua_settable(lua, -3);
    lua_setmetatable(lua, -2);
    
    lua_settable(lua, -3);
    lua_pop(lua, 1);
}

static void init_qtun_log(lua_State* lua)
{
    int new_log = 0;
    
    get_qtun_obj(lua);
    lua_pushstring(lua, "log");
    lua_gettable(lua, -2);
    if (lua_isnil(lua, -1))
    {
        lua_pop(lua, 1);
        lua_pushstring(lua, "log");
        lua_newtable(lua);
        new_log = 1;
    }
    push_log_level(lua, "LOG_EMERG"  , 0);
    push_log_level(lua, "LOG_ALERT"  , 1);
    push_log_level(lua, "LOG_CRIT"   , 2);
    push_log_level(lua, "LOG_ERR"    , 3);
    push_log_level(lua, "LOG_WARNING", 4);
    push_log_level(lua, "LOG_NOTICE" , 5);
    push_log_level(lua, "LOG_INFO"   , 6);
    push_log_level(lua, "LOG_DEBUG"  , 7);
    if (new_log)
        lua_settable(lua, -3);
    lua_pop(lua, 1);
    
}

int script_global_init(lua_State* lua)
{
    load_lib(lua, "_G", luaopen_base);
    load_lib(lua, LUA_TABLIBNAME, luaopen_table);
    load_lib(lua, LUA_STRLIBNAME, luaopen_string);
    load_lib(lua, LUA_IOLIBNAME, luaopen_io);
    
    lua_pushcfunction(lua, _syslog);
    lua_setglobal(lua, "_syslog");
    
    init_qtun_state(lua);
    init_qtun_conf(lua);
    init_qtun_log(lua);
    return 1;
}

int script_load_config(lua_State* lua, library_conf_t* conf, const char* file_path)
{
    char path[MAX_PATH];
    lua_pushlightuserdata(lua, conf);
    lua_setglobal(lua, "__conf__");
    
    strcpy(path, qtun->this_path);
    strcat(path, "scripts/load_config.lua");
    if (luaL_dofile(lua, path) != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(lua, -1));
        lua_close(qtun->lua);
        exit(1);
    }
    return 1;
}

int script_signature_verify(lua_State* lua, const unsigned char signature[31]) {
    lua_getglobal(lua, "signature_verify");
    lua_pushlstring(lua, (char*)signature, 31);
    if (lua_pcall(lua, 1, 1, 0) != 0) {
        SYSLOG(LOG_ERR, "%s\n", lua_tostring(lua, -1));
        return 0;
    }
    return lua_toboolean(lua, -1);
}

int script_load_signature(lua_State* lua, unsigned char signature[31]) {
    const char* str;
    lua_getglobal(lua, "signature_load");
    if (lua_pcall(lua, 0, 1, 0) != 0) {
        SYSLOG(LOG_ERR, "%s\n", lua_tostring(lua, -1));
        return 0;
    }
    str = lua_tostring(lua, -1);
    memcpy(signature, str, 31);
    return 1;
}

