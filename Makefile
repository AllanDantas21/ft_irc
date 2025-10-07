CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
OBJDIR = obj
SRCS = $(shell find srcs -name "*.cpp")
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
NAME = ircserv

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
	$(MAKE) -C tests/bdd fclean

re: fclean all

test:
	$(MAKE) -C tests/bdd test

# The following commands are used for containerized builds in the macOS environment.
# Ensure you have the 'container' command available, which is a native macOS Docker,
# realeased in june 2025.
# If you want to use Docker directly, replace 'container' with 'docker'.

container-build:
	container system start && container build -t ft_irc_debian .

container-run:
	container run --rm -it -v $(PWD):/app -w /app ft_irc_debian make

container-shell:
	container run --rm -it -v $(PWD):/app -w /app ft_irc_debian bash

container:
	$(MAKE) container-build
	$(MAKE) container-run
	$(MAKE) container-shell

container-stop:
	container stop --all && container system stop
