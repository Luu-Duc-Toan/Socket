#include "ServerHeader.h"

int main() {
	WSADATA wsaData;
	InitWinsock(wsaData); 
	
	ServerSocket server;

	server.Listen();

	const char* welcomeMessage = "Hello, Client!";
	while (true) {
		server.Receive();
		char input = 'o';
		cin >> input;
		cout << input;
		if (input == ' ') break;
		if (input == 's') {
			server.Send(welcomeMessage);
		}
	}

	server.Close();
	CloseWinsock();
	return 0;
}