NAME		= webserv

BUILD_DIR	= build

VPATH		= src:src/utils:src/config
SRCS		= main.cpp \
			utils.cpp Logger.cpp \
			Config.cpp ConfigParser.cpp
OBJS		= $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))
INCS		= -Iinc -Iinc/utils -Iinc/config

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 $(INCS)
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
