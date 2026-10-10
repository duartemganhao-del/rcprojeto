CC = gcc
CFLAGS = -Wall -Wextra -pedantic

SRC = user.c config.c validation.c udp.c tcp.c login.c logout.c unregister.c publish.c remove.c list.c versions.c

all: user

user: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o user

clean:
	rm -f user