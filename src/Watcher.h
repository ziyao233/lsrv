/*
 *	lsrv
 *	File:/src/Watcher.h
 *	Date:2022.08.30
 *	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#ifndef __LSRV_WATCHER_H_INC__
#define __LSRV_WATCHER_H_INC__

typedef struct {
	int maxNum;
	int maxFd;
	fd_set rSet,wSet,allSet;
} Lsrv_Watcher;

#define LSRV_WATCHER_NULL 0x00
#define LSRV_WATCHER_READ 0x01
#define LSRV_WATCHER_WRITE 0x02

Lsrv_Watcher *lsrv_watcher_new(int maxNum);
int lsrv_watcher_watch(Lsrv_Watcher *watcher,int fd,int flag);
int lsrv_watcher_wait(Lsrv_Watcher *watcher,int *readyList,
		       int *errorList,int maxNum,int *timeout);
void lsrv_watcher_unwatch(Lsrv_Watcher *watcher,int fd);
void lsrv_watcher_destory(Lsrv_Watcher *watcher);
int lsrv_watcher_resize(Lsrv_Watcher *watcher,int maxNum);

#endif	// __LSRV_WATCHER_H_INC__
