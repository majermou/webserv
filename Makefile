# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abel-mak & majermou                        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/12/31 14:10:42 by abel-mak          #+#    #+#              #
#    Updated: 2021/12/31 14:11:31 by abel-mak         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=webserv

SRC=  main.cpp $(wildcard ./sources/*.cpp)

all: $(NAME)

$(NAME):
	clang++ -Wall -Werror -Wextra $(SRC) -o  $(NAME)

clean:
	rm $(NAME)
