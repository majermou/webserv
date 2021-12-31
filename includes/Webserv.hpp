#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#ifndef DEBUG
#	define DEBUG 0
#endif /* ifndef DEBUG */

#ifndef LOGS_FILE
#	define LOGS_FILE "logs.webserv"
#endif /* ifndef LOGS_FILE */

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <ctime>

#include "Location.hpp"
#include "ServerData.hpp"
#define log  std::cout <<
#define line << std::endl

#define SP               	" "
#define CRLF             	"\r\n"
#define CRLFCRLF         	"\r\n\r\n"
#define HeaderPairsDelim 	": "
#define HTTPv1				"HTTP/1.1"
#define HTTPv2				"HTTP/2"
#define Mbytes				1000000

struct Ret
{
	std::string response;
	bool connection;
	bool safi;
};

struct Request
{
	std::string request_line;
	std::multimap<std::string, std::string> request_headers;
	std::string body;
};

struct Response
{
	std::string status_line;
	std::map<std::string, std::string> response_headers;
	std::string body;
};

struct CGIparam {
	std::string	method;
	std::string	path;
	std::string query;
	std::string	content_type;
	std::string	content_length;
	std::string body;
	std::string fastcgipass;
	std::string cookie;
	int			execvRet;
};

struct RqLineData {
	std::string path;
	std::string query;
	int server_num;
	int location_num;
	std::vector<Location> locations;
};

struct filenames {
	std::string	data;
	std::string	path;
};

int	examineLocations(std::vector<Location> locations, std::string path);
int	examineServers(Request &req, std::vector<ServerData> &data);
void checkvalid(std::string &path);
std::string getToken(std::string &str, std::string delimiter);
struct Ret generateResponse(struct Response resp);
std::string	contentType(std::string path);
std::string NumberToString(int number);
std::vector<filenames>	parsePost(std::string body, std::string boundary);
struct Ret handleRequest(std::string buff, std::vector<ServerData> &data, bool done);
void outputLogs(std::string logs);
std::string runCgi(CGIparam p);

#endif  // !WEBSERV_HPP
