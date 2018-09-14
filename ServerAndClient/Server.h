#pragma once
#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define TIMER_START QueryPerformanceCounter(&t1);
#define TIMER_STOP 	QueryPerformanceCounter(&t2);

#define BUFFER 1024 
#define STR "File ended"

#ifdef linux
#include <unistd.h>
#include <netinet/in.h>  //
#include <netinet/tcp.h>
#include <netdb.h>  //
#include <memory.h> //
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
typedef int mySocket;
typedef unsigned int acceptInt;
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#pragma		comment (lib,"ws2_32.lib")
typedef SOCKET mySocket;
typedef int acceptInt;
#endif
using namespace std;

class Server
{
public:
	Server();
	~Server();

	void Clear();
	void CloseMySocket(mySocket);
	void Download(mySocket, char*);
	void Echo(mySocket, char*);
	mySocket GetConnect(mySocket);
	mySocket Initial();
	char** Parsing(char*);
	void TimeSend(mySocket);
	void Upload(mySocket, char*);
	void WorkServer(mySocket);
};

