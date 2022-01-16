#pragma once
#include <String>
#include <cstdio>
using namespace std;
#include <map>
#define LANG "lang"
#define HTTPDOT0 "HTTP/1.0"
#define HTTPDOT1 "HTTP/1.1"
#define OPTIONS_ENTIRE_SERVER "/*"
#define NO_PATH "/"
#define CR static_cast<char>(13)
#define LF static_cast<char>(10)
#define MAX_MODIFIED_VERSIONS 10
#define SERVER_BASE_FILE "C:/temp"

static map<int, string> Months = { {1,"Jan"},{ 2,"Feb" },{ 3,"Mar" },{ 4,"Apr" },{ 5,"May" },{ 6,"Jun" },{ 7,"Jul" },{ 8,"Aug" },{ 9,"Sep" },{ 10,"Oct" },{ 11,"Nov" },{ 12,"Dec" }};
static map<int, string> WeekDays{ { 1,"Sun" },{ 2,"Mon" },{ 3,"Tue" },{ 4,"Wed" },{ 5,"Thu" },{ 6,"Fri" },{ 7,"Sat" }};

enum QueryParam { lang = 1 };

typedef struct query
{
	QueryParam param;
	string val;
}Query;

typedef struct uri
{
	bool isEmpty;
	string path;
	Query query;
	bool isQuery;
}URI;

enum Version { httpDot0, httpDot1 };

typedef struct status_code
{
	struct status_code(const char* name, int number) : name(name), code(number) {}
	struct status_code(){}

	string name;
	int code;
}Status;

enum serviceMethod { Options, Get, Head, Post, Put, Delete, Trace };

typedef struct request_line
{
	serviceMethod method;
	URI uri;
	Version version = Version::httpDot1;
}RequestLine;


typedef struct request
{
	RequestLine requestLine;
	map<string, string> headers;
	string body;
}Request;

typedef struct status_line 
{
	Version version;
	struct status_code status;
}StatusLine;


typedef struct response{
	
	StatusLine statusLine;
	map<string, string> headers;
	string body;
}Response;


enum Formats { txt, html, NotSupported, Undetected};

const string OPTIONS("OPTIONS");
const string GET("GET");
const string HEAD("HEAD");
const string POST("POST");
const string PUT("PUT");
const string _DELETE("DELETE");
const string TRACE("TRACE");


//general headers:
#define CASH_CONTROL "Cash-Control"
#define CONNECTION "Connection"
#define DATE "Date"
#define PRAGMA "Pragma"
#define TRAILER "Trailer"
#define TRANSFER_ENCODING "Transfer-Encoding"
#define UPGRADE "Upgrade"
#define VIA "Via"
#define WARNING "Warning"


//request headers:
#define ACCEPT "Accept"
#define ACCEPT_CHARSET "Accept_Charset"
#define ACCEPT_ENCODING "Accept-Encoding"
#define ACCEPT_LANGUAGE "Accept-Language"
#define AUTHORIZATION "Authorization"
#define EXPECT "Expect"
#define FROM "From"
#define HOST "Host"
#define IF_MATCH "If-Match"

#define IF_MODIFIED_SINCE "If-Modified-Since"
#define IF_NONE_MATCH "If-None-Match"
#define IF_RANGE "If-Range"
#define IF_UNMODIFIED_SINCE "If-Unmodified-Since"
#define MAX_FORWARDS "Max-Forwards"
#define PROXY_AUTHRIZATION "Proxy-Authorization"
#define RANGE "Range"
#define REFERER "Referer"
#define TE "TE"
#define USER_AGENT "User-Agent"

//entity headers:
#define ALLOW "Allow"
#define CONTENT_ENCODING "Content-Encoding"
#define CONTENT_LANGUAGE "Content-Language"
#define CONTENT_LENGTH "Content-Length"
#define CONTENT_LOCATION "Content-Location"
#define CONTENT_MD5 "Content-MD5"
#define CONTENT_RANGE "Content-Range"
#define CONTENT_TYPE "Content-Type"
#define EXPIRES "Expires"
#define LAST_MODIFIED "Last-Modified"
#define EXTENSION_HEADER "extension-header"


//response headers:
#define DATE "Date"
#define CONTENT_LENGTH "Content-Length"
#define CONTENT_TYPE "Content-Type"
#define ACCEPT_RANGES "Accept-Ranges"
#define AGE "Age"
#define ETAG "ETag"
#define LOCATION "Location"
#define PROXY_AUTHENTICATE "Proxy-Authenticate"
#define RETRY_AFTER "Retry-After"
#define SERVER "Server"
#define VARY "Vary"
#define WWW_AUTHENTICATE "WWW-Authenticate"














