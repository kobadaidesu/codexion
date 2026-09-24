NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRC =	src/main.c \
		src/parse.c \
		src/init.c \
		src/thread.c \
		src/dongle_take.c \
		src/dongle_release.c \
		src/heap.c \
		src/coder.c \
		src/state.c \
		src/monitor.c \
		src/time_log.c \

OBJ = $(SRC:.c=.o)

INCLUDES = -Iincludes

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
