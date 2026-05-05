NAME		= webserv

INC_DIR		= inc
SRC_DIR		= src
BUILD_DIR	= build

VPATH		= src:src/config
SRCS		= main.cpp
OBJS		= $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
RM			= rm -rf

all: $(NAME)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $^ -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
