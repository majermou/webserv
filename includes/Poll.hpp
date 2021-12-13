/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 15:32:24 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/13 15:54:28 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POLL_HPP
#define POLL_HPP

#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <iostream>
#include <vector>

class Poll
{
private:
	std::vector<int> _masterSockets;
	fd_set _readfds;
	fd_set _activefds;
	int _queue;
	struct addrinfo *_addrInfoVal;

public:
	Poll(void);
	Poll(std::vector<int> ports);
	~Poll(void);
	std::vector<int> getReadyfds(void);
	void clearActiveFd(int fd);
};

#endif /* ifndef POLL */
