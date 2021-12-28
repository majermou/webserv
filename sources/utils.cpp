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
#define HTTPv1				"HTTP/1.1"
#define HTTPv2				"HTTP/2"

struct filenames {
	std::string	data;
	std::string	path;
};

std::string getToken(std::string &str, std::string delimiter)
{
	size_t pos        = str.find(delimiter);
	std::string token = str.substr(0, pos);
	str.erase(0, pos + delimiter.length());
	return token;
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

std::string	contentType(std::string path) {
	std::string extention;

	getToken(path, ".");
	extention = getToken(path, ".");
	if (extention == "html" || extention == "htm")
		return "text/html";
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
	else if (extention == "mp4")
		return "video/mp4";
	return "text/plain";
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
	ret.response += CRLF;
	if (resp.response_headers.count("Cookie") == 1) {
		std::string	cookie = resp.response_headers.find("Cookie")->second;
		std::string	str;
		while ((str = getToken(cookie, ";")) != "") {
			ret.response = "Set-Cookie: ";
			ret.response += str;
			ret.response += CRLF;
		}
	}
	ret.response += CRLF;
	ret.response += resp.body;
	ret.response += CRLFCRLF;
	if (resp.response_headers.find("Connection")->second == "keep-alive")
		ret.connection = true;
	else
		ret.connection = false;
	ret.safi = true;
	return ret;
}

std::vector<filenames>	parsePost(std::string body, std::string boundary)
{
	std::string					two_dashes = "--";
	std::string					str0("\r\n");
	std::string					last_boundary = str0 + two_dashes + boundary + two_dashes;
	std::string					in_boundary = str0 + two_dashes + boundary;
	filenames					filename;
	std::vector<filenames>		vect;
	std::string					str;

	body = getToken(body, last_boundary);
	getToken(body, two_dashes + boundary);
	while (body.find(in_boundary) != std::string::npos) {
		str = body.substr(0, body.find(in_boundary));
		body.erase(0, body.find(in_boundary));
		body.erase(0, in_boundary.length());
		filename.path = getToken(str, "filename=\"");
		filename.path = getToken(str, "\"");
		filename.data = getToken(str, CRLFCRLF);
		filename.data = str;
		vect.push_back(filename);
	}
	filename.path = getToken(body, "filename=\"");
	filename.path = getToken(body, "\"");
	filename.data = getToken(body, CRLFCRLF);
	filename.data = body;
	vect.push_back(filename);
	return vect;
}
