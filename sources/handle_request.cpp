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
#define Mbytes				1000000



int	examineLocations(std::vector<Location> locations, std::string path);
int	examineServers(Request &req, std::vector<ServerData> &data);
void checkvalid(std::string &path);
std::string getToken(std::string &str, std::string delimiter);
struct Ret generateResponse(struct Response resp);
std::string	contentType(std::string path);

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

std::vector<filenames>	parsePost(std::string body, std::string boundary);






struct Ret HandleErrors(std::string errorType, std::vector<ServerData> &server_data, int server_num)
{
	Response					response;
	std::string					ln;
	std::map<int, std::string>	error_pages;
	int							contentLenght = 0;
	std::string					error_page_path;
	std::ifstream				ifs;
	std::string					str;

	response.status_line += HTTPv1;
	response.status_line += SP;
	response.status_line += errorType;
	response.response_headers["Connection"] = "close";
	response.response_headers["Content-type"] = "text/html; charset=UTF-8";
	if (server_num >= 0 && server_data[server_num].getErrorPageMap().count(atoi(errorType.c_str())) == 1) {
		error_page_path = server_data[server_num].getErrorPageMap().find(atoi(errorType.c_str()))->second;
		error_page_path = server_data[server_num].getRootDir() + error_page_path;
		checkvalid(error_page_path);
		ifs.open(error_page_path);
		if (ifs.is_open()) {
			while (getline(ifs, str)) {
				response.body += str;
				response.body += CRLF;
			}
		} else {
			errorType = "500 Internal Server Error";
			goto label;
		}
	} else {
		label:
		response.body += "<html>";
		response.body += CRLF;
		response.body += "<head>";
		response.body += CRLF;
		response.body += "<title>";
		response.body += errorType;
		response.body += "</title>";
		response.body += CRLF;
		response.body += "</head>";
		response.body += CRLF;
		response.body += "<body>";
		response.body += CRLF;
		response.body += "<h1>";
		response.body += errorType;
		response.body += "</h1>";
		response.body += CRLF;
		response.body += "<h4>";
		response.body += "WebServ/v0.1";
		response.body += "</h4>";
		response.body += CRLF;
		response.body += "</body>";
		response.body += CRLF;
		response.body += "</html>";
		response.body += CRLFCRLF;
	}
	response.response_headers["Content-Length"] = std::to_string(response.body.length()); // c++11
	return generateResponse(response);
}

struct Ret  handle_POST_Request(Request &request, std::vector<ServerData> &server_data, RqLineData &data)
{
    std::string				return_code = "200 OK";
    Response				response;
    std::string				uploadLocation;
    std::string				boundary;
	std::string				content_type;
	std::vector<filenames>	files;
	std::ofstream			ofs;
	std::string				payload;

    if (data.locations[data.location_num].getAllowedMethods().find("POST")->second == false ||
		data.locations[data.location_num].getUploadEnabled() == false)
    	return HandleErrors("405 Not Allowed", server_data, data.server_num);
	uploadLocation = data.locations[data.location_num].getUploadLocation();
	uploadLocation = server_data[data.server_num].getRootDir() + uploadLocation;
	uploadLocation += "/";
	content_type = getToken(request.request_headers.find("Content-Type")->second, ";");
    if (content_type ==  "multipart/form-data") {
		boundary = getToken(request.request_headers.find("Content-Type")->second, "boundary=");
		boundary = getToken(request.request_headers.find("Content-Type")->second, "boundary=");
		files = parsePost(request.body, boundary);
		for (int i = 0; i < files.size(); i++) {
			files[i].path = uploadLocation + files[i].path;
			checkvalid(files[i].path);
			ofs.open(files[i].path);
			if (ofs.is_open()) {
				ofs << files[i].data;
				ofs.close();
			} else
				return HandleErrors("500 Internal Server Error", server_data, data.server_num);
		}
	} else if (content_type ==  "application/x-www-form-urlencoded" || content_type == "text/plain")
		payload = request.body;
	else
		return HandleErrors("415 Unsupported Media Type", server_data, data.server_num);
	response.body += "<html>";
    response.body += CRLF;
    response.body += "	<body>";
    response.body += CRLF;
    response.body += "		<h1>";
    response.body += " Successfull Upload</h1>";
    response.body += CRLF;
    response.body += "	</body>";
    response.body += CRLF;
    response.body += "</html>";
    response.status_line = HTTPv1;
	response.status_line += " 200 KO";
	if (request.request_headers.count("Cookie") == 1) {
		response.response_headers["Cookie"] = request.request_headers.find("Cookie")->second;
	}
    response.response_headers["Content-Type"] = "text/html; charset=UTF-8";
    response.response_headers["Content-Length"] = std::to_string(response.body.length()); ///// c++11
    if (request.request_headers.count("Connection") == 0 ||
		request.request_headers.find("Connection")->second == "keep-alive") {
		response.response_headers["Connection"] = "keep-alive";
	} else 
	    response.response_headers["Connection"] = "closed";
	return generateResponse(response);
}

