#include "stdafx.h"
#include "Server.h"

#define _CRT_SECURE_NO_WARNINGS 1

Server::Server()
{
	UniversalSocket s = Initialize();

	if (s < 0)
	{
		cout << "Error with socket initialization\npress any button to close application." << endl;
		getchar();
		return;
	}

	cout << "server is running" << endl;

	while (true) {
		UniversalSocket s2 = GetConnect(s);
		if (s2<0)
		{
			cout << "failed connection attempt" << endl;
			continue;
		}
		WorkServer(s2);
		CloseUniversalSocket(s2);
		cout << "UniversalSocket closed" << endl;
	}

	CloseUniversalSocket(s);

	Clear();
}


Server::~Server()
{
}

UniversalSocket Server::Initialize()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#endif

	UniversalSocket universalSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (universalSocket < 0)
	{
		return universalSocket;
	}

	struct sockaddr_in sockAddrIn;

#ifdef linux
	memset((char*)&sockAddrIn, '\0', sizeof(sockAddrIn));
#endif

	sockAddrIn.sin_family = AF_INET;
	sockAddrIn.sin_port = htons(1280);
	sockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

	int result;
	result = bind(universalSocket, (struct sockaddr*)&sockAddrIn, sizeof(sockAddrIn));

	if (result < 0)
	{
		CloseUniversalSocket(universalSocket);
		return -1;
	}

	result = listen(universalSocket, 5);	// 5 is limit value because of limits of winsocket ver1.1

	if (result < 0)
	{
		CloseUniversalSocket(universalSocket);
		return -1;
	}

	return universalSocket;
}

void Server::Clear()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void Server::CloseUniversalSocket(UniversalSocket universalSocket)
{
#ifdef WIN32
	closesocket(universalSocket);
#endif
#ifdef linux
	close(universalSocket);
#endif
}

void Server::Download(UniversalSocket universalSocket, char* path)
{
	LARGE_INTEGER freq;
	LARGE_INTEGER t1, t2;
	QueryPerformanceFrequency(&freq);
	int gottenSize;
	int fileSize;

	int result;
	result = recv(universalSocket, (char*)(&fileSize), sizeof(fileSize), 0);

	if (result < 0)
	{
		cout << "Error retrieving data" << endl;
		return;
	}

	if (fileSize == -1)
	{
		return;
	}

	FILE* file = nullptr;
	fopen_s(&file, path, "wb");

	cout << "Download file: " << path << endl;

	char* buffer = new char[BUFFER];
	int downloadedSize = 0;

	TIMER_START

		while (downloadedSize < fileSize)
		{
			gottenSize = recv(universalSocket, buffer, BUFFER, 0);

			if (gottenSize < 0)
			{
				cout << "Error retrieving data" << endl;
				return;
			}

			downloadedSize += gottenSize;
			fwrite(buffer, sizeof(char), gottenSize, file);
		}

	TIMER_STOP;

	double elapsedTime1 = (float)(t2.QuadPart - t1.QuadPart) / freq.QuadPart;

	cout << "Download finish " << "time: " << elapsedTime1 << endl;

	fclose(file);
}

void Server::Echo(UniversalSocket universalSocket, char* str)
{
	if (send(universalSocket, str, strlen(str) + 1, 0)<0)
	{
		cout << "Error sending data" << endl;
	}
}

UniversalSocket Server::GetConnect(UniversalSocket universalSize)
{
	sockaddr_in remoteAddr;
	acceptInt size = sizeof(remoteAddr);
	UniversalSocket s2 = accept(universalSize, (struct sockaddr*)&remoteAddr, &size);

	if (s2 < 0)
	{
		return -1;
	}
		
#ifdef linux
	int keepalive = 1;
	if (setsockopt(s2, SOL_SOCKET, SO_KEEPALIVE, &keepalive, (socklen_t)sizeof(keepalive)) < 0)
	{

		perror("setsockopt(keepalive)");
	}

	int keepcnt = 1;
	if (setsockopt(s2, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, (socklen_t)sizeof(int)) < 0)
	{

		perror("setsockopt(keepcnt)");
	}

	int keepidle = 30;
	if (setsockopt(s2, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, (socklen_t)sizeof(int)) < 0)
	{

		perror("setsockopt(keepidle)");
	}

	int keepintvl = 1;
	if (setsockopt(s2, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, (socklen_t)sizeof(int)) < 0)
	{
		perror("setsockopt(keepalive)");
	}
#endif

#ifdef WIN32
	tcp_keepalive tcpKeepAlive;
	tcpKeepAlive.onoff = 1;
	tcpKeepAlive.keepaliveinterval = 3;
	tcpKeepAlive.keepalivetime = 30000;
	DWORD ret;
	DWORD error = WSAIoctl(universalSize, SIO_KEEPALIVE_VALS, &tcpKeepAlive, sizeof(tcpKeepAlive), NULL, 0, &ret, NULL, NULL);

	if (error == SOCKET_ERROR)
	{
		cout << "error with keepalive" << endl;
	}
#endif

	char buf[64];
	inet_ntop(AF_INET, &remoteAddr.sin_addr, (PSTR)(&buf), 64);

	cout << "client " << buf << endl;

	return s2;
}



