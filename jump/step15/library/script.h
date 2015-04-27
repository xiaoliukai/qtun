#ifndef _LIBRARY_SCRIPT_H_
#define _LIBRARY_SCRIPT_H_

#include <lua.h>

#include "library.h"

extern int script_global_init(lua_State* lua);
extern int script_load_config(lua_State* lua, library_conf_t* conf, const char* file_path);
extern int script_signature_verify(lua_State* lua, const unsigned char signature[31]);

#endif

