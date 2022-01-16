#include "SocketClass.h"
#include <fstream>
using namespace std;
namespace http {
	SocketClass::SocketClass() {
		//waiting for...

		status.receiveStatus = RECEIVE;
		status.requestStatus = IDLE;
		status.sendStatus = IDLE;

		time(&time_last_byte_received);
	}
	

	SocketClass::SocketClass(SOCKET& otherSocket) : SocketClass()
	{
		socket = otherSocket;
	}

	/*This function is responsible for receiving a new messege. When a new messege is received, a snapshot of the internal clock is being taken.
	Taking the snapshot is important since the socket should be removed if it hasn't received a new messege for more than 2 minutes.
	When the messege is received, it is being pushed to a requests queue for further handling.*/
	void SocketClass::receiveMessage(){

		string eror;
		char tempBuffer[BUFFER_MAX_LEN];
		
		int bytesRecv = recv(socket, tempBuffer, sizeof(tempBuffer), 0);
		time(&time_last_byte_received);

		switch (bytesRecv) {
		case SOCKET_ERROR:
			eror = "DEBUG Server: Error at recv(): " + to_string(WSAGetLastError());
			closesocket(socket);
			throw eror;
			break;

		case 0:
			closesocket(socket);
			throw (string)"close socket";
			break;

		default:
			tempBuffer[bytesRecv] = '\0'; //add the null-terminating to make it a string
			cout << "Server: Recieved a " << bytesRecv << " byte message." << endl;;

			requestsBuffers.push(string(tempBuffer));
			status.requestStatus = REQUEST;
		}
	}

	/*This function pops out from the requests queue the next request, and using HTTPRequest processor to handle the request.
	The object HTTPRequestProcessor is responsible to handle the request according to the http protocol. When it finished to process the 
	request, the response is pushed to reponse queue for further handling. the sockets class's status is being updated to "SEND", which means
	that the socket class is ready for sending the response.*/
	void SocketClass::handleRequest()
	{
		/*Get the next request from the requests queue*/
		string response;
		HTTPRequestProcessor requestProcessor(requestsBuffers.front());
		requestsBuffers.pop();

		/*Handle the request and get a HTTP string response*/
		requestProcessor.interpretRequest();
		requestProcessor.handleRequest();
		response = requestProcessor.getHTTPResponse();

		/*Push the new response to responses queue and update the status to "Send"*/
		responseBuffers.push(response);
		status.sendStatus = SEND;

		if (requestsBuffers.empty())
			status.requestStatus = IDLE;

	}

	/*This function pops from the response queue the next response, and sending it to the client*/
	void SocketClass::sendMessage()
	{
		int bytesSent = 0;
		char response[BUFFER_MAX_LEN];

		string nextResponseInQueue = responseBuffers.front();


		strcpy_s(response, nextResponseInQueue.c_str());
	
		responseBuffers.pop();

		

		bytesSent = send(socket, response, (int)strlen(response), 0);
		if (bytesSent == SOCKET_ERROR)
		{
			string eror = "DEBUG - Server: Error at send(): " + to_string(WSAGetLastError());
			throw eror;
		}

		cout << "Server: Sent a " << bytesSent << "\\" << strlen(response) << " byte message." << endl;
		status.sendStatus = IDLE;
	}

	/*This function checks if a socket's timeout was reached(it compares the current clock to the snapshot that was taken when the last
	messege was received.)*/
	bool SocketClass::isSocketTimeOut()
	{
		time_t time_now;
		time(&time_now);

		if (time_now - time_last_byte_received > SOCKET_TIMEOUT)
			return true;

		return false;

		
	}

	bool SocketClass::operator==(const SocketClass& other) const
	{
		return (other.socket == socket);
	}
}