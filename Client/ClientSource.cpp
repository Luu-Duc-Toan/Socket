#include "ClientHeader.h"

int main() {
	WSADATA wsaData;
	InitWinsock(wsaData);

	ClientSocket client;

	while (true) {
		char input = 'o';
		cin >> input;
		if (input == ' ') break;
		if (input == 's') {
			client.Send("Hello Server");
		}
		client.Receive();
	}

	client.Close();
	CloseWinsock();
}