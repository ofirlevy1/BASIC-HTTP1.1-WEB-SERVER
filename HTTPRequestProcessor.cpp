#include "HTTPRequestProcessor.h"

HTTPRequestProcessor::HTTPRequestProcessor(string request) : requestStr(request), currentIndex(0)
{
	/*inserting the constants headers in http response*/
	response.headers.insert({ CONTENT_LENGTH, "0" });
}

/*This function interprets the request. It takes the request (that was received as a stirng), and stores the data in the relevat objects.*/
void HTTPRequestProcessor::interpretRequest()
{
	setResponseStatusLine({ "Bad Request", 400 });
	char numStr[10];
	try
	{
		readStatusLine();
		goToNextLine();
		readHeaders();
		goToNextLine();
		if (currentIndex < requestStr.length())
		{
			request.body = requestStr.substr(currentIndex, requestStr.length() - currentIndex);
		}
	}
	catch (string str)
	{
		response.body = "Server: Error when parsing the HTTP request: \n";
		response.body += str;
		response.headers[CONTENT_LENGTH] = _itoa(response.body.length(), numStr, 10);
		createStringHttpResponse();
		requestParsedSuccessfuly = false;
		return;
	}
	catch (...)
	{
		createStringHttpResponse();
		requestParsedSuccessfuly = false;
		return;
	}
	requestParsedSuccessfuly = true;
	

}
/*This function is respobsible for interpreting the headers fields that were received in the http message. It stores that data in the relevant objects.*/
void HTTPRequestProcessor::readHeaders()
{
	skipWhiteSpaces();
	bool headersFinished = false;

	while (!headersFinished)
	{
		readSingleHeader();
		goToNextLine();
		if (currentIndex > requestStr.length() - 1)
			throw string("Input ended after <header>CRLF! after headers section, there should be CRLFCRLF!");
		if (requestStr[currentIndex] == CR)
			headersFinished = true;
	}
}

void HTTPRequestProcessor::readSingleHeader()
{
	skipWhiteSpaces();
	int headerIndex = currentIndex;

	while (requestStr[headerIndex] != ':')
	{
		headerIndex++;
	}

	string headerName(requestStr.substr(currentIndex, headerIndex - currentIndex));
	currentIndex += headerIndex - currentIndex + 1;

	skipWhiteSpaces();
	skipLWSs();

	string headerData = readAllDataUntillEndOfLine();

	while (isLWS())
	{
		headerData += CR;
		headerData += LF;
		headerData += ' ';
		currentIndex += 2;
		skipWhiteSpaces();
		headerData += readAllDataUntillEndOfLine();
	}

	request.headers[headerName] = headerData;
}

string HTTPRequestProcessor::readAllDataUntillEndOfLine()
{
	skipWhiteSpaces();
	int index = currentIndex;
	if (index > requestStr.length() - 1)
		throw string("Expected end-of-line (CRLF) after an header field!");
	while (requestStr[index] != CR)
	{
		index++;
		if (index > requestStr.length() - 1)
			throw string("Expected end-of-line (CRLF) after an header field!");
	}
		
	string res(requestStr.substr(currentIndex, index - currentIndex));
	currentIndex += index - currentIndex;
	return res;
}

void HTTPRequestProcessor::goToNextLine()
{
	skipWhiteSpaces();

	if (currentIndex + 1 > requestStr.length() - 1)
		throw string("Expected end-of-line (CRLF) - before end of input!");
	if (requestStr[currentIndex] != CR)
		throw string("expected a new line!");
	currentIndex++;
	if (requestStr[currentIndex] != LF)
		throw string("expected a new line!");
	currentIndex++;
}
/*This function is respobsible for interpreting the status line. It stores the data in the relevant objects.*/
void HTTPRequestProcessor::readStatusLine()
{
	skipWhiteSpaces();
	readMethod();
	if (!(requestStr[currentIndex] == ' '))
		throw string("no seperator between status line items!");
	currentIndex++;
	readURL();
	if (!(requestStr[currentIndex] == ' '))
		throw string("no seperator between status line items!");
	currentIndex++;
	readHTTPVersion();
}

void HTTPRequestProcessor::readHTTPVersion()
{
	skipWhiteSpaces();
	string version = readNextSubString();
	if (version.compare("HTTP/1.1") != 0)
	{
		setResponseStatusLine({ "HTTP Version Not Supported", 505 });
		throw string("server only supports version HTTP/1.1! version in request was: " + version);
	}
	request.requestLine.version = Version::httpDot1;
}