char** Server::Parsing(char* str)
{
	// USE COMMAND CENTER HERE
	/*
	char** response;
	response = new char*[3];

	int  i = 0;
	int count = 0;
	do
	{
		char* word = new char[BUFFER];
		int j = 0;
		while (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
		{
			word[j] = str[i];
			i++;
			j++;
		}
		word[j] = '\0';
		word = (char*)realloc(word, j + 1);
		response[count] = word;
		count++;
		i++;
	} while (str[i - 1] != '\0');
	return response;*/
}

void Server::TimeSend(UniversalSocket s)
{
	// USE COMMAND CENTER INSTEAD
	/*
	char buffer[12];
	time_t seconds = time(NULL);
	tm timeinfo;
	localtime_s(&timeinfo, &seconds);
	strftime(buffer, 10, "%H:%M:%S", &timeinfo);
	send(s, buffer, sizeof(buffer), 0);
	cout << "Time sended on client" << endl;*/
}

void Server::Upload(UniversalSocket s, char* path)
{
	// USE COMMAND CENTER INSTEAD
	/*
	LARGE_INTEGER freq;
	LARGE_INTEGER t1, t2;
	QueryPerformanceFrequency(&freq);
	char* outbuffer = new char[BUFFER + 1];
	char *b = new char[BUFFER];
	FILE* file = nullptr;
	fopen_s(&file, path, "rb");
	int sizeFile = -1;
	if (!file)
	{
		cout << "can't open file" << endl;
		if (send(s, (char*)(&sizeFile), sizeof(sizeFile), 0)<0)
		{
			fclose(file);
			cout << "Error sending data" << endl;
			return;
		}
		return;
	}

	fseek(file, 0, SEEK_END);
	sizeFile = ftell(file);
	if (send(s, (char*)(&sizeFile), sizeof(sizeFile), 0)<0)
	{
		fclose(file);
		cout << "Error sending data" << endl;
		return;
	}
	fseek(file, 0, SEEK_SET);
	cout << "upload is starting" << endl;
	int sizeUpload = 0;
	TIMER_START
		while (sizeUpload < sizeFile)
		{
			int size = fread(b, sizeof(char), BUFFER, file);

			if (send(s, b, size, 0)<0)
			{
				cout << "Error sending data" << endl;
				fclose(file);
				return;
			}
			sizeUpload += size;
			int per, per1 = -1;
			per = (int)((double)sizeUpload / (double)sizeFile * 100);
			if (per != per1)
			{
				cout << "\r" << per << "%";
				per1 = per;
			}
		}
	TIMER_STOP;
	double elapsedTime1 = (float)(t2.QuadPart - t1.QuadPart) / freq.QuadPart;
	cout << "\nupload finished " << "time:" << elapsedTime1 << endl;
	fclose(file);*/
}

void Server::WorkServer(UniversalSocket s)
{

	bool flag = true;
	do
	{
		// USE COMMAND CENTER HERE
		/*
		char command[128];
		int getSize;
		getSize = recv(s, command, sizeof(command), 0);
		if (getSize == -1)
		{
			cout << "The UniversalSocket on the client is closed for an unknown reason" << endl;
			return;
		}
		char** com = Parsing(command);
		switch (com[0][0])
		{
		case 'u': Download(s, com[1]); break;
		case 'c': flag = false; break;
		case 't': TimeSend(s); break;
		case 'd': Upload(s, com[1]); break;
		case 'e': Echo(s, com[1]); break;
		default: break;
		*/
		}
	} while (flag);
	cout << "client closed UniversalSocket" << endl;
}
