# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abel-mak & majermou                        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/12/31 14:10:42 by abel-mak          #+#    #+#              #
#    Updated: 2021/12/31 15:37:25 by abel-mak         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=webserv

SRC= main.cpp $(wildcard ./sources/*.cpp)

OBJ= $(SRC:.cpp=.o)

FLAGS= -std=c++98 -Wall -Werror -Wextra

all: $(NAME)

$(NAME): $(OBJ)
	clang++  $(FLAGS) $(OBJ) -o  $(NAME)

%.o:%.cpp $(HDR)
	clang++ -c $(FLAGS) $< -o $@

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all
