#include <map>
#include <ctime>
#include <fstream>
#include <string>
#include <iostream>
#include <sys/stat.h>

#include "includes/Webserv.hpp"
#include "includes/ServerData.hpp"


#define SP " "
#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define HeaderPairsDelim ": "
#define HTTP_VERSION "HTTP/1.1"

std::string getToken(std::string &str, std::string delimiter) {
    size_t pos = str.find(delimiter);
    std::string token = str.substr(0, pos);
    str.erase(0 , pos + delimiter.length());
    return token;
}

// struct Ret {
//     std::string response;
//     bool        connection;
// };

struct Request {
    std::string                                 request_line;
    std::multimap<std::string, std::string>     request_headers;
    std::string                                 body;
};

struct Response {
    std::string                         status_line;
    std::map<std::string, std::string>  response_headers;
    std::string                         body;
};

struct Ret  generateResponse(struct Response resp) {
    Ret     ret;

    ret.response += resp.status_line;
    ret.response += CRLF;
    for (std::map<std::string, std::string>::iterator it = resp.response_headers.begin(); it != resp.response_headers.end(); it++) {
        ret.response += it->first;
        ret.response += ": ";
        ret.response += it->second;
        ret.response += CRLF;
    }
    ret.response += CRLF;
    ret.response += resp.body;
    ret.response += CRLFCRLF;
    if (resp.response_headers.find("Connection")->second == "keep-alive")
        ret.connection = true;
    else
        ret.connection = false;
    return ret;
}

struct Ret  HandleErrors(std::string errorType) {
    Response    resp;
    char        buff[1000];

    resp.status_line += HTTP_VERSION;
    resp.status_line += SP;
    resp.status_line += errorType;
    resp.response_headers["Connection"]= "close";
    resp.response_headers.insert(std::make_pair("Server", "Webserv"));
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buff, sizeof buff, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    resp.response_headers.insert(std::make_pair("Date", std::string(buff)));
    resp.response_headers.insert(std::make_pair("Content-type", "text/html; charset=UTF-8"));
    int contentLenght = 0;
    std::string ln;
    std::ifstream ifs("/Users/majermou/webserv/404.html");
    while (ifs) {
        getline(ifs, ln);
        resp.body += ln;
        resp.body += CRLF;
        contentLenght += ln.length();
    }
    resp.response_headers.insert(std::make_pair("Content-Length", std::to_string(contentLenght)));
    return generateResponse(resp);
}


/// Handle CGI  ///


size_t examineRoutes(std::vector<Location> locations, std::string path) {
    size_t pathNum = 0;
    size_t priority = 0;

    for (int j = 0; j < locations.size(); j++) {
        if (path.find(locations[j].getPath()) != std::string::npos && locations[j].getPath().length() > priority)
            pathNum = j;
    }
    return pathNum;
}


void checkvalid(std::string &path) {
    for (int i = 0; i < path.length() - 1; i++) {
        if (path[i] == path[i+1] && path[i] == '/') {
            path.erase(i, 1);
        }
    }
}


struct Ret  handle_GET_Request(Request request, std::vector<ServerData> &data) {
    std::string     path;
    int             i, j;
    struct stat     buffer;
    std::ifstream   ifs;
    Response        response;
    size_t          contentLenght = 0;
    std::string     http_version;
    std::string   ln;

    if (request.request_headers.count("Host") == 0) {
        return HandleErrors("400 Bad Request");
    } else {
        for (i = data.size() - 1; i > 0; i--) {
            if (std::find(data[i].getNames().begin(), data[i].getNames().end(), request.request_headers.find("Host")->second) != data[i].getNames().end()) {
                break;
            }
        }
        path = data[i].getRootDir();
        path += getToken(request.request_line, SP);
        checkvalid(path);
        std::vector<Location> locations = data[i].getLocations();
        j = examineRoutes(locations, path);
        if (locations[j].getAllowedMethods().find("GET")->second == false)
            return HandleErrors("405 Method Not Allowed");

        // Check redirection & CGI
        std::cout <<  "{{{" <<  path << '\n';
        if (stat(path.c_str(), &buffer) == 0) {
            if (buffer.st_mode & S_IFDIR)
                path += locations[j].getDefaultFile();
        } else {
            return HandleErrors("404 Not Found");
        }
        http_version = request.request_line;
        if (http_version != HTTP_VERSION) {
            if (http_version == "HTTP/2")
                return HandleErrors("505 HTTP Version Not Supported");
            return HandleErrors("400 Bad Request");
        }
        ifs.open(path);
        while (ifs) {
            getline(ifs, ln);
            response.body += ln;
            response.body += CRLF;
            contentLenght += ln.length();
        }
        response.status_line = "HTTP/1.1 200 OK";
        response.response_headers["Content-Type"] = "text/html; charset=UTF-8";
        response.response_headers["Content-Length"] = std::to_string(contentLenght);
        if (request.request_headers.count("Connection") == 0 || response.response_headers["Content-Type"] == "Keep-alive")
            response.response_headers["Connection"] = "keep-alive";
        response.response_headers["Connection"] = "closed";
    }
    return generateResponse(response);
}


struct Ret  handleRequest(std::string buff, std::vector<ServerData> &data) {
    Request         request;
    std::string     token;


    request.request_line = getToken(buff, CRLF);
    while ((token = getToken(buff, CRLF)) != "") {
        request.request_headers.insert(std::make_pair(getToken(token, HeaderPairsDelim), getToken(token, HeaderPairsDelim)));
    }
    request.body = getToken(buff, CRLFCRLF);
    std::string method = getToken(request.request_line, SP);
    if (method == "GET")
        return handle_GET_Request(request, data);
    // else if (method == "DELETE")
    //     return handle_DELETE_Request(request);
    // else if (method == "POST")
    //     return handle_POST_Request(rq);
    return HandleErrors("400 Bad Request");
}
