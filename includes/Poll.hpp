/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 15:32:24 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/31 19:27:19 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POLL_HPP
#define POLL_HPP

#include "../includes/ServerData.hpp"
#include "../includes/Webserv.hpp"

class Poll
{
private:
	std::vector<int> _masterSockets;
	fd_set _readfds;
	fd_set _activefds;
	fd_set _writefds;
	fd_set _writeActivefds;
	int _queue;
	std::vector<struct sockaddr_in> _sockAddrVal;
	std::vector<ServerData> _data;

public:
	Poll(void);
	Poll(std::vector<ServerData> data);
	~Poll(void);
	std::vector<int> getReadyfds(void);
	void clearActiveFd(int fd);
	void setWriteActiveFd(int fd);
	void clearWriteActiveFd(int fd);
	void clearWriteFd(int fd);
	std::vector<ServerData> &getData(void);
	std::vector<int> getWriteReadyFds(void);
	bool isReady(void);
};

#endif /* ifndef POLL */
