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

struct ServerSocket {
	SOCKET listenSocket;
	SOCKET clientSocket;
	addrinfo* serverAddr = NULL;
	char buffer[512];

	void GetServerAddrInfo() {
		addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &serverAddr) != 0) {
			cout << "getaddrinfo failed: %d\n";
			WSACleanup();
			exit(1);
		}
	}
	void SetSocketOption() {
		int opt = 1;
		if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
			cout << "Set socket options failed! Error: " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
		}
	}
	void BindSocket() {
		if (bind(listenSocket, serverAddr->ai_addr, (int)serverAddr->ai_addrlen)) {
			cout << "Bind failed: " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
			exit(1);
		}
		freeaddrinfo(serverAddr); //No longer needed
		serverAddr = NULL;
	}
	void Accept() {
		clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Accept failed: " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
		}
		cout << "Client connected!" << endl;
	}
	//May not properly for project ***
	void Shutdown() {
		// shutdown the connection
		if (shutdown(clientSocket, SD_SEND) == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
		}
	}
	void Listen() {
		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
			cout << "Listen failed: " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
		}
		cout << "Server is listening on port " << DEFAULT_PORT << endl;
		Accept();
	}
	void Send(const char* message) {
		if (send(clientSocket, message, (int)strlen(message), 0) == SOCKET_ERROR) {
			cout << "send failed: %d" << endl;
			WSAGetLastError();
			closesocket(clientSocket);
			WSACleanup();
		}
		cout << "Message: " << message << endl;
	}
	void Receive() {
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			cout << "Message from server: " << buffer << endl;
		}
		else if (bytesReceived == 0){
			cout << "Connection closed" << endl;
		}
		else {
			cout << "Receive failed: " << WSAGetLastError() << endl;
		}
	}
	void Close() {
		if (clientSocket != INVALID_SOCKET) {
			closesocket(clientSocket);
			clientSocket = INVALID_SOCKET;
		}
		if (listenSocket != INVALID_SOCKET) {
			closesocket(listenSocket);
			listenSocket = INVALID_SOCKET;
		}
	}
	ServerSocket() {
		GetServerAddrInfo();
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SetSocketOption();
		if (listenSocket == INVALID_SOCKET) {
			cout << "Socket creation failed: " << WSAGetLastError() << endl;
			WSACleanup();
		}
		BindSocket();
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