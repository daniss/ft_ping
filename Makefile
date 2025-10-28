NAME = ft_ping
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = main.c ping.c utils.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)
SRCS = $(addprefix $(SRCDIR)/, $(SOURCES))

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(NAME) -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re