void HTTPRequestProcessor::readURL()
{
	skipWhiteSpaces();
	string uriStr = readNextSubString();

	//if url is empty:
	if ((uriStr[0] == '*' && uriStr.length() == 1) || (uriStr[0] == '/' && uriStr.length() == 1)) //changed from '\\' to /
	{
		request.requestLine.uri.isEmpty = true;
		request.requestLine.uri.isQuery = false;
		request.requestLine.uri.path = "*";
		return;
	}
	request.requestLine.uri.isEmpty = false;

	// otherwise, if it doesn't have a Query string (no ? character) :
	if (uriStr.find_first_of('?') == string::npos)
	{
		request.requestLine.uri.path = uriStr;
		request.requestLine.uri.isQuery = false;
		return;
	}

	//if it does have a query:
	request.requestLine.uri.isQuery = true;
	int queryIndex = uriStr.find_first_of('?') + 1;
	//making sure the query parameter is "lang" (only one supported by server right now)
	if (uriStr.substr(queryIndex, 4).compare(LANG) != 0)
	{
		throw string("Qeury string parameter <" + uriStr.substr(queryIndex, 4) + "> unsupported! (system only supports \"lang\"");
	}


	request.requestLine.uri.query.param = QueryParam::lang;
	queryIndex += 4;
	if (uriStr[queryIndex] != '=')
		throw string("the sign after parameter lang was not '=' !");

	queryIndex++;
	//now checking the speciefic language
	if (uriStr.substr(queryIndex, 2).compare("he") == 0 || uriStr.substr(queryIndex, 2).compare("HE") == 0)
		request.requestLine.uri.query.val = "HE";
	else if (uriStr.substr(queryIndex, 2).compare("en") == 0 || uriStr.substr(queryIndex, 2).compare("EN") == 0)
		request.requestLine.uri.query.val = "EN";
	else if (uriStr.substr(queryIndex, 2).compare("fr") == 0 || uriStr.substr(queryIndex, 2).compare("FR") == 0)
		request.requestLine.uri.query.val = "FR";
	else
		throw string("the requested language is unparsable/unsupported");

	//filling the IRI field with the string NOT INCLUDING the query part:
	request.requestLine.uri.path = uriStr.substr(0, uriStr.find_first_of('?'));
}

void HTTPRequestProcessor::skipLWSs()
{
	while (isLWS())
	{
		goToNextLine();
		currentIndex++;
	}
}

bool HTTPRequestProcessor::isLWS()
{
	if (currentIndex + 2 > requestStr.length() - 1)
		return false;
	if (requestStr[currentIndex] == CR && requestStr[currentIndex + 1] == LF
		&& (requestStr[currentIndex + 2] == ' ' || requestStr[currentIndex + 2] == '\t'))
	{
		return true;
	}
	return false;
}

void HTTPRequestProcessor::skipWhiteSpaces()
{
	while (requestStr[currentIndex] == ' ' || requestStr[currentIndex] == '\t')
	{
		currentIndex++;
	}
}

void HTTPRequestProcessor::readMethod()
{
	string methodName = readNextSubString();
	if (methodName.compare(OPTIONS) == 0)
		request.requestLine.method = serviceMethod::Options;
	else if (methodName.compare(GET) == 0)
		request.requestLine.method = serviceMethod::Get;
	else if (methodName.compare(HEAD) == 0)
		request.requestLine.method = serviceMethod::Head;
	else if (methodName.compare(POST) == 0)
		request.requestLine.method = serviceMethod::Post;
	else if (methodName.compare(PUT) == 0)
		request.requestLine.method = serviceMethod::Put;
	else if (methodName.compare(_DELETE) == 0)
		request.requestLine.method = serviceMethod::Delete;
	else if (methodName.compare(TRACE) == 0)
		request.requestLine.method = serviceMethod::Trace;
	else
		throw string("method name <" + methodName + "> not supported/unparsable");
}

string HTTPRequestProcessor::readNextSubString()
{
	string subString;
	skipWhiteSpaces();
	int startIndex = currentIndex;

	while (requestStr[currentIndex] != ' ' && requestStr[currentIndex] != '\t'
		&& requestStr[currentIndex] != CR)
	{
		currentIndex++;
	}

	return requestStr.substr(startIndex, currentIndex - startIndex);
}

/*This debug function prints the data that was extracted from the request.*/
void HTTPRequestProcessor::debugPrintRequest()
{
	cout <<
		"Method: " << request.requestLine.method << endl
		<< "URI (without query): " << request.requestLine.uri.path << endl;
	if (!request.requestLine.uri.isQuery)
		cout << "there are no queries" << endl;
	else
		cout << "query: " << request.requestLine.uri.query.param << " = " << request.requestLine.uri.query.val << endl;

	map<string, string>::iterator iter = request.headers.begin();
	cout << "There are " << request.headers.size() << " headers: " << endl;
	for (; iter != request.headers.end(); ++iter)
	{
		cout << iter->first << ": " << iter->second << endl;
	}

	cout << "body:" << endl;
	cout << request.body << endl;
}

