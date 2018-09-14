#include "stdafx.h"
#include "Client.h"
#include <string>

Client::Client()
{
	UniversalSocket s = Initial();
	if (s != -1)
	{
		WorkClient(s);
		CloseUniversalSocket(s);
	}
	else
	{
		cout << "application closed\npress any key" << endl;
		getchar();
	}
	Clear();
}


Client::~Client()
{
}

void Client::Clear()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void Client::CloseUniversalSocket(UniversalSocket s)
{
#ifdef WIN32
	closesocket(s);
#endif

#ifdef linux
	close(s);
#endif
}

void Client::Download(UniversalSocket s, char* path)
{
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
	cout << "Download finish" << endl;
	fclose(file);
}

int Client::GetRes(UniversalSocket s)
{
	char buffer[128];
	if (recv(s, buffer, sizeof(buffer), 0)<0)
	{
		cout << "Error retrieving data" << endl;
		return 0;
	}
	cout << "response from server: " << buffer << endl;
	return 1;
}

UniversalSocket Client::Initial()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#endif
	cout << "enter server adress dot10 encap " << endl;
	string inet_address = "";
	getline(cin, inet_address);
	cout << inet_address.c_str() << endl;
	struct sockaddr_in peer;
	peer.sin_family = AF_INET;
	peer.sin_port = htons(1280);
	inet_pton(AF_INET, inet_address.c_str(), &peer.sin_addr.s_addr);
	UniversalSocket s = socket(AF_INET, SOCK_STREAM, 0);
	if (s<0)
	{
		cout << "Socket Error" << endl;
		return -1;
	}

#ifdef linux

	int keepalive = 1;
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &keepalive, (socklen_t)sizeof(keepalive)) < 0)
	{

		perror("setsockopt(keepalive)");
	}
	int keepcnt = 1;
	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, (socklen_t)sizeof(int))<0)
	{

		perror("setsockopt(keepcnt)");
	}
	int keepidle = 30;
	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, (socklen_t)sizeof(int))<0)
	{

		perror("setsockopt(keepidle)");
	}
	int keepintvl = 1;
	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, (socklen_t)sizeof(int))<0)
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

	if (WSAIoctl(s, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &ret, NULL, NULL) == SOCKET_ERROR)
	{
		cout << "ERROR KEEPALIVE" << endl;
	}
#endif
	int con_res = connect(s, (struct sockaddr*) &peer, sizeof(peer));
	if (!(con_res < 0))
	{
		cout << "connected to server: " << inet_address.c_str() << endl;
		return s;
	}
	cout << "connect error" << endl;
	CloseUniversalSocket(s);
	return -1;
}

char** Client::Parsing(char* str)
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
	} while (str[i] != '\0');
	return response;
}

void Client::Upload(UniversalSocket s, char* path)
{
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
	cout << "\nupload finished" << endl;
	fclose(file);
}

void Client::WorkClient(UniversalSocket s)
{
	bool flag = true;
	do
	{
		char cmd[128];
		cout << "> ";
		fgets(cmd, 128, stdin);
		FILE* file = NULL;
		char** cmdp = Parsing(cmd);
		if (send(s, cmd, sizeof(cmd), 0)<0)
		{
			flag = false;
			cout << "problem with connection" << endl;
		}
		switch (cmdp[0][0])
		{
		case 'u': Upload(s, cmdp[1]);
			break;
		case 'c': flag = false; break;
		case 'd': Download(s, cmdp[1]); break;
		case 't':
		case 'e': GetRes(s); break;
		}
	} while (flag);
}
