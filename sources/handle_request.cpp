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

struct Ret handle_DELETE_Request(Request &request, std::vector<ServerData> &data);


std::string getToken(std::string &str, std::string delimiter)
{
	size_t pos        = str.find(delimiter);
	std::string token = str.substr(0, pos);
	str.erase(0, pos + delimiter.length());
	return token;
}

struct Ret generateResponse(struct Response resp)
{
	Ret ret;
	char buff[1000];

	ret.response += resp.status_line;
	ret.response += CRLF;
	for (std::map<std::string, std::string>::iterator it =
	         resp.response_headers.begin();
	     it != resp.response_headers.end(); it++)
	{
		ret.response += it->first;
		ret.response += ": ";
		ret.response += it->second;
		ret.response += CRLF;
	}
	ret.response += "Server: WebServ/v0.1";
	ret.response += CRLF;
	time_t now   = time(0);
	struct tm tm = *gmtime(&now);
	strftime(buff, sizeof buff, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	ret.response += "Date: " + std::string(buff);
	ret.response += CRLFCRLF;
	ret.response += resp.body;
	ret.response += CRLFCRLF;
	if (resp.response_headers.find("Connection")->second == "keep-alive")
		ret.connection = true;
	else
		ret.connection = false;
	return ret;
}

struct Ret HandleErrors(std::string errorType)
{
	Response	resp;
	std::string ln;

	resp.status_line += HTTP_VERSION;
	resp.status_line += SP;
	resp.status_line += errorType;
	resp.response_headers["Connection"] = "close";
	resp.response_headers["Content-type"] = "text/html; charset=UTF-8";
	int contentLenght = 0;
	std::ifstream ifs("/Users/majermou/webserv/404.html");
	while (ifs) {
		getline(ifs, ln);
		resp.body += ln;
		resp.body += CRLF;
		contentLenght += ln.length();
	}
	resp.response_headers.insert(
	    std::make_pair("Content-Length", std::to_string(contentLenght))); // c++98
	return generateResponse(resp);
}

void checkvalid(std::string &path)
{
	for (size_t i = 0; i < path.length() - 1; i++)
	{
		if (path[i] == path[i + 1] && path[i] == '/')
		{
			path.erase(i, 1);
		}
	}
}

int	examineLocations(std::vector<Location> locations, std::string path)
{
	int		LocationNum  = 0;
	size_t	priority = 0;

	for (size_t i = 0; i < locations.size(); i++)
	{
		if (path.find(locations[i].getPath()) != std::string::npos &&
		    locations[i].getPath().length() > priority)
			LocationNum = i;
	}
	return LocationNum;
}

int	examineServers(Request &req, std::vector<ServerData> &data) {
	for (int i = data.size() - 1; i > 0; i--) {
		if (std::find(data[i].getNames().begin(), data[i].getNames().end(),
			req.request_headers.find("Host")->second) != data[i].getNames().end())
			return i;
	}
	return data.size() - 1;
}

std::string	contentType(std::string path) {
	std::string extention;

	getToken(path, ".");
	extention = getToken(path, ".");
	if (extention == "html")
		return "text/html; charset=UTF-8";
	else if (extention == "css")
		return "text/css";
	else if (extention == "js")
		return "text/javascript";
	else if (extention == "gif")
		return "image/gif";
	else if (extention == "webp")
		return "image/webp";
	else if (extention == "jpeg" || extention == "jpg")
		return "image/jpeg";
	else if (extention == "png")
		return "image/png";
	return "text/plain";
}

std::string	HandleAutoIndexing(std::string path, std::string uri) {
	struct dirent	*entry;
	std::string		link;
	std::string ret = "<html><head><title>Indexing</title></head><body>\r\n";
	ret += "<h1>Index of ";
	ret += path;
	ret += "</h1><hr>\r\n";
	DIR *dir = opendir(path.c_str());

	if (dir == NULL) {
    	return std::string();
	}
	while ((entry = readdir(dir)) != NULL) {
		link = uri + "/" + entry->d_name;
		checkvalid(link);
		ret += "<a href=\"" + link + "\">" + entry->d_name + "</a>\r\n";
		ret += "<br>";
	}
	closedir(dir);
	ret += "<hr></body></html>\r\n\r\n";
	return ret;
}

struct Ret handle_GET_Request(Request &req, std::vector<ServerData> &data)
{
	std::string return_code = "200 OK";
	std::string http_version;
	std::string path;
	std::string uri;
	int i, j;
	struct stat buffer;
	std::ostringstream stream;
	Response resp;
	std::ifstream file;
	std::string ln;

