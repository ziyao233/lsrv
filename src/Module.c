/*
 *	lsrv
 *	File:/src/Module.c
 *	Date:2022.09.04
 *	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<lua.h>
#include<lauxlib.h>

#include"lsrv.h"
#include"Module.h"

static int lsrv_write(lua_State *s);

static const struct luaL_Reg lsrvModule[] = {
		{"write",lsrv_write},
		{NULL,NULL},
	};

int luaopen_lsrv(lua_State *s)
{
	luaL_newlib(s,lsrvModule);
	return 1;
}

static int lsrv_writek(lua_State *s,int status,lua_KContext ctx)
{
	(void)s,(void)status,(void)ctx;
	return 0;
}

static int lsrv_write(lua_State *s)
{
	size_t size;
	const char *data = lua_tolstring(s,-1,&size);
	lsrv_worker_write(s,data,size);
	return lua_yieldk(s,0,0,lsrv_writek);
}
