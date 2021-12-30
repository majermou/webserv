/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/26 18:56:39 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/30 17:47:04 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"

std::string runCgi(CGIparam p)
{
	int pid;
	int fd[2];
	int resFd[2];
	int state;
	char buf[1024];
	std::string cgiResponse;
	char *phpArgv[2] = {0, 0};
	int bytes;

	phpArgv[0] = const_cast<char *>(p.fastcgipass.c_str());

	pipe(fd);
	pipe(resFd);

	setenv("QUERY_STRING", p.query.c_str(), 1);
	setenv("REDIRECT_STATUS", "true", 1);
	setenv("SCRIPT_FILENAME", p.path.c_str(), 1);
	setenv("REQUEST_METHOD", p.method.c_str(), 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	setenv("CONTENT_TYPE", p.content_type.c_str(), 1);
	setenv("CONTENT_LENGTH", p.content_length.c_str(), 1);

	write(fd[1], p.body.c_str(), atoi(p.content_length.c_str()));
	pid = fork();
	if (pid == 0)
	{
		close(fd[1]);
		close(resFd[0]);
		dup2(fd[0], 0);
		dup2(resFd[1], 1);
		execv(phpArgv[0], (char **)phpArgv);
	}
	else
	{
		close(fd[0]);
		close(resFd[1]);

		waitpid(pid, &state, 0);

		while ((bytes = read(resFd[0], buf, 1023)) > 0)
		{
			buf[bytes] = '\0';
			cgiResponse += buf;
		}
		std::cout << cgiResponse << std::endl;
	}
	close(fd[1]);
	close(resFd[0]);
	return (cgiResponse);
}