	if (req.request_headers.count("Host") != 1) {
		return HandleErrors("400 Bad Request");
	} else {
		i = examineServers(req, data);
		std::vector<Location> locations = data[i].getLocations();
		uri = getToken(req.request_line, SP);
		path = uri;
		j = examineLocations(locations, path);
		if (locations[j].isRedirection() == true) {
			stream << locations[j].getReturnCode();
			return_code = stream.str() + " Moved Permanently";
			path = locations[j].getReturnUrl();
			j = examineLocations(locations, path);
			resp.response_headers["Location"] = path;
		}

		// CGI //

		if (locations[j].getAllowedMethods().find("GET")->second == false)
			return HandleErrors("405Not Allowed");
		if (locations[j].getRootDir().empty()) {
			path = data[i].getRootDir() + path;
		} else {
			path = locations[j].getRootDir() + path;
		}
		checkvalid(path);
		if (stat(path.c_str(), &buffer) == 0) {
			if (access(path.c_str(), F_OK) != 0)
				return HandleErrors("403 Forbidden");
			if (buffer.st_mode & S_IFDIR) {
				if (locations[j].getAutoIndex() == true) {
					resp.body = HandleAutoIndexing(path, uri);
					if (resp.body.empty() == true)
						return HandleErrors("500 Internal Server Error");
					resp.response_headers["Content-Type"] = "text/html; charset=UTF-8";
				} else {
					if (locations[j].getDefaultFile().empty() == true)
						return HandleErrors("403 Forbidden");
					path += locations[j].getDefaultFile();
				}
			}
		}
		else
			return HandleErrors("404 Not Found");
		http_version = req.request_line;
		if (http_version != HTTP_VERSION) {
			if (http_version == "HTTP/2")
				return HandleErrors("505 HTTP Version Not Supported");
			return HandleErrors("400 Bad Request");
		}
		if (resp.body.empty() == true) {
			if (contentType(path) == "image/jpeg") {
				file.open(path, std::ios::binary);
				char buff;
				if (file.is_open()) {
					while (!file.eof()) {
						file >> std::noskipws >> buff;
						resp.body += buff;
					}
					file.close();
				}
			} else {
				file.open(path);
				for (std::string str; getline(file, str); ) {
					resp.body += str;
					resp.body += CRLF;
				}
				file.close();
			}
		}
		resp.status_line = http_version + " " + return_code;
		if (resp.response_headers.count("Content-Type") != 1)
			resp.response_headers["Content-Type"] = contentType(path);
		resp.response_headers["Content-Length"] = std::to_string(resp.body.length());
		if (req.request_headers.count("Connection") == 0 ||
		    req.request_headers.find("Connection")->second == "keep-alive") {
			resp.response_headers["Connection"] = "keep-alive";
		} else 
			resp.response_headers["Connection"] = "closed";
	}
	return generateResponse(resp);
}

struct Ret handleRequest(std::string buff, std::vector<ServerData> &data)
{
	Request request;
	std::string token;

	request.request_line = getToken(buff, CRLF);
	while ((token = getToken(buff, CRLF)) != "") {
		request.request_headers.insert(std::make_pair(getToken(token, HeaderPairsDelim),
		                			   getToken(token, HeaderPairsDelim)));
	}
	request.body = getToken(buff, CRLFCRLF);
	std::string method = getToken(request.request_line, SP);
	if (method == "GET")
		return handle_GET_Request(request, data);
	else if (method == "DELETE")
	    return handle_DELETE_Request(request, data);
	// else if (method == "POST")
	//     return handle_POST_Request(rq);
	else if (method == "OPTIONS" || method == "HEAD" || method == "PUT" || method == "PATCH")
		return HandleErrors("501 Not Implemented");
	return HandleErrors("400 Bad Request");
}
