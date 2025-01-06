CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
OBJDIR = obj
SRCS = $(shell find . -name "*.cpp")
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
NAME = ft_irc

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all