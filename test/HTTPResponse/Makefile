NAME        = webserv
CXX         = c++
CXXFLAGS    = -std=c++98 -I../../inc/
SRCS        = HTTPResponse.cpp \
			  main.cpp
VPATH		= ../../src
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