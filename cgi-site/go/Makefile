NAME    = guestbook
SRC     = guestbook.go
RM      = rm -f

all: $(NAME)

$(NAME): $(SRC)
	go build -o $(NAME) $(SRC)

clean:
	$(RM) $(NAME)

re: clean all

.PHONY: all clean re
