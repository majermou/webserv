/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 15:48:46 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/13 19:30:03 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Poll.hpp"

#include <cstdlib>

Poll::Poll(void)
{
}

Poll::~Poll(void)
{
	// freeaddrinfo
}

Poll::Poll(std::vector<int> ports) : _queue(10)
{
	struct addrinfo hint;
	char tmpPort[20];
	int ret;
	int optval;
	int i;

	i = 0;
	FD_ZERO(&_activefds);
	while (i < ports.size())
	{
		memset(&hint, 0, sizeof(hint));
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags    = AI_PASSIVE;
		hint.ai_family   = AF_INET;

		bzero(tmpPort, 20);
		sprintf(tmpPort, "%d", ports[i]);

		ret = getaddrinfo(NULL, tmpPort, &hint, &_addrInfoVal);
		if (ret == 0)
		{
			_masterSockets.push_back(socket(_addrInfoVal->ai_family,
			                                _addrInfoVal->ai_socktype,
			                                _addrInfoVal->ai_protocol));
			optval = 1;
			setsockopt(_masterSockets.back(), SOL_SOCKET, SO_REUSEADDR, &optval,
			           sizeof(optval));

			/******************************************************************/

			bind(_masterSockets.back(), _addrInfoVal->ai_addr,
			     _addrInfoVal->ai_addrlen);
			std::cout << "server listen on: "
			          << ntohs(((struct sockaddr_in *)_addrInfoVal->ai_addr)
			                       ->sin_port)
			          << std::endl;
			std::cout << strerror(errno) << std::endl;
			listen(_masterSockets.back(), _queue);
			std::cout << "cur masterSocket: " << _masterSockets.back()
			          << std::endl;
			fcntl(_masterSockets.back(), F_SETFL, O_NONBLOCK);

			/******************************************************************/

			FD_SET(_masterSockets.back(), &_activefds);
		}
		else
		{
			// print error
			// program exit
		}
		i++;
	}
}

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
			slavesocket = accept(_masterSockets[i], _addrInfoVal->ai_addr,
			                     &_addrInfoVal->ai_addrlen);
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
