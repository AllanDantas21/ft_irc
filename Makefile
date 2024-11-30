CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = $(shell find . -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)
NAME = ft_irc

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
