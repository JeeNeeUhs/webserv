NAME        = test_http_parser
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -I../../inc/
SRCS        = HTTPRequest.cpp \
              main.cpp
VPATH       = ../../src
OBJS        = $(addprefix $(BUILD), $(SRCS:.cpp=.o))
RM          = rm -rf
BUILD       = build/

all: $(BUILD) $(NAME)

$(BUILD):
	@mkdir -p $(BUILD)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(BUILD)%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(BUILD)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
