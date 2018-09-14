#include "stdafx.h"
#include "Server.h"

#define _CRT_SECURE_NO_WARNINGS 1

Server::Server()
{
	mySocket s = Initial();
	if (s < 0)
	{
		cout << "Error with socket initialization\npress any button to close application." << endl;
		getchar();
		return;
	}

	cout << "server is running" << endl;
	while (true) {
		mySocket s2 = GetConnect(s);
		if (s2<0)
		{
			cout << "failed connection attempt" << endl;
			continue;
		}
		WorkServer(s2);
		CloseMySocket(s2);
		cout << "mySocket closed" << endl;
	}
	CloseMySocket(s);
	Clear();
}


Server::~Server()
{
}

void Server::Clear()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void Server::CloseMySocket(mySocket s)
{
#ifdef WIN32
	closesocket(s);
#endif
#ifdef linux
	close(s);
#endif
}

void Server::Download(mySocket s, char* path)
{
	LARGE_INTEGER freq;
	LARGE_INTEGER t1, t2;
	QueryPerformanceFrequency(&freq);
	int getSize;
	int sizeFile;
	if (recv(s, (char*)(&sizeFile), sizeof(sizeFile), 0)<0)
	{
		cout << "Error retrieving data" << endl;
		return;
	}
	if (sizeFile == -1)
	{
		return;
	}

	FILE* file = nullptr;
	fopen_s(&file, path, "wb");

	cout << "Download file: " << path << endl;
	char* b = new char[BUFFER];
	int sizeDownload = 0;
	TIMER_START
		while (sizeDownload < sizeFile)
		{

			getSize = recv(s, b, BUFFER, 0);
			if (getSize < 0)
			{
				cout << "Error retrieving data" << endl;
				return;
			}
			sizeDownload += getSize;
			fwrite(b, sizeof(char), getSize, file);
		}
	TIMER_STOP;
	double elapsedTime1 = (float)(t2.QuadPart - t1.QuadPart) / freq.QuadPart;
	cout << "Download finish " << "time: " << elapsedTime1 << endl;
	fclose(file);
}

void Server::Echo(mySocket s, char* str)
{
	if (send(s, str, strlen(str) + 1, 0)<0)
	{
		cout << "Error sending data" << endl;
		return;
	}
}

mySocket Server::GetConnect(mySocket s)
{
	sockaddr_in remote_addr;
	acceptInt size = sizeof(remote_addr);
	mySocket s2 = accept(s, (struct sockaddr*)&remote_addr, &size);
	if (s2 < 0)
		return -1;
#ifdef linux
	int keepalive = 1;
	if (setsockopt(s2, SOL_SOCKET, SO_KEEPALIVE, &keepalive, (socklen_t)sizeof(keepalive)) < 0)
	{

		perror("setsockopt(keepalive)");
	}
	int keepcnt = 1;
	if (setsockopt(s2, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, (socklen_t)sizeof(int))<0)
	{

		perror("setsockopt(keepcnt)");
	}
	int keepidle = 30;
	if (setsockopt(s2, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, (socklen_t)sizeof(int))<0)
	{

		perror("setsockopt(keepidle)");
	}
	int keepintvl = 1;
	if (setsockopt(s2, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, (socklen_t)sizeof(int))<0)
	{
		perror("setsockopt(keepalive)");
	}
#endif
#ifdef WIN32
	tcp_keepalive alive;
	alive.onoff = 1;
	alive.keepaliveinterval = 3;
	alive.keepalivetime = 30000;
	DWORD ret;
	DWORD Res = WSAIoctl(s, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &ret, NULL, NULL);
	if (Res == SOCKET_ERROR)
	{
		cout << "error with keepalive" << endl;
	}
#endif
	char buf[64];
	inet_ntop(AF_INET, &remote_addr.sin_addr, (PSTR)(&buf), 64);
	cout << "client " << buf << endl;
	return s2;
}

mySocket Server::Initial()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#endif
	mySocket s = socket(AF_INET, SOCK_STREAM, 0);
	if (s<0)
	{
		return -1;
	}
	struct sockaddr_in local;
#ifdef linux
	memset((char*)&local, '\0', sizeof(local));
#endif
	local.sin_family = AF_INET;
	local.sin_port = htons(1280); //htons  - host to network short",ближн€€ св€зь." htonl - host to network long"дальн€€ св€зь.
								  //преобразование целый чисел в обратный пор€док байт дл€ протокола IP.
								  //номера портов от 1025-5000. т.к меньше 1025- резерв дл€ telnet и ftp. c большим дл€ других служб.
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	int c = bind(s, (struct sockaddr*)&local, sizeof(local));
	if (c < 0)
	{
		CloseMySocket(s);
		return -1;
	}
	int r = listen(s, 5);// 5 is limit value because of limits of winsocket ver1.1
	if (r < 0)
	{
		CloseMySocket(s);
		return -1;
	}

	return s;
}

char** Server::Parsing(char* str)
{
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
	return response;
}

void Server::TimeSend(mySocket s)
{
	char buffer[12];
	time_t seconds = time(NULL);
	tm timeinfo;
	localtime_s(&timeinfo, &seconds);
	strftime(buffer, 10, "%H:%M:%S", &timeinfo);
	send(s, buffer, sizeof(buffer), 0);
	cout << "Time sended on client" << endl;
}

void Server::Upload(mySocket s, char* path)
{
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
	fclose(file);
}

void Server::WorkServer(mySocket s)
{
	bool flag = true;
	do
	{
		char command[128];
		int getSize;
		getSize = recv(s, command, sizeof(command), 0);
		if (getSize == -1)
		{
			cout << "The mySocket on the client is closed for an unknown reason" << endl;
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
		}
	} while (flag);
	cout << "client closed mySocket" << endl;
}
