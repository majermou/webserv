/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 11:34:23 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/30 12:26:56 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Server_HPP
#define Server_HPP

#include <string>

#include "Poll.hpp"
#include "ServerData.hpp"

class Server
{
public:
	Server(const std::vector<ServerData> &data);
	~Server(void);
	// void serverListen(std::string port);
	// void serverStopListen(void);
	void run(void);

private:
	void sendResponse(void);
	Server(void);
	Poll _mypoll;
	// int _masterSocket;
	// struct addrinfo *_addrinfoVal;
	const int _bufSize;
	// std::string _rawRequest;
	std::map<int, std::string> _rawRequest;
	std::map<int, struct Ret> _responseData;
};

#endif /* ifndef Server_HPP*/
