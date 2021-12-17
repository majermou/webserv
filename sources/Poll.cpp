/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 15:48:46 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/17 19:25:00 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Poll.hpp"

#include <cstdlib>

#include "../includes/ServerData.hpp"
#include "../includes/Webserv.hpp"

Poll::Poll(void) : _addrInfoVal()
{
}

Poll::~Poll(void)
{
	std::vector<int>::iterator it;

	it = _masterSockets.begin();
	while (it != _masterSockets.end())
	{
		close(*it);
		it++;
	}
}

std::vector<int> getAllPorts(const std::vector<ServerData> &data)
{
	size_t i;
	std::vector<int> allPorts;

	i = 0;
	while (i < data.size())
	{
		allPorts.push_back(data[i].getPort());
		i++;
	}
	return (allPorts);
}

/*
 * set up socket fds for each port
 * set socket as activefds so we can monitor them using select
 */

Poll::Poll(std::vector<ServerData> data)
    : _queue(10), _addrInfoVal(), _data(data)
{
	struct addrinfo hint;
	struct addrinfo *tmpAddrInfo;
	int tmpMasterSocket;
	char tmpPort[11];
	int ret;
	int optval;
	int i;

	FD_ZERO(&_activefds);
	i = 0;
	while (i < _data.size())
	{
		memset(&hint, 0, sizeof(hint));
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags    = AI_PASSIVE;
		hint.ai_family   = AF_INET;

		bzero(tmpPort, 11);
		sprintf(tmpPort, "%d", _data[i].getPort());
		ret = getaddrinfo(_data[i].getHost().c_str(), tmpPort, &hint,
		                  &tmpAddrInfo);
		std::cout << _data[i].getHost().c_str() << std::endl;
		if (ret == 0)
		{
			tmpMasterSocket =
			    socket(tmpAddrInfo->ai_family, tmpAddrInfo->ai_socktype,
			           tmpAddrInfo->ai_protocol);
			if (tmpMasterSocket < 0)
			{
				outputLogs(std::string("server socket: ") +
				           std::string(strerror(errno)));
				// error socket failed
				// exit
			}
			optval = 1;
			setsockopt(tmpMasterSocket, SOL_SOCKET, SO_REUSEADDR, &optval,
			           sizeof(optval));

			/******************************************************************/

			if (bind(tmpMasterSocket, tmpAddrInfo->ai_addr,
			         tmpAddrInfo->ai_addrlen) < 0)
			{
				outputLogs(std::string("server bind: ") +
				           std::string(strerror(errno)));
				std::cout << strerror(errno) << std::endl;
				// error bind failed
			}
			else if (listen(tmpMasterSocket, _queue) == 0)
			{
				std::cout << "server listen on: "
				          << ntohs(((struct sockaddr_in *)tmpAddrInfo->ai_addr)
				                       ->sin_port)
				          << std::endl;
				std::cout << "cur masterSocket: " << tmpMasterSocket
				          << std::endl;
				fcntl(tmpMasterSocket, F_SETFL, O_NONBLOCK);

				/**************************************************************/

				FD_SET(tmpMasterSocket, &_activefds);

				/**************************************************************/
				_addrInfoVal.resize(_addrInfoVal.size() + 1);
				memcpy(&_addrInfoVal.back(), tmpAddrInfo,
				       sizeof(struct addrinfo));
				_masterSockets.push_back(tmpMasterSocket);
				freeaddrinfo(tmpAddrInfo);
			}
			else
			{
				freeaddrinfo(tmpAddrInfo);
				outputLogs(std::string("server listen: ") +
				           std::string(strerror(errno)));
				// free tmpAddrInfo
				// error listen failed
				// exit
			}
		}
		else
		{
			outputLogs(std::string("server getaddrinfo: ") +
			           std::string(strerror(errno)));
			// print error
			// program exit
		}
		i++;
	}
}

/*
 * copy activefds to readfds because select is distructive, we can't give it
 * directly activefds it will overwrite it.
 * select return ready to read descriptors from activefds.
 * check for incoming connection and add them to activefds so we can monitore...
 * return all ready file descriptors
 */

std::vector<int> Poll::getReadyfds(void)
{
	int sret;
	int i;
	int slavesocket;
	std::vector<int> res;

	memcpy(&_readfds, &_activefds, sizeof(_readfds));
	sret = select(FD_SETSIZE, &_readfds, NULL, NULL, NULL);

	i = 0;
	while (i < _masterSockets.size())
	{
		if (FD_ISSET(_masterSockets[i], &_readfds) != 0)
		{
			slavesocket = accept(_masterSockets[i], _addrInfoVal[i].ai_addr,
			                     &_addrInfoVal[i].ai_addrlen);
			if (slavesocket < 0)
			{
				// error accept
				// exit
			}
			else if (slavesocket > 0)
			{
				FD_SET(slavesocket, &_activefds);
				fcntl(slavesocket, F_SETFL, O_NONBLOCK);
				std::cout << "Accepted connection on listening socket: "
				          << _masterSockets[i] << std::endl;
			}
		}
		i++;
	}

	i = 0;
	while (i < FD_SETSIZE)
	{
		if ((find(_masterSockets.begin(), _masterSockets.end(), i) ==
		     _masterSockets.end()) &&
		    FD_ISSET(i, &_readfds) != 0)
		{
			res.push_back(i);
		}
		i++;
	}
	return (res);
}

void Poll::clearActiveFd(int fd)
{
	FD_CLR(fd, &_activefds);
}

std::vector<ServerData> &Poll::getData(void)
{
	return (_data);
}
