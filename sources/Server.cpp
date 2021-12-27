/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 13:24:54 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/20 18:53:47 by abel-mak         ###   ########.fr       */
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

#include <cstdio>

bool checkCrlf(std::string &req, int fd)
{
	int i;

	i = 0;
	return (req.find("\r\n\r\n") != std::string::npos);
}

Server::Server(void) : _bufSize(1024)
{
}

Server::Server(const std::vector<ServerData> &data)
    : _mypool(data), _bufSize(1024)
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
	struct Ret r;
	std::string str = "\r\n\r\n";

	response =
	    "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
	    "\n"
	    "<div style=\"font-family: monospace; white-space: pre;\">"
	    " _    _  _____ ______  _____  _____ ______  _   _ <br>"
	    "| |  | ||  ___|| ___ \\/  ___||  ___|| ___ \\| | | |<br>"
	    "| |  | || |__  | |_/ /\\ `--. | |__  | |_/ /| | | |<br>"
	    "| |/\\| ||  __| | ___ \\ `--. \\|  __| |    / | | | |<br>"
	    "\\  /\\  /| |___ | |_/ //\\__/ /| |___ | |\\ \\ \\ \\_/ /<br>"
	    " \\/  \\/ \\____/ \\____/ \\____/ \\____/ \\_| \\_| \\___/ <br>"
	    "</div>"
	    "<style>body{margin-left: auto;margin-right: auto;max-width: "
	    "1000px;}</style>";
	while (1)
	{
		readyFds = _mypool.getReadyfds();
		if (readyFds.size() > 0)
		{
			i = 0;
			while (i < readyFds.size())
			{
				if (_rawRequest.find(readyFds[i]) == _rawRequest.end())
					_rawRequest[readyFds[i]] = "";

				/**************************************************************/

				bzero(buf, _bufSize);
					// std::cout << "==================================START===================================\n";
				while ((tmp = recv(readyFds[i], buf, _bufSize - 1, 0)) > 0)
				{
					_rawRequest[readyFds[i]] += buf;
					std::cout << buf;
				}
				if (tmp < 0)
				{
					// error
					// exit
				}
			
				//std::cout << _rawRequest[readyFds[i]] << "\n";
				// std::cout << "==================================END===================================\n";
				if (checkCrlf(_rawRequest[readyFds[i]], readyFds[i]) == true)
				{
					response = (handleRequest(_rawRequest[readyFds[i]],
					                          _mypool.getData()))
					               .response;
					send(readyFds[i], response.c_str(), response.length(), 0);
					_rawRequest.erase(readyFds[i]);
					_mypool.clearActiveFd(readyFds[i]);
					close(readyFds[i]);
				}
				i++;
			}
		}
	}
}
