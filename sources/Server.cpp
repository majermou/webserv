/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 13:24:54 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/14 15:20:27 by abel-mak         ###   ########.fr       */
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

/*
 * Why FD_SET/FD_ZERO for select() inside of loop?
 * When select returns, it has updated the sets to show which file descriptors
 * have become ready for read/write/exception. All other flags have been
 * cleared. It's important that you re-enable the file descriptors that were
 * cleared prior to starting another select, otherwise, you will no longer be
 * waiting on those file descriptors.
 */

/* FD_ISSET
 * Returns a non-zero value if the file descriptor is set in the file descriptor
 *  set pointed to by fdset; otherwise returns 0.
 */

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
				// std::cout << "readyFds: " << readyFds[i] << std::endl;
				bzero(buf, _bufSize);
				while ((tmp = recv(readyFds[i], buf, _bufSize - 1, 0) >= 0))
				{
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

