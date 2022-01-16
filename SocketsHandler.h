#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <list>
#include "SocketClass.h"
#include <time.h>


namespace http {

	const TIMEVAL selectTimeOut = { 120,0 };

	class SocketsHandler {

	public:
		SocketsHandler();
		~SocketsHandler();
		void run();
		


	private:
		void addSocketsToSets();
		void useSelect();
		void handleSockets();
		void handleListenSocket();
		void handleReceiveSockets();
		void acceptConnection(SOCKET& mainSocket);
		void addSocket(SOCKET socket);
		void removeSockets();

		void debugPrintSockets();
	
		
		WSAData wsaData;
		sockaddr_in serverService;
		SOCKET listenSocket;
		list<SocketClass> socketsList;
		list<list<SocketClass>::iterator> socketsToRemove;
		

		fd_set waitRecv;
		fd_set waitSend;
		

	};
}