/*This function is responsible for handling the request according to http 1.1 protocol.*/
void HTTPRequestProcessor::handleRequest()
{
	if (!requestParsedSuccessfuly)
		return;
	
	switch (request.requestLine.method)
	{
	case serviceMethod::Get:
		Get();
		break;
	case serviceMethod::Post:
		Post();
		break;
	case serviceMethod::Trace:
		Trace();
		break;
	case serviceMethod::Delete:
		Delete();
		break;
	case serviceMethod::Put:
		Put();
		break;
	case serviceMethod::Options:
		Options();
		break;
	case serviceMethod::Head:
		Head();
		break;
	}

	createStringHttpResponse();
	
}


/*This function gets the current date and stores it in a string, according to http date format*/
string HTTPRequestProcessor::getDate()
{
	string timeInFormat;
	time_t curr_time;
	curr_time = time(NULL);

	tm* tm_gmt = gmtime(&curr_time);

	timeInFormat.append(WeekDays[tm_gmt->tm_wday +1]);
	timeInFormat.append(", ");
	timeInFormat.append(to_string(tm_gmt->tm_mday));
	timeInFormat.append(" ");
	timeInFormat.append(Months[tm_gmt->tm_mon]);
	timeInFormat.append(" ");
	timeInFormat.append(to_string(tm_gmt->tm_year + 1900));
	timeInFormat.append(" ");
	timeInFormat.append(to_string(tm_gmt->tm_hour + 3));
	timeInFormat.append(":");
	timeInFormat.append(to_string(tm_gmt->tm_min));
	timeInFormat.append(":");
	timeInFormat.append(to_string(tm_gmt->tm_sec));
	timeInFormat.append(" ");
	timeInFormat.append("GMT");
	return timeInFormat;
}

void HTTPRequestProcessor::setResponseStatusLine(Status status)
{
	response.statusLine.version = Version::httpDot1;
	response.statusLine.status = status;
}

/*This function receives a file stream as an argument, and copies the file's data to the response body*/
void HTTPRequestProcessor::getFileToResponseBody(ifstream& fileStream)
{
	char c;
	int count = 0;

	while (!fileStream.eof()) 
	{
		fileStream.get(c);
		response.body.push_back(c);
		count++;
	}
	response.headers[CONTENT_LENGTH] = to_string(count);
}

/*This function updates the file's path according to the request's query. The specific file is derived from the query.*/
void HTTPRequestProcessor::updatePathAccordingToQuery()
{
	if (request.requestLine.uri.query.val.compare("EN") == 0) {
		request.requestLine.uri.path = "/EN" + request.requestLine.uri.path;
	}

	else if (request.requestLine.uri.query.val.compare("FR") == 0) {
		request.requestLine.uri.path = "/FR" + request.requestLine.uri.path;
	}

	else if (request.requestLine.uri.query.val.compare("HE") == 0) {
		request.requestLine.uri.path = "/HE" + request.requestLine.uri.path;
	}

}

/*This function is responsible for getting the data from the response objects and storing the data in a string, which will be sent further to the client.*/
void HTTPRequestProcessor::createStringHttpResponse()
{
	if(response.body.length() != 0 && response.headers.count(CONTENT_TYPE) == 0)
		response.headers[CONTENT_TYPE] = "text/html";
	//=======Create the Status line=========//
	getStatusLineToResponseString();
	HTTP_response_string.push_back(CR);
	HTTP_response_string.push_back(LF);

	//=======Create the headers=========//
	getHeadersToResponseString();
	HTTP_response_string.push_back(CR);
	HTTP_response_string.push_back(LF);

	//=======Create the body=========//
	getBodyToResponseString();
	if (response.body.empty())
		return;
	HTTP_response_string.push_back(CR);
	HTTP_response_string.push_back(LF);
}

void HTTPRequestProcessor::getStatusLineToResponseString()
{
	
	switch (response.statusLine.version)
	{
	case Version::httpDot0:
		HTTP_response_string.append(HTTPDOT0);
		break;

	case Version::httpDot1:
		HTTP_response_string.append(HTTPDOT1);
		break;
	}
	HTTP_response_string.append(" ");
	HTTP_response_string.append(to_string(response.statusLine.status.code));
	HTTP_response_string.append(" ");
	HTTP_response_string.append(response.statusLine.status.name);
	
}

