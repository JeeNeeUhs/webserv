NAME		= webserv

BUILD_DIR	= build

VPATH		= src:src/utils:src/config:src/server:src/http:src/handler:test/unit

SRCS		= main.cpp \
			utils.cpp Logger.cpp \
			Config.cpp ConfigParser.cpp \
			ServerManager.cpp Listener.cpp \
			HTTPResponse.cpp HTTPRequest.cpp HTTPParser.cpp \
			RequestHandler.cpp StaticHandler.cpp ErrorResponse.cpp \
			UploadStore.cpp cgi.cpp SessionHandler.cpp

OBJS		= $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

INCS		= -Iinc -Iinc/utils -Iinc/config -Iinc/server \
			-Iinc/http -Iinc/handler

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

test-stress: $(NAME)
	siege -b http://127.0.0.1:8080/

clean:
	$(RM) $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)
	$(RM) $(TEST_NAME)

re: fclean all

.PHONY: all test-unit test-stress clean fclean re
