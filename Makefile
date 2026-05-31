NAME		= webserv
TEST_NAME	= webserv_tester

BUILD_DIR	= build

VPATH		= src:src/utils:src/config:src/server:src/http:src/handler:test/unit

SRCS		= main.cpp \
			utils.cpp Logger.cpp \
			Config.cpp ConfigParser.cpp \
			ServerManager.cpp Listener.cpp \
			HTTPResponse.cpp HTTPRequest.cpp HTTPParser.cpp \
			ErrorResponse.cpp RequestHandler.cpp StaticHandler.cpp
TEST_SRCS	= tests.cpp test_http.cpp test_config.cpp

OBJS		= $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))
TEST_OBJS	= $(addprefix $(BUILD_DIR)/,$(TEST_SRCS:.cpp=.o))
# exclude the main func for compile tests
CORE_OBJS	= $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

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

$(TEST_NAME): $(BUILD_DIR) $(CORE_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CORE_OBJS) $(TEST_OBJS)

test-unit: $(TEST_NAME)

test-stress: $(NAME)
	siege -b http://127.0.0.1:8080/

clean:
	$(RM) $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)
	$(RM) $(TEST_NAME)

re: fclean all

.PHONY: all test-unit test-stress clean fclean re
