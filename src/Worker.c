/*
 *	lsrv
 *	File:/src/Worker.c
 *	Date:2022.10.28
 *	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#include<pthread.h>
#include<sys/types.h>
#include<sys/unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

#include<lua.h>
#include<lauxlib.h>

#include"lsrv.h"

#define SOCKET_MASTER	0
#define SOCKET_WORKER	1

/*	Trick: co's address + IDX_XXX stores the value	*/
#define IDX_CO		0
#define	IDX_CONNFD	1

static struct Worker {
	int sock;
	pthread_t thread;
	int socketPairs[2];
	Lsrv_Watcher *watcher;
} *gWorkers;

static struct Connection {
	int workerId;
	lua_State *co;
	uint8_t *buffer;
	size_t targetSize,sentSize;

	/*	Flags	*/
	unsigned int write:1;		// 0 for read,1 for write
} *gConnection;

static int get_conn_fd(lua_State *co)
{
	lua_rawgetp(co,LUA_REGISTRYINDEX,(uint8_t*)co + IDX_CONNFD);
	int fd = lua_tointeger(co,-1);
	lua_pop(co,1);
	return fd;
}

/*
 *	Do the stuff **in Lua side** needed when a coroutine dies
 */
static void on_die(lua_State *co)
{
	lua_pushnil(co);
	lua_rawsetp(co,LUA_REGISTRYINDEX,(uint8_t*)co + IDX_CO);
	return;
}

static void prepare_env(lua_State *s)
{
	lua_pushboolean(s,1);
	lua_setglobal(s,"LSRV_MAIN");

	luaL_requiref(s,"lsrv",luaopen_lsrv,1);

	lua_settop(s,0);

	return;
}

/*
 *	We assume that the handler function (in Lua side) is on the top in s
 */
static void accept_connection(lua_State *s,struct Worker *worker)
{
	int conn = accept(worker->sock,NULL,NULL);

	lua_State *co = lua_newthread(s);
	lua_rawsetp(s,LUA_REGISTRYINDEX,(void*)co);	// Hook the coroutine

	lua_pushinteger(co,conn);
	lua_rawsetp(co,LUA_REGISTRYINDEX,(uint8_t*)co + IDX_CONNFD);

	lua_pushvalue(s,-1);
	lua_xmove(s,co,1);

	gConnection[conn].workerId = worker - gWorkers;
	gConnection[conn].co	   = co;

	int retNum = 0;
	if (lua_resume(co,NULL,0,&retNum) == LUA_OK) {
		close(conn);
		on_die(co);
	} else {
		lua_pop(co,retNum);
	}
	return;
}

static void continue_connection(struct Worker *worker,int fd)
{
	struct Connection *conn = gConnection + fd;
	if (conn->write) {
		ssize_t size = write(fd,conn->buffer + conn->sentSize,
				     conn->targetSize - conn->sentSize);
		// FIXME: handler errors correctly
		lsrv_assert(size > 0);
		conn->sentSize += size;

		if (conn->sentSize == conn->targetSize) {
			lsrv_watcher_unwatch(worker->watcher,fd);

			int retNum = 0;
			if (lua_resume(conn->co,NULL,0,&retNum) == LUA_OK) {
				close(fd);
				on_die(conn->co);
			} else {
				lua_pop(conn->co,retNum);
			}
		}
	} else {
		abort();
	}
	return;
}

static void *worker_body(void *arg)
{
	int workerId = ((uint8_t*)arg - (uint8_t*)gWorkers) / sizeof(*gWorkers);
	printf("Worker %d Hello\n",workerId);

	struct Worker *worker = (struct Worker*)arg;

	worker->watcher = lsrv_watcher_new(gLsrvConf.maxConnection);
	if (!worker->watcher)
		return NULL;
	lsrv_watcher_watch(worker->watcher,worker->sock,LSRV_WATCHER_READ);

	int *readyList = malloc(sizeof(int) * gLsrvConf.maxConnection);
	int *errorList = malloc(sizeof(int) * gLsrvConf.maxConnection);
	if ((!readyList) || (!errorList))
		return NULL;

	lua_State *lState = luaL_newstate();
	if (!lState)
		return NULL;
	prepare_env(lState);

	// The handler is on the top
	luaL_dofile(lState,gLsrvConf.mainFile);

	while (1) {
		int readyNum = lsrv_watcher_wait(worker->watcher,
						 readyList,errorList,
						 gLsrvConf.maxConnection,NULL);
		for (int i = 0;i < readyNum;i++) {
			if (readyList[i] == worker->sock) {
				accept_connection(lState,worker);
			} else {
				continue_connection(worker,readyList[i]);
			}
		}
	}

	free(readyList);
	free(errorList);

	return NULL;
}

