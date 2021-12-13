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
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <sys/poll.h>
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
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#define log  std::cout <<
#define line << std::endl

void outputLogs(std::string logs);

#endif  // !WEBSERV_HPP
