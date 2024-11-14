#pragma once
#ifndef HEADER_H
#define HEADER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //resolve conflict between windows.h and winsock2.h
#endif
#define DEFAULT_PORT "8080"

#include <iostream>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h> //for IP Helper API
#include <iphlpapi.h> //for IP Helper API

#pragma comment(lib, "Ws2_32.lib") //Winsock Library
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

struct ClientSocket {
	SOCKET clientSocket;
	addrinfo* serverAddr = NULL;
	char buffer[512];

	void GetHostName(char*& hostname) {
		const int length = 256;
		hostname = new char[256];
		if (gethostname(hostname, length) == SOCKET_ERROR) {
			cout << "gethostname failed: " << WSAGetLastError() << endl;
			WSACleanup();
			exit(1);
		}
	}
	void GetServerAddrInfo() {
		addrinfo hints;
		char* hostName = NULL;
		GetHostName(hostName);
		cout << "Host name: " << hostName << endl;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		if (getaddrinfo(hostName, DEFAULT_PORT, &hints, &serverAddr) != 0) {
			cout << "getaddrinfo failed: %d\n";
			WSACleanup();
			exit(1);
		}
	}
	void ConnectToServer() {
		if (connect(clientSocket, serverAddr->ai_addr, (int)serverAddr->ai_addrlen) == SOCKET_ERROR) {
			cout << "Connect failed: " << WSAGetLastError() << endl;
			freeaddrinfo(serverAddr);
			closesocket(clientSocket);
			WSACleanup();
		}
		freeaddrinfo(serverAddr); //No longer needed
	}
	void Send(const char* message) {
		send(clientSocket, message, strlen(message), 0);
		cout << "Message: " << message << endl;
	}
	void Receive() {
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			cout << "Message from server: " << buffer << endl;
		}
		else if (bytesReceived == 0) {
			cout << "Connection closed" << endl;
		}
		else {
			cout << "Receive failed: " << WSAGetLastError() << endl;
		}
	}
	void Close() {
		if (clientSocket != INVALID_SOCKET) {
			closesocket(clientSocket);
		}
	}
	ClientSocket() {
		GetServerAddrInfo();
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Socket creation failed: " << WSAGetLastError() << endl;
			WSACleanup();
		}
		ConnectToServer();
	}
};

void InitWinsock(WSADATA& wsadata) {
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed: %d\n";
		exit(1);
	}
}
void CloseWinsock() {
	WSACleanup();
}
#endif