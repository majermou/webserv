/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 13:24:54 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/30 17:39:44 by abel-mak         ###   ########.fr       */
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

#include <algorithm>
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
    : _mypoll(data), _bufSize(1024)
{
}

Server::~Server(void)
{
}

void Server::sendResponse(void)
{
	std::vector<int> wReadyFds = _mypoll.getWriteReadyFds();
	int i;
	int tmp;
	int chunk;

	i = 0;
	while (i < wReadyFds.size())
	{
		// std::cout << i << " wReadyFds: " << wReadyFds.size() << std::endl;
		if (_responseData.find(wReadyFds[i]) != _responseData.end())
		{
			tmp =
			    send(wReadyFds[i], _responseData[wReadyFds[i]].response.data(),
			         _responseData[wReadyFds[i]].response.size(), 0);
			signal(SIGPIPE, SIG_IGN);
			if (tmp < 0)
			{
				// std::cout << "-------B-------" << std::endl;
				// std::cout << strerror(errno) << std::endl;
				// std::cout << wReadyFds[i] << std::endl;
				// std::cout << "-------E-------" << std::endl;
			}
			else if (tmp == _responseData[wReadyFds[i]].response.length())
			{
				if (_responseData[wReadyFds[i]].connection == false)
				{
					_mypoll.clearActiveFd(wReadyFds[i]);
					// std::cout << "-------B-------" << std::endl;
					// std::cout << "sendResp closed" << std::endl;
					close(wReadyFds[i]);
					// std::cout << "-------E-------" << std::endl;
				}
				//	std::cout << "-------B-------" << std::endl;
				//	std::cout << "cleared writeactive and erased" << std::endl;
				_mypoll.clearWriteActiveFd(wReadyFds[i]);
				_responseData.erase(wReadyFds[i]);
				// std::cout << "-------E-------" << std::endl;
			}
			else if (tmp != _responseData[wReadyFds[i]].response.length())
			{
				//	std::cout << tmp << " *************** "
				//	          << _responseData[wReadyFds[i]].response.length()
				//	          << std::endl;
				_responseData[wReadyFds[i]].response =
				    _responseData[wReadyFds[i]].response.substr(tmp);
			}
			// std::cout << tmp << " << tmp | _resdata >> "
			// std::cout << "_resdata"
			//          << _responseData[wReadyFds[i]].response.length()
			//          << std::endl;
		}
		i++;
	}
}

/*
 * add clearWriteFd because _writefds is used in sendResponse
 * so that, don't use closed fd.
 */

void Server::run(void)
{
	std::vector<int> readyFds;
	int i;
	int tmp;
	char buf[_bufSize];
	std::string response;
	struct Ret hr;
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
		readyFds = _mypoll.getReadyfds();
		if (readyFds.size() > 0)
		{
			i = 0;
			while (i < readyFds.size())
			{
				if (_rawRequest.find(readyFds[i]) == _rawRequest.end())
					_rawRequest[readyFds[i]] = "";

				/**************************************************************/

				bzero(buf, _bufSize);
				while ((tmp = recv(readyFds[i], buf, _bufSize - 1, 0)) > 0)
				{
					_rawRequest[readyFds[i]].append(buf, tmp);
				}
				if (tmp < 0 && errno != EAGAIN)
				{
					outputLogs("server error recv: " +
					           std::string(strerror(errno)));
				}
				if (checkCrlf(_rawRequest[readyFds[i]], readyFds[i]) == true ||
				    tmp == 0)
				{
					hr = handleRequest(_rawRequest[readyFds[i]],
					                   _mypoll.getData(), (tmp == 0));
					if (hr.safi == true)
					{
						if (tmp != 0)
						{
							_responseData[readyFds[i]] = hr;
							_mypoll.setWriteActiveFd(readyFds[i]);
							// response = hr.response;
							// send(readyFds[i], response.c_str(),
							//     response.length(), 0);
						}
						if (tmp == 0)
						{
							_mypoll.clearActiveFd(readyFds[i]);
							_mypoll.clearWriteActiveFd(readyFds[i]);
							_mypoll.clearWriteFd(readyFds[i]);
							// std::cout << "-------B-------" << std::endl;
							// std::cout << "other closed" << std::endl;
							close(readyFds[i]);
							// std::cout << "-------E-------" << std::endl;
						}
						_rawRequest.erase(readyFds[i]);
					}
				}
				i++;
			}
		}
		this->sendResponse();
	}
}
