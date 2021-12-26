/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abel-mak <abel-mak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/26 18:56:39 by abel-mak          #+#    #+#             */
/*   Updated: 2021/12/26 19:16:41 by abel-mak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

#include <iostream>
#include <string>

std::string runCgi(void)
{
	int pid;
	int fd[2];
	int resFd[2];
	int state;
	char buf[1024];
	std::string cgiResponse;
	const char *phpArgv[] = {"/Users/abel-mak/.brew/bin/php-cgi", 0};
	int bytes;

	pipe(fd);
	pipe(resFd);

	setenv("QUERY_STRING", "", 1);
	setenv("REDIRECT_STATUS", "true", 1);
	setenv("SCRIPT_FILENAME", "/Users/abel-mak/test.php", 1);
	setenv("REQUEST_METHOD", "POST", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	setenv("CONTENT_TYPE", "application/x-www-form-urlencoded", 1);
	setenv("CONTENT_LENGTH", "7", 1);

	write(fd[1], "test=42", 7);
	pid = fork();
	if (pid == 0)
	{
		close(fd[1]);
		close(resFd[0]);
		dup2(fd[0], 0);
		dup2(resFd[1], 1);
		execv("/Users/abel-mak/.brew/bin/php-cgi", (char **)phpArgv);
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

int main(void)
{
	runCgi();
}
