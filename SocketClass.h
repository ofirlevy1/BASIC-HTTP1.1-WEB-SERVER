#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <queue>
#include"HTTPRequestProcessor.h"



namespace http{

	const int SERVER_PORT = 27015;
	const int MAX_SOCKETS = 60;
	const int LISTEN = 1;
	const int RECEIVE = 2;
	const int REQUEST = 3;
	const int SEND = 4;
	const int IDLE = 0;
	const int BUFFER_MAX_LEN = 2048;
	const int SOCKET_TIMEOUT = 120; //in seconds
	

	struct Status {
		int receiveStatus;
		int requestStatus;
		int sendStatus;

	};

	class SocketClass {
	public:
		SocketClass();
		SocketClass(SOCKET& otherSocket);
		Status getStatus() {return status;}
		SOCKET& getSocket() { return socket; }
		void setSendStatus(int newSendStatus) { status.sendStatus = newSendStatus; }
		void receiveMessage();
		void handleRequest();
		void sendMessage();
		bool isSocketTimeOut();
		

		bool operator==(const SocketClass& other) const;

		
	private:

		SOCKET socket;
		Status status;
		queue<string> requestsBuffers;
		queue<string> responseBuffers;
		time_t time_last_byte_received;


	};

}