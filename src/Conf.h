/*
 *	lsrv
 *	File:/src/Conf.h
 *	Date:2022.09.07
 *	By MIT License
 *	Copyright (c) 2022 Ziyao.
 */

struct Lsrv_Conf {
	int listenPort;
	int workerNum;
	int backlog;
	int maxConnection;
	char *workPath;
	char *mainFile;
	char *listenIp;
};
