#include "../includes/Webserv.hpp"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <map>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include "../includes/Webserv.hpp"

#define SP               	" "
#define CRLF             	"\r\n"
#define CRLFCRLF         	"\r\n\r\n"
#define HeaderPairsDelim 	": "
#define HTTP_VERSION     	"HTTP/1.1"

#define HTTP2 "HTTP/2"

std::string getToken(std::string &str, std::string delimiter);
struct Ret generateResponse(struct Response resp);
struct Ret HandleErrors(std::string errorType);
void checkvalid(std::string &path);
int	examineLocations(std::vector<Location> locations, std::string path);
int	examineServers(Request &req, std::vector<ServerData> &data);


struct Ret handle_DELETE_Request(Request &request, std::vector<ServerData> &data) {
    std::string path = getToken(request.request_line, SP);
    std::string http_version = request.request_line;
    int serverNum = 0;
    int locationNum = 0;
    std::vector<Location> locations;
    struct stat buffer;
    Response response;

    if (request.request_headers.count("Host") != 1) {
        return HandleErrors("400 Bad Request");
    }
    serverNum = examineServers(request, data);
    locations = data[serverNum].getLocations();
    locationNum = examineLocations(locations, path);
    if (locations[locationNum].isRedirection() == true
        || locations[locationNum].getAllowedMethods().find("DELETE")->second == false) {
        return HandleErrors("405 Not Allowed");
    }
    if (locations[locationNum].getRootDir().empty()) {
		path = data[serverNum].getRootDir() + path;
	} else {
		path = locations[locationNum].getRootDir() + path;
	}
    checkvalid(path);
    if (stat(path.c_str(), &buffer) == 0) {
        if (buffer.st_mode & S_IFDIR)
            return HandleErrors("405 Not Allowed");
        if (remove(path.c_str()) != 0) {
            std::cout << "deleted";
            return HandleErrors("403 Forbidden");
        }
    } else {
        return HandleErrors("404 Not Found");
    }
    if (http_version != HTTP_VERSION) {
        if (http_version == HTTP2)
            return HandleErrors("505 HTTP Version Not Supported");
        return HandleErrors("400 Bad Request");
    }
    response.body += "<html>";
    response.body += CRLF;
    response.body += "<body>";
    response.body += CRLF;
    response.body += "<h1>";
    response.body += path;
    response.body += " was deleted.</h1>";
    response.body += CRLF;
    response.body += "</body>";
    response.body += CRLF;
    response.body += "</html>";
    response.status_line = http_version + " 200 KO";
    response.response_headers["Content-Type"] = "text/html; charset=UTF-8";
    response.response_headers["Content-Length"] = std::to_string(response.body.length()); ///// c++11
    if (request.request_headers.count("Connection") == 0 ||
		request.request_headers.find("Connection")->second == "keep-alive") {
		response.response_headers["Connection"] = "keep-alive";
	} else 
	    response.response_headers["Connection"] = "closed";
    return generateResponse(response);
}
