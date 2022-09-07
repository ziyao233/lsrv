/*
 *	lsrv
 *	File:/src/Module.h
 *	Date:2022.09.07
*	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#ifndef __LSRV_MODULE_H_INC__
#define __LSRV_MODULE_H_INC__

#include<lua.h>
#include<lauxlib.h>

int luaopen_lsrv(lua_State *s);

#endif	// __LSRV_MODULE_H_INC__
