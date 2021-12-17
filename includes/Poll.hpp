/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 15:32:24 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/16 17:22:59 by abel-mak         ###   ########.fr       */
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
	int _queue;
	std::vector<struct addrinfo> _addrInfoVal;
	std::vector<ServerData> _data;

public:
	Poll(void);
	Poll(std::vector<ServerData> data);
	~Poll(void);
	std::vector<int> getReadyfds(void);
	void clearActiveFd(int fd);
};

#endif /* ifndef POLL */