void HTTPRequestProcessor::getHeadersToResponseString()
{
	auto itr = response.headers.begin();
	while (itr != response.headers.end()) {
		HTTP_response_string.append(itr->first);
		HTTP_response_string.append(": ");
		HTTP_response_string.append(itr->second);
		HTTP_response_string.append("\r\n");
		itr++;
	}
}

void HTTPRequestProcessor::getBodyToResponseString()
{
	HTTP_response_string.append(response.body);
}

/*This function handles TRACE method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Trace()
{
	response.headers[DATE] = getDate();
	char numStr[10];

	response.body = requestStr;
	response.headers[CONTENT_LENGTH] = _itoa(requestStr.length(), numStr, 10);
	response.headers[CONTENT_TYPE] = "message/http";

	setResponseStatusLine({ "OK", 202 });
}

/*This function handles DELETE method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Delete()
{
	response.headers[DATE] = getDate();
	char numStr[10];
	request.requestLine.uri.path = SERVER_BASE_FILE + request.requestLine.uri.path;

	ifstream fileStream(request.requestLine.uri.path);

	if (!fileStream)
	{
		setResponseStatusLine({ "Not Found", 404 });
		return;
	}
	fileStream.close();
	if (remove(request.requestLine.uri.path.data()) == 0)
	{
		setResponseStatusLine({ "No Content", 204 });
	}
	else
	{
		setResponseStatusLine({ "Accepted", 202 });
		response.body = "An attempt to delete the file was unssuccesful!";
		response.headers[CONTENT_LENGTH] = _itoa(response.body.length(), numStr, 10);
	}
}

/*This function handles GET method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Get()
{
	updatePathAccordingToQuery();
	request.requestLine.uri.path = SERVER_BASE_FILE + request.requestLine.uri.path;

	

	ifstream fileStream(request.requestLine.uri.path);

	response.headers[DATE] = getDate();

	if (fileStream.is_open()) {
		setResponseStatusLine({ "OK",200 });
		getFileToResponseBody(fileStream);
		fileStream.close();
		return;
	}

	setResponseStatusLine({ "NOT FOUND", 404 });
	response.headers[CONTENT_LENGTH] = "0";
	//createStringHttpResponse();
}

/*This function handles POST method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Post()
{
	response.headers[DATE] = getDate();
	response.headers[CONTENT_LENGTH] = "0";

	if (request.requestLine.uri.isEmpty) {
		cout << "========Post method:=========" << endl;
		cout << request.body << endl;
		cout << "=============================" << endl;
		setResponseStatusLine({ "OK",200 });
	}
	else {
		setResponseStatusLine({ "Bad Request",400 });
	}
}

/*This function handles PUT method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Put()
{
	response.headers[DATE] = getDate();
	char numStr[10];
	request.requestLine.uri.path = SERVER_BASE_FILE + request.requestLine.uri.path;
	
	int dotIndex = request.requestLine.uri.path.find_last_of('.');
	string body("");


	if (request.headers.count(CONTENT_ENCODING) != 0 || request.headers.count(CONTENT_LANGUAGE) != 0
		|| request.headers.count(CONTENT_LOCATION) != 0 || request.headers.count(CONTENT_MD5) != 0
		|| request.headers.count(CONTENT_RANGE) != 0)
	{
		setResponseStatusLine({ "Not Implemented", 501 });
		return;
	}


	string fileFormatStr("");

	ifstream file(request.requestLine.uri.path); //we check if the file already exists, by attempting to open it as input file

	if (!file) //if no such file exists - we try to create a new one:
	{
		if ((getFileFormat() == Formats::NotSupported) || (getFileFormat() == Formats::Undetected)) //if its not ".txt"/".html" - ERROR!
		{
			setResponseStatusLine({ "Unsupported Media Type", 415 });
			body = ("A \"PUT\" method was attempted, on a file that has an unsupported format(postfix!)! \n server supports only .txt/.html files!");
			response.headers[CONTENT_LENGTH] = _itoa(body.length(), numStr, 10);
			response.body = body;
			return;
		}

		ofstream newFile(request.requestLine.uri.path); //we attemp the create a new file, in the path given in the URL

		if (!newFile) // if we have faild  - send approppriate response
		{
			setResponseStatusLine({ "Internal Server Error", 500 });
			body = ("Error whilst trying to create the resource on server! please check URL for possible errors/unsupported/reserved characters");
			response.headers[CONTENT_LENGTH] = _itoa(body.length(), numStr, 10);
			response.body = body;
			//createStringHttpResponse();
			return;
		}
		//if we have successeeded, we put the request body in the file and finish
		newFile << request.body;
		setResponseStatusLine({ "Created", 201 });
		return;
	}

	else //if a file with that name DOES already exist - we attemp to create a MODIFIED version
	{
		bool modificationAvailable = false;
		file.close();
		// Inserting the string "Modified" AFTER THE DOT - (for example ("hello.txt" -->> "helloModified.txt")
		string modifiedPath = request.requestLine.uri.path.insert(dotIndex, "Modified");
		dotIndex += 8;
		char numStr[10];

		for (int i=1; i <= MAX_MODIFIED_VERSIONS; i++)
		{
			modifiedPath.insert(dotIndex, _itoa(i, numStr, 10));
			file.open(modifiedPath);
			if (!file)
			{
				modificationAvailable = true;
				break;
			}
			file.close();
			modifiedPath.erase(dotIndex, strlen(numStr));
		}

		if (!modificationAvailable)
		{
			setResponseStatusLine({ "Conflict", 409 });
			body = ("A modified version of that resource could not be formed in server, because there are already ");
			body += _itoa(MAX_MODIFIED_VERSIONS, numStr, 10);
			body += " modified versions, which is the maximum allowed";
			response.body = body;
			response.headers[CONTENT_LENGTH] = _itoa(body.length(), numStr, 10);
			createStringHttpResponse();
			return;
		}


		ofstream newFile(modifiedPath);

		if (!newFile) //if we have faild to open a new file in that path - send error in the response
		{
			setResponseStatusLine({ "Internal Server Error", 500 });
			body = ("A \"PUT\" method was attempted on an existing file - server faild while trying to create a new modified version.");
			response.body = body;
			response.headers[CONTENT_LENGTH] = _itoa(body.length(), numStr, 10);
			//createStringHttpResponse();
			return;
		}

		//if we've successeeded - we put the request's body in the file, and inform the client in out response
		newFile << request.body;
		setResponseStatusLine({ "OK", 200 });
		//createStringHttpResponse();
		return;

	}

}

/*This function handles OPTIONS method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Options()
{
	setResponseStatusLine({ "No Content",204 });
	response.headers[CONTENT_LENGTH] = "0";
	response.headers[DATE] = getDate();

	if (request.requestLine.uri.path == OPTIONS_ENTIRE_SERVER) {
		OptionsEntireServer();
	}

	else {
		OptionsSpecificURL();
	}
}

/*This function handles HEAD method according to the http 1.1 protocol*/
void HTTPRequestProcessor::Head()
{
	/*performing Get request but deleting the body*/
	Get();
	response.body.clear();
}

