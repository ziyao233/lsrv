#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#include<pthread.h>
#include<sys/types.h>
#include<sys/unistd.h>
#include<sys/socket.h>

#include"lsrv.h"

#define SOCKET_MASTER 0
#define SOCKET_WORKER 1

static struct {
	pthread_t thread;
	int socketPairs[2];
} *gWorkers;

static void *worker_body(void *arg)
{
	int workerId = ((uint8_t*)arg - (uint8_t*)gWorkers) / sizeof(*gWorkers);
	printf("Worker %d Hello\n",workerId);
	return NULL;
}

int lsrv_worker_start()
{
	gWorkers = malloc(sizeof(*gWorkers) * gLsrvConf.workerNum);
	if (!gWorkers)
		return -1;

	for (int i = 0;i < gLsrvConf.workerNum;i++) {
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
