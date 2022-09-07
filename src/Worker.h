/*
 *	lsrv
 *	File:/src/Worker.h
 *	Date:2022.09.07
 *	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#ifndef __LSRV_WORKER_H_INC__
#define __LSRV_WORKER_H_INC__

#include<stdlib.h>

#include<lua.h>

int lsrv_worker_start();
int lsrv_worker_master();

int lsrv_worker_write(lua_State *s,const void *buffer,size_t size);

#endif	// __LSRV_WORKER_H_INC__
