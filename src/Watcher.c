/*
 *	lsrv
 *	Date:2022.08.30
 *	File:/src/Watcher.c
 *	By MIT License.
 *	Copyright (c) 2022 Ziyao.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<unistd.h>
#include<sys/select.h>

#include"Debug.h"
#include"Watcher.h"

Lsrv_Watcher *lsrv_watcher_new(int maxNum)
{
	if (maxNum > FD_SETSIZE) {
		return NULL;
	}

	Lsrv_Watcher *w = (Lsrv_Watcher*)malloc(sizeof(Lsrv_Watcher));
	if (!w)
		return NULL;

	FD_ZERO(&(w->rSet));
	FD_ZERO(&(w->wSet));
	FD_ZERO(&(w->allSet));
	w->maxFd  = 0;
	w->maxNum = maxNum;

	return w;
}

int lsrv_watcher_watch(Lsrv_Watcher *w,int fd,int flag)
{
	if (fd > w->maxNum || !flag)
		return -1;

	if (FD_ISSET(fd,&(w->allSet)))
		return -1;

	if (flag & LSRV_WATCHER_READ)
		FD_SET(fd,&(w->rSet));

	if (flag & LSRV_WATCHER_WRITE)
		FD_SET(fd,&(w->wSet));

	FD_SET(fd,&(w->allSet));

	w->maxFd = w->maxFd > fd ? w->maxFd : fd;

	return 0;
}

/*
 *	function: lsrv_watcher_wait
 *	input:
 *		timeout		The max time of delayment in microsecond
 */
int lsrv_watcher_wait(Lsrv_Watcher *w,int *readyList,int *errorList,
		      int maxNum,int *timeout)
{
	lsrv_assert(w && readyList && errorList);

	fd_set wSet,rSet,errSet;

	memcpy(&wSet,&(w->wSet),sizeof(fd_set));
	memcpy(&rSet,&(w->rSet),sizeof(fd_set));
	memcpy(&errSet,&(w->allSet),sizeof(fd_set));

	struct timeval timeval,*timeoutPointer = NULL;
	if (timeout) {
		timeoutPointer = &timeval;
		timeval = (struct timeval) {
				.tv_sec = (time_t)(*timeout / 1000),
				.tv_usec = (suseconds_t)(*timeout % 1000),
			  };
	}

	int readyNum = select(w->maxFd + 1,&rSet,&wSet,&errSet,timeoutPointer);
	*timeout = timeval.tv_sec * 1000 + timeval.tv_usec;
	if (readyNum <= 0) {
		return 0;
	}

	int readyIndex = 0,errorIndex = 0;
	for (int testFd = 0;
	     testFd <= w->maxFd && readyIndex < maxNum;
	     testFd++) {
		if (FD_ISSET(testFd,&errSet)) {
			errorList[errorIndex] = testFd;
			errorIndex++;
		} else if (FD_ISSET(testFd,&wSet) || FD_ISSET(testFd,&rSet)) {
			readyList[readyIndex] = testFd;
			readyIndex++;
		}
	}

	errorList[errorIndex] = -1;

	return readyIndex;
}

void lsrv_watcher_unwatch(Lsrv_Watcher *w,int fd)
{
	FD_CLR(fd,&(w->rSet));
	FD_CLR(fd,&(w->wSet));
	FD_CLR(fd,&(w->allSet));
	if (fd == w->maxFd) {
		int newMax = w->maxFd - 1;
		while (!FD_ISSET(newMax,&(w->allSet)) && newMax > 0)
			newMax--;
		w->maxFd = newMax;
	}
	return;
}

void lsrv_watcher_destroy(Lsrv_Watcher *w)
{
	free(w);
	return;
}

int lsrv_watcher_resize(Lsrv_Watcher *w,int maxNum)
{
	(void)w;
	if (maxNum > FD_SETSIZE)
		return -1;
	return 0;
}