struct Ret	handle_DELETE_Request(Request &request, std::vector<ServerData> &server_data, RqLineData &data)
{
    Response		response;
    struct stat		buffer;

    if (data.locations[data.location_num].isRedirection() == true
        || data.locations[data.location_num].getAllowedMethods().find("DELETE")->second == false) {
    	return HandleErrors("405 Not Allowed", server_data, data.server_num);
    }
    if (data.locations[data.location_num].getRootDir().empty()) {
		data.path = server_data[data.server_num].getRootDir() + data.path;
	} else {
		data.path = data.locations[data.location_num].getRootDir() + data.path;
	}
    checkvalid(data.path);
    if (stat(data.path.c_str(), &buffer) == 0) {
        if (buffer.st_mode & S_IFDIR)
            return HandleErrors("405 Not Allowed", server_data, data.server_num);
        if (remove(data.path.c_str()) != 0)
            return HandleErrors("403 Forbidden", server_data, data.server_num);
    } else {
        return HandleErrors("404 Not Found", server_data, data.server_num);
    }
    response.body += "<html>";
    response.body += CRLF;
    response.body += "	<body>";
    response.body += CRLF;
    response.body += "		<h1>";
    response.body += data.path;
    response.body += " was deleted.</h1>";
    response.body += CRLF;
    response.body += "	</body>";
    response.body += CRLF;
    response.body += "</html>";
    response.status_line = HTTPv1;
	response.status_line += " 200 KO";
    response.response_headers["Content-Type"] = "text/html; charset=UTF-8";
    response.response_headers["Content-Length"] = std::to_string(response.body.length()); ///// c++11
    if (request.request_headers.count("Connection") == 0 ||
		request.request_headers.find("Connection")->second == "keep-alive") {
		response.response_headers["Connection"] = "keep-alive";
	} else 
	    response.response_headers["Connection"] = "closed";
    return generateResponse(response);
}

std::string	HandleAutoIndexing(std::string path, std::string uri)
{
	struct dirent	*entry;
	std::string		link;
	std::string		ret = "<html><head><title>Indexing</title></head><body>\r\n";
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

struct Ret handle_GET_Request(Request &request, std::vector<ServerData> &server_data, RqLineData &data)
{
	Response			response;
	std::string			uri = data.path;
	std::string			return_code = " 200 OK";
	struct stat			buffer;
	std::ostringstream	stream;
	std::ifstream		file;
	std::string			ln;
	char				buff;

	if (data.locations[data.location_num].isRedirection() == true) {
		return_code += " ";
		return_code += std::to_string(data.locations[data.location_num].getReturnCode()); // c++11
		return_code += " Moved Permanently";
		data.path = data.locations[data.location_num].getReturnUrl();
		data.location_num = examineLocations(data.locations, data.path);
		response.response_headers["Location"] = data.path;
	}
	if (data.locations[data.location_num].getAllowedMethods().find("GET")->second == false)
		return HandleErrors("405 Not Allowed", server_data, data.server_num);
	if (data.locations[data.location_num].getRootDir().empty()) {
		data.path = server_data[data.server_num].getRootDir() + data.path;
	} else {
		data.path = data.locations[data.location_num].getRootDir() + data.path;
	}
	checkvalid(data.path);
	if (stat(data.path.c_str(), &buffer) == 0) {
		if (access(data.path.c_str(), F_OK) != 0)
			return HandleErrors("403 Forbidden", server_data, data.server_num);
		if (buffer.st_mode & S_IFDIR) {
			if (data.locations[data.location_num].getAutoIndex() == true) {
				response.body = HandleAutoIndexing(data.path, uri);
				if (response.body.empty() == true)
					return HandleErrors("500 Internal Server Error", server_data, data.server_num);
				response.response_headers["Content-Type"] = "text/html; charset=UTF-8";
			} else {
				if (data.locations[data.location_num].getDefaultFile().empty() == true)
					return HandleErrors("403 Forbidden", server_data, data.server_num);
				data.path += data.locations[data.location_num].getDefaultFile();
			}
		}
	}
	else
		return HandleErrors("404 Not Found", server_data, data.server_num);
	if (response.body.empty() == true) {
		if (contentType(data.path) == "image/jpeg" || contentType(data.path) == "image/gif" || 
			contentType(data.path) == "image/png" ) {
			file.open(data.path, std::ios::binary);
			if (file.is_open()) {
				while (!file.eof()) {
					file >> std::noskipws >> buff;
					response.body += buff;
				}
				file.close();
			} else
				return HandleErrors("500 Internal Server Error", server_data, data.server_num);
		} else {
			file.open(data.path);
			for (std::string str; getline(file, str); ) {
				response.body += str;
				response.body += CRLF;
			}
			file.close();
		}
	}
	response.status_line += HTTPv1;
	response.status_line += return_code;
	if (request.request_headers.count("Cookie") == 1) {
		response.response_headers["Cookie"] = request.request_headers.find("Cookie")->second;
		
	}
	if (response.response_headers.count("Content-Type") != 1)
		response.response_headers["Content-Type"] = contentType(data.path);
	response.response_headers["Content-Length"] = std::to_string(response.body.length()); // c++11
	if (request.request_headers.count("Connection") == 0 ||
		request.request_headers.find("Connection")->second == "keep-alive") {
		response.response_headers["Connection"] = "keep-alive";
	} else 
		response.response_headers["Connection"] = "closed";
	return generateResponse(response);
}

struct Ret	HandleCGI(Request &request, std::vector<ServerData> &server_data, RqLineData &data, std::string method) {
	CGIparam		param;
	std::string		CGI_resp;
	Response		response;

	param.method = method;
	param.path = data.path;
	param.path = server_data[data.server_num].getRootDir() + data.path;
	checkvalid(param.path);
	param.query = data.query;
	if (request.request_headers.count("Content-Type") == 1)
		param.content_type = request.request_headers.find("Content-Type")->second;
	if (request.request_headers.count("Content-Length") == 1)
		param.content_length = request.request_headers.find("Content-Length")->second;
	param.body = request.body;
	param.fastcgipass = data.locations[data.location_num].getFastCgiPass();
	std::cout << "----{{{{{{{{";
	CGI_resp = runCgi(param);

	response.status_line = HTTPv1;
	response.status_line += " ";
	if (CGI_resp.find("Status") != std::string::npos) {
		getToken(CGI_resp, "Status: ");
		response.status_line += getToken(CGI_resp, CRLF);
	} else
		response.status_line += "200 OK";
	getToken(CGI_resp, "Content-type: ");
	response.response_headers["Content-Type"] = getToken(CGI_resp, CRLF);
	getToken(CGI_resp, CRLF);
	response.body = CGI_resp;
	response.response_headers["Content-Length"] = std::to_string(response.body.length()); // c++11
	return generateResponse(response);
}



struct Ret handleRequest(std::string buff, std::vector<ServerData> &server_data, bool done)
{
	Request					request;
	std::string				HeaderToken;
	std::string 			method;
	std::string				http_version;
	RqLineData				req_line_data;
	Ret						ret;

	request.request_line = getToken(buff, CRLF);
	method = getToken(request.request_line, SP);
	ret.safi = false;
	if (!done && method == "POST") {
		size_t pos;
		int pos0 = buff.find("Content-Length:");
		if (pos0 != std::string::npos) {
			int n = atoi(std::string(buff, pos0+ 15).c_str());
			if (n < 0)
				return ret;
			size_t pos1 = buff.find(CRLFCRLF);
			if (n > std::string(buff, pos1+ 4).length()) {
				return ret;
			}
		} else 
			return HandleErrors("411 Length Required", server_data, -1);
	}
	while ((HeaderToken = getToken(buff, CRLF)) != "") {
		request.request_headers.insert(std::make_pair(getToken(HeaderToken, HeaderPairsDelim),
								getToken(HeaderToken, HeaderPairsDelim)));
	}
	request.body = buff;
	req_line_data.query = getToken(request.request_line, SP);
	req_line_data.path = getToken(req_line_data.query, "?");
	http_version = request.request_line;
	if (request.request_headers.count("Host") != 1) {
		return HandleErrors("400 Bad Request", server_data, -1);
	}
	req_line_data.server_num = examineServers(request, server_data);
	req_line_data.locations = server_data[req_line_data.server_num].getLocations();
	req_line_data.location_num = examineLocations(req_line_data.locations, req_line_data.path);
	if (http_version != HTTPv1) {
		if (http_version == HTTPv2)
			return HandleErrors("505 HTTP Version Not Supported", server_data, req_line_data.server_num);
		return HandleErrors("400 Bad Request", server_data, req_line_data.server_num);
	}
	if (request.body.length() > server_data[req_line_data.server_num].getClientBodySize() * Mbytes)
		return HandleErrors("413 Payload Too Large", server_data, req_line_data.server_num);
	if (req_line_data.locations[req_line_data.location_num].isCGI() == true)
		return HandleCGI(request, server_data, req_line_data, method);
	if (method == "GET")
		return handle_GET_Request(request, server_data, req_line_data);
	else if (method == "DELETE")
	    return handle_DELETE_Request(request, server_data, req_line_data);
	else if (method == "POST")
	    return handle_POST_Request(request, server_data, req_line_data);
	else if (method == "OPTIONS" || method == "HEAD" || method == "PUT" || method == "PATCH")
		return HandleErrors("501 Not Implemented", server_data, req_line_data.server_num);
	return HandleErrors("400 Bad Request", server_data, req_line_data.server_num);
}
