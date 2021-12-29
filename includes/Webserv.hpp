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

#include "Location.hpp"
#include "ServerData.hpp"
#define log  std::cout <<
#define line << std::endl

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
};

struct Ret handleRequest(std::string buff, std::vector<ServerData> &data, bool done);

void outputLogs(std::string logs);
std::string runCgi(CGIparam p);

#endif  // !WEBSERV_HPP
