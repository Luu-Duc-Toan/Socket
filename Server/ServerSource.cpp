#include "ServerHeader.h"

int main() {
	WSADATA wsaData;
	InitWinsock(wsaData); //Initialize Winsock
	
	ServerSocket server;

	server.Listen();

	const char* welcomeMessage = "Hello, Client!";
	while (true) {
		server.Receive();
		char input = 'o';
		cin >> input;
		if (input == ' ') break;
		if (input == 's') {
			server.Send(welcomeMessage);
		}
	}

	server.Close();
	CloseWinsock();
}