#include "SocketsHandler.h"
using namespace std;

namespace http {
	/*Sockets handler constructor. it initialize the listen socket.*/
	SocketsHandler::SocketsHandler()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)!= NO_ERROR)
		{
			throw (string)"DEBUG - Server : Error at WSAStartup()";
			return;

		}

		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (listenSocket == INVALID_SOCKET)
		{
			string eror = "DEBUG - Server: Error at socket(): " + to_string(WSAGetLastError());
			WSACleanup();
			throw eror;
		}

		serverService.sin_family = AF_INET;
		serverService.sin_addr.s_addr = INADDR_ANY;
		serverService.sin_port = htons(SERVER_PORT);

		if (bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR)
		{
			string eror = "DEBUG - Server: Error at bind():" + to_string(WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			throw eror;
		}

		if (listen(listenSocket, 5)== SOCKET_ERROR)
		{

			string eror = "DEBUG -  Server: Error at listen(): " + to_string(WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			throw eror;
		}

		cout << "server is ready for requests" << endl;
		

	}
	SocketsHandler::~SocketsHandler()
	{
		// Closing connections and Winsock.
		cout << "DEBUG -  Server: Closing Connection." << endl;
		closesocket(listenSocket);
		WSACleanup();
	}

	/*run function is the main loop of the server. This loop is devided to 3 simple steps:
	1. Adding the sockets to receive set and send set according to their state
	2. Using select function and removing irrelevant sockets from sets
	3. Handeling the sockets that either received new message or send new message*/
	void SocketsHandler::run()
	{
		while (true) {

			FD_ZERO(&waitRecv);
			FD_ZERO(&waitSend);
			
			addSocketsToSets();
			useSelect();
			handleSockets();
		}
	}

	/*This function adds sockets to fd sets according to their status.
	1."RECEIVE" status means that the socket is ready to receive a new message.
	2."SEND" status means that the socket is ready to send a new response.*/
	void SocketsHandler::addSocketsToSets()
	{
		Status status;

		FD_SET(listenSocket, &waitRecv); //put the main socket in waitRecv

		auto itr = socketsList.begin();
		while (itr != socketsList.end()){

			status = itr->getStatus();

			if (status.receiveStatus == RECEIVE) {
				SOCKET& socket = itr->getSocket();
				FD_SET(socket, &waitRecv);
			}

			if (status.sendStatus == SEND) {
				SOCKET& socket = itr->getSocket();
				FD_SET(socket, &waitSend);
			}

			++itr;
		}

	}

	/*This function uses select function to remove from the fd sets the irelevant sockets*/
	void SocketsHandler::useSelect() {

		int nfd;

		//debugPrintSockets();

		nfd = select(0, &waitRecv, &waitSend, NULL, &selectTimeOut); 

		if (nfd == SOCKET_ERROR)
		{
			string eror = "DEBUG - Server: Error at select(): " + to_string(WSAGetLastError());
			WSACleanup();
			throw eror;
		}


	}

	/*This function is the main handler of the relevant sockets(sockets that has to perform one of the following: receive/handle request/ send.
	Listen socket is responsible for receiving new connections. A new socket is created for every new connection.*/
	void SocketsHandler::handleSockets() {

		handleListenSocket(); //check for new connections 
		handleReceiveSockets(); //handle the "regular" sockets
	}

	/*This function checks wether the listen socket is in receive fd set or not. If the socket has't removed from receive fd set after calling
	select function, that means that a new message has been received. hence, the listen socket is accepting the new connection*/
	void SocketsHandler::handleListenSocket()
	{
		if (FD_ISSET(listenSocket, &waitRecv)) {
			acceptConnection(listenSocket);
		}

	}

	/*This function handles the sockets according to their fd set status.
	After select function was called, only sockets that could perform their assigment are waiting in the fd set.
	"FD_ISSET" takes as arguments the socket and the fd set, and returns "true" only if the socket is in the fd set.
	This process is being perform for each active socket in the server (sockets are stored in "socketsList")*/
	void SocketsHandler::handleReceiveSockets()
	{
		
		int requestStatus;
		auto itr = socketsList.begin();

		while (itr != socketsList.end()) {
			SOCKET& socket = itr->getSocket();

			if (FD_ISSET(socket, &waitRecv)) {
				try {
					itr->receiveMessage();
				}

				catch (string eror) {
					
					socketsToRemove.push_back(itr);
					break;
				}

			}

			requestStatus = itr->getStatus().requestStatus;
			if (requestStatus == REQUEST) {
				try {
					itr->handleRequest();
				}

				catch (string eror) {
					
					socketsToRemove.push_back(itr);
					break;
				}
			}

			if (FD_ISSET(socket, &waitSend)) {
				itr->sendMessage();
			}


			if (itr->isSocketTimeOut()) {
				
				socketsToRemove.push_back(itr);
				break;
			}

			++itr;
		}
		removeSockets();
	}

	/*This function is responsible for accepting the new connection, which means generating a new socket and pushing is to "socketList", where the
	sockets are stored.*/
	void SocketsHandler::acceptConnection(SOCKET& mainSocket)
	{
		struct sockaddr_in from;		// Address of sending partner
		int fromLen = sizeof(from);

		
		SOCKET msgSocket = accept(mainSocket, (struct sockaddr*)&from, &fromLen);

		if (msgSocket == INVALID_SOCKET)
		{
			string eror = "DEBUG - Server: Error at accept(): " + to_string(WSAGetLastError());
			cout << eror;
		}

		cout << "Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

		unsigned long flag = 1;
		if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)  // Set the socket to be in non-blocking mode.
		{
			string eror = "DEBUG - Error at ioctlsocket(): " + to_string(WSAGetLastError());
			cout << eror;
		}

		addSocket(msgSocket);
	}
	void SocketsHandler::addSocket(SOCKET socket)
	{
		if (socketsList.size() == MAX_SOCKETS){

			throw(string)"\t\tToo many connections, dropped!";
			closesocket(socket);
		}
		
		SocketClass newSocketClass(socket);
		socketsList.push_back(newSocketClass);
	}

	/*This function is responsble for closing a socket and removing the socket from the socket list, where sockets are stored*/
	void SocketsHandler::removeSockets()
	{
		auto itr = socketsToRemove.begin();
		while (itr != socketsToRemove.end()){
			SOCKET& socket = (*itr)->getSocket();
			cout << "removing socket:" << socket <<endl;
			closesocket(socket);
			socketsList.erase(*itr);
			++itr;
		}

		socketsToRemove.clear();
	}

	/*This function is a debug function which prints the active sockets on server*/
	void SocketsHandler::debugPrintSockets()
	{
		cout << "==========" <<" Active SOCKETS on server: ==========" <<endl;
	
		cout << "Listen socket: " << ntohs(serverService.sin_port) << endl;
		cout << "Receive sockets: ";
		
		
		for (auto socket : socketsList) {
			cout << socket.getSocket() << ", ";
		}
		cout << endl;
		cout << "=======================================================" << endl;
	
	}
}
	