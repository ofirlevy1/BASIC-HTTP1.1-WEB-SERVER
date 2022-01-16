#pragma once
#include "HTTPServerConstantsAndTypes.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdio>
using namespace std;

class HTTPRequestProcessor
{

public:
	//HTTPRequestProcessor();
	HTTPRequestProcessor(string request);
	void interpretRequest();
	void debugPrintRequest();
	void handleRequest();
	string getHTTPResponse() { return HTTP_response_string;}

private:
	string requestStr;
	int currentIndex;
	Request request;
	Response response;
	string HTTP_response_string;
	bool requestParsedSuccessfuly;

	void readStatusLine();
	void skipWhiteSpaces();
	void skipLWSs();
	void readMethod();
	string readNextSubString();
	void readURL();
	void readHTTPVersion();
	void goToNextLine();
	void readHeaders();
	void readSingleHeader();
	string readAllDataUntillEndOfLine();
	bool isLWS();

	/*Service functions*/
	void Get();
	void Post();
	void Trace();
	void Delete();
	void Put();
	void Options();
	void Head();

	string getDate();
	void setResponseStatusLine(Status status);
	void setResponseBody(ifstream& fileStream);
	void updatePathAccordingToQuery();

	/*Options Methods helpers*/
	void OptionsEntireServer();
	void OptionsSpecificURL();

	/*creating the actual HTTP string response*/
	void createStringHttpResponse();
	void getStatusLineToResponseString();
	void getHeadersToResponseString();
	void getBodyToResponseString();
	Formats getFileFormat();
	void getFileToResponseBody(ifstream& fileStream);
};