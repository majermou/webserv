/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 13:24:54 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/13 16:28:47 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

#include "../includes/Webserv.hpp"

/*
 * SO_REUSEADDR
 * This socket option tells the kernel that even if this port is busy (in
 * the TIME_WAIT state), go ahead and reuse it anyway.  If it is busy,
 * but with another state, you will still get an address already in use
 * error
 */

std::vector<int> getAllPorts(const std::vector<ServerData> &data)
{
	size_t i;
	std::vector<int> allPorts;
	std::vector<int> svPorts;

	i = 0;
	while (i < data.size())
	{
		svPorts = data[i++].getPorts();
		reverse(svPorts.begin(), svPorts.end());
		allPorts.insert(allPorts.begin(), svPorts.begin(), svPorts.end());
	}
	reverse(allPorts.begin(), allPorts.end());
	return (allPorts);
}

Server::Server(void) : _bufSize(1024)
{
}

Server::Server(const std::vector<ServerData> &data)
    : _mypool(getAllPorts(data)), _bufSize(1024)
{
}

Server::~Server(void)
{
}

void Server::run(void)
{
	std::vector<int> readyFds;
	int i;
	int tmp;
	char buf[_bufSize];
	std::string response;

	response =
	    "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\nSIR "
	    "T9WAD\n";
	while (1)
	{
		readyFds = _mypool.getReadyfds();

		if (readyFds.size() > 0)
		{
			i = 0;
			while (i < readyFds.size())
			{
				_rawRequest = "";
				std::cout << "readyFds: " << readyFds[i] << std::endl;
				while ((tmp = recv(readyFds[i], buf, _bufSize - 1, 0) > 0))
				{
					buf[tmp] = '\0';
					_rawRequest += buf;
				}
				if (tmp < 0)
				{
					// error
					// exit
				}
				send(readyFds[i], response.c_str(), response.length(), 0);
				_mypool.clearActiveFd(readyFds[i]);
				close(readyFds[i]);
				i++;
			}
			// std::cout << readyFds[i] << " =====================" <<
			// std::endl;
		}
	}
}

