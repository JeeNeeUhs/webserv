NAME        = webserv
CXX         = c++
CXXFLAGS    = -std=c++98 -I../../inc/config -I../../inc/ -I../../inc/http
SRCS        = utils.cpp \
			  Config.cpp \
			  Location.cpp \
			  Server.cpp \
			  cgi.cpp \
			  HTTPRequest.cpp \
			  main.cpp
VPATH		= ../../src:../../src/config:../../src/http
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