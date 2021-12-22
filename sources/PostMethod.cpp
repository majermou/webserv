








struct Ret  handle_POST_Request(Request &request, std::vector<ServerData> &data) {
    std::string path = getToken(request.request_line, SP);
    std::string http_version = request.request_line;
    int serverNum = 0;
    int locationNum = 0;
    std::vector<Location> locations;
    Response response;

    if (request.request_headers.count("Host") != 1) {
        return HandleErrors("400 Bad Request");
    }
    serverNum = examineServers(request, data);
    locations = data[serverNum].getLocations(locations, path);
    locationNum = examineLocations(locations, path);
}