#define error(msg,...) fprintf(stderr,"Error: " msg,__VA_ARGS__)

#define SOCKETTYPE_IPV4_TCP	1
#define SOCKETTYPE_IPV6_TCP	2
#define SOCKETTYPE_IPV4_UDP	3
#define SOCKETTYPE_IPV6_UDP	4
#define SOCKETTYPE_UNIX		5

static int open_socket()
{
	int sock;
	if (gLsrvConf.socketType == SOCKETTYPE_IPV4_TCP) {
		sock = socket(AF_INET,SOCK_STREAM,0);
		if (sock < 0)
			return -1;

		struct sockaddr_in addr;
		addr.sin_family		= AF_INET;
		addr.sin_port		= htons(gLsrvConf.listenPort);
		if (inet_pton(AF_INET,gLsrvConf.listenIp,
			      &addr.sin_addr.s_addr) <= 0) {
			error("Cannot convert %s into IPv4 address\n",
			      gLsrvConf.listenIp);
			return -1;
		}

		if (bind(sock,(struct sockaddr*)(&addr),
			 sizeof(struct sockaddr)) < 0) {
			error("Cannot bind socket to %s:%d\n",
			      gLsrvConf.listenIp,gLsrvConf.listenPort);
			return -1;
		}
	} else if (gLsrvConf.socketType == SOCKETTYPE_IPV6_TCP) {
		sock = socket(AF_INET6,SOCK_STREAM,0);
		if (sock < 0)
			return -1;

		struct sockaddr_in6 addr;
		addr.sin6_family	= AF_INET6;
		addr.sin6_port		= htons(gLsrvConf.listenPort);
		if (inet_pton(AF_INET6,gLsrvConf.listenIp,
			      &addr.sin6_addr.s6_addr) <= 0) {
			error("Cannot convert %s into IPv6 address\n",
			      gLsrvConf.listenIp);
		}

		if (bind(sock,(struct sockaddr*)(&addr),
			 sizeof(struct sockaddr_storage)) < 0) {
			error("Cannot bind socket to %s:%d\n",
			      gLsrvConf.listenIp,gLsrvConf.listenPort);
			return -1;
		}
	} else {
		error("Unsupported socket type %d\n",gLsrvConf.socketType);
		return -1;
	}

	if (listen(sock,gLsrvConf.backlog) < 0) {
		error("Cannot listen to the socket %d\n",sock);
		return -1;
	}

	return sock;
}

int lsrv_worker_start()
{
	int sock = open_socket();
	if (sock < 0)
		return -1;

	gWorkers = malloc(sizeof(*gWorkers) * gLsrvConf.workerNum);
	if (!gWorkers)
		return -1;

	gConnection = malloc(sizeof(struct Connection) *
			     gLsrvConf.maxConnection);
	if (!gConnection)
		return -1;

	for (int i = 0;i < gLsrvConf.workerNum;i++) {
		gWorkers[i].sock = sock;
		if (socketpair(AF_UNIX,SOCK_STREAM,0,gWorkers[i].socketPairs))
			return -1;
		if (pthread_create(&gWorkers[i].thread,NULL,
				   worker_body,gWorkers + i))
			return -1;
	}

	return 0;
}

int lsrv_worker_master()
{
	puts("Master Hello");

	for (int i = 0;i < gLsrvConf.workerNum;i++)
		pthread_join(gWorkers[i].thread,NULL);

	return 0;
}

int lsrv_worker_write(lua_State *co,const void *data,size_t size)
{
	int fd = get_conn_fd(co);
	struct Connection *conn = gConnection + fd;
	conn->buffer	= (uint8_t*)data;
	conn->targetSize= size;
	conn->sentSize	= 0;
	conn->write	= 1;
	return lsrv_watcher_watch(gWorkers[conn->workerId].watcher,fd,
				  LSRV_WATCHER_WRITE);
}
