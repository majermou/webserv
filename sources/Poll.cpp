/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 15:48:46 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/31 19:34:25 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Poll.hpp"

#include <cstdlib>

#include "../includes/ServerData.hpp"
#include "../includes/Webserv.hpp"

Poll::Poll(void)
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

/*
 * set up socket fds for each port
 * set socket as activefds so we can monitor them using select
 */

Poll::Poll(std::vector<ServerData> data) : _queue(10), _data(data)
{
	int tmpMasterSocket;
	int optval;
	size_t i;
	struct sockaddr_in server;

	FD_ZERO(&_writeActivefds);
	FD_ZERO(&_activefds);
	i = 0;
	while (i < _data.size())
	{
		server.sin_family      = AF_INET;
		server.sin_addr.s_addr = inet_addr(_data[i].getHost().c_str());
		server.sin_port        = htons(_data[i].getPort());

		std::cout << _data[i].getHost().c_str() << std::endl;
		tmpMasterSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (tmpMasterSocket < 0)
		{
			std::cout << strerror(errno) << std::endl;
			// error socket failed
			// exit
		}
		optval = 1;
		setsockopt(tmpMasterSocket, SOL_SOCKET, SO_REUSEADDR, &optval,
		           sizeof(optval));

		/******************************************************************/

		if (bind(tmpMasterSocket, (struct sockaddr *)&server, sizeof(server)) <
		    0)
		{
			outputLogs("server error bind: " + std::string(strerror(errno)));
			if (errno == EADDRINUSE)
			{
				std::cout << ntohs(server.sin_port) << " | " << strerror(errno)
				          << std::endl;
			}
			// error bind failed
		}
		else if (listen(tmpMasterSocket, _queue) == 0)
		{
			std::cout << "server listen on: " << ntohs(server.sin_port)
			          << std::endl;
			std::cout << "cur masterSocket: " << tmpMasterSocket << std::endl;
			fcntl(tmpMasterSocket, F_SETFL, O_NONBLOCK);

			/**************************************************************/

			FD_SET(tmpMasterSocket, &_activefds);

			/**************************************************************/

			_sockAddrVal.push_back(server);
			_masterSockets.push_back(tmpMasterSocket);
		}
		else
		{
			outputLogs("server error listen: " + std::string(strerror(errno)));
			std::cout << strerror(errno) << std::endl;
			// error listen failed
			// exit
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
	size_t i;
	int slavesocket;
	std::vector<int> res;
	std::map<int, std::string> _rawRequest;
	std::string response;
	unsigned int len;

	/**************************************************************************/

	memcpy(&_writefds, &_writeActivefds, sizeof(_writefds));
	memcpy(&_readfds, &_activefds, sizeof(_readfds));

	sret = select(FD_SETSIZE, &_readfds, &_writefds, NULL, NULL);

	i = 0;
	while (i < _masterSockets.size())
	{
		if (FD_ISSET(_masterSockets[i], &_readfds) != 0)
		{
			len         = sizeof(struct sockaddr);
			slavesocket = accept(_masterSockets[i],
			                     ((struct sockaddr *)&_sockAddrVal[i]), &len);
			if (slavesocket < 0)
			{
				outputLogs("server error accept: " +
				           std::string(strerror(errno)));
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

std::vector<int> Poll::getWriteReadyFds(void)
{
	int i;
	std::vector<int> res;

	i = 0;
	while (i < FD_SETSIZE)
	{
		if ((find(_masterSockets.begin(), _masterSockets.end(), i) ==
		     _masterSockets.end()) &&
		    FD_ISSET(i, &_writefds) != 0)
		{
			res.push_back(i);
		}
		i++;
	}
	return (res);
}

bool Poll::isReady(void)
{
	return (_masterSockets.size() > 0);
}

void Poll::setWriteActiveFd(int fd)
{
	FD_SET(fd, &_writeActivefds);
}

void Poll::clearWriteActiveFd(int fd)
{
	FD_CLR(fd, &_writeActivefds);
}

void Poll::clearWriteFd(int fd)
{
	FD_CLR(fd, &_writefds);
}

void Poll::clearActiveFd(int fd)
{
	FD_CLR(fd, &_activefds);
}

std::vector<ServerData> &Poll::getData(void)
{
	return (_data);
}