/*This function handles the OPTIONS method when the client used (*) in the start line */
void HTTPRequestProcessor::OptionsEntireServer()
{
	string serverOptions = GET + " " + HEAD + " " + POST + " " + OPTIONS + " " + TRACE + " " + PUT + " " + _DELETE;
	response.headers.insert({ ALLOW, serverOptions });
}

/*This function handles OPTIONS method when the client entered a specific URL in the start line*/
void HTTPRequestProcessor::OptionsSpecificURL()
{
	string serverOptions = OPTIONS + " " + TRACE + " ";
	request.requestLine.uri.path = SERVER_BASE_FILE + request.requestLine.uri.path;
	ifstream fileStream(request.requestLine.uri.path);
	Formats format = getFileFormat();

	/*POST requests can only be sent directly to server*/
	if (request.requestLine.uri.isEmpty){
		serverOptions += POST + " ";
	}

	if (format == Formats::html ||	 format == Formats::txt) {
		serverOptions += PUT + " ";
	}

	
	if (fileStream.is_open()){
		serverOptions += GET + " " + HEAD + " " + _DELETE;
		fileStream.close();
	}

	response.headers.insert({ ALLOW, serverOptions });
}

/*This function checks what is the file format in the URL, and returns an enum accordingly.*/
Formats HTTPRequestProcessor::getFileFormat()
{
	unsigned int dotPos = request.requestLine.uri.path.find_last_of('.');

	if (dotPos == string::npos|| request.requestLine.uri.path.find('.') == string::npos)
		return Formats::Undetected;

	if(request.requestLine.uri.path.length() - 4 < dotPos)
		return Formats::NotSupported;

	if (request.requestLine.uri.path.substr(dotPos, 5).compare(".html") == 0)
		return Formats::html;
		
	if (request.requestLine.uri.path.substr(dotPos, 4).compare(".txt") == 0)
		return Formats::txt;

	return Formats::NotSupported;
}