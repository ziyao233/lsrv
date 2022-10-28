/*
 *	lsrv
 *	File:/src/Conf.h
 *	Date:2022.10.28
 *	By MIT License
 *	Copyright (c) 2022 Ziyao.
 */

struct Lsrv_Conf {
	int listenPort;
	int workerNum;
	int backlog;
	int maxConnection;
	int socketType;
	char *workPath;
	char *mainFile;
	char *listenIp;
};
