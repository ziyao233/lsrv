/*
 *	lsrv
 *	File:/src/lsrv.c
 *	Date:2022.10.28
 *	By MIT License
 *	Copyright (c) 2022 Ziyao.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<lua.h>
#include<lauxlib.h>

#include"lsrv.h"

struct Lsrv_Conf gLsrvConf;

static int read_conf(const char *path)
{

#define TERM(name) {&gLsrvConf.name,#name}

	static const struct {
				int *pointer;
				const char *name;
		     } integerTerm[] = {
					TERM(workerNum),
					TERM(listenPort),
					TERM(listenPort),
					TERM(backlog),
					TERM(maxConnection),
					TERM(socketType),
					{NULL,NULL},
				     };
	static const struct {
				char **pointer;
				const char *name;
		     } stringTerm[] = {
					TERM(workPath),
					TERM(mainFile),
					TERM(listenIp),
					{NULL,NULL},
				    };
#undef TERM

	lua_State *s = luaL_newstate();
	if (!s)
		return -1;

	lua_pushboolean(s,1);
	lua_setglobal(s,"LSRV_CONF");

	int ret = -1;

	if (luaL_dofile(s,path)) {
		fprintf(stderr,"Error: Parsing Configuration\n%s",
			lua_tostring(s,-1));
		goto end;
	}

	if (lua_type(s,-1) != LUA_TTABLE) {
		fputs("Error: Return value is not a table",stderr);
		goto end;
	}

	for (int i = 0;integerTerm[i].pointer;i++) {
		int type = lua_getfield(s,-1,integerTerm[i].name);
		if (type == LUA_TNIL) {
			lua_pop(s,1);
			lua_pushinteger(s,1);
		} else if (type != LUA_TNUMBER) {
			fprintf(stderr,"Error: Configuration term %s requires "
					"a number\n",integerTerm[i].name);
			goto end;
		}
		*integerTerm[i].pointer = lua_tointeger(s,-1);
		lua_pop(s,1);
	}

	for (int i = 0;stringTerm[i].pointer;i++) {
		int type = lua_getfield(s,-1,stringTerm[i].name);
		if (type == LUA_TNIL) {
			lua_pop(s,1);
			lua_pushlstring(s,"",0);
		} else if (type != LUA_TSTRING) {
			fprintf(stderr,"Error: Configuration term %s requires "
					"a string\n",stringTerm[i].name);
			goto end;
		}
		*stringTerm[i].pointer = strdup(lua_tostring(s,-1));
		lua_pop(s,1);
	}
	ret = 0;

end:
	lua_close(s);

	return ret;
}

static void usage(const char *name)
{
	fprintf(stderr,"Usage:\n\t%s <CONFIGURATION>\n",name);
	return;
}

int main(int argc,const char *argv[])
{
	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}

	if (read_conf(argv[1])) {
		return -1;
	}

	printf("port: %d\n",gLsrvConf.listenPort);
	printf("workPath: %s\n",gLsrvConf.workPath);
	printf("workerNum: %d\n",gLsrvConf.workerNum);

	if (lsrv_worker_start()) {
		fputs("Cannot start workers\n",stderr);
		return -1;
	}

	lsrv_worker_master();

	return 0;
}
