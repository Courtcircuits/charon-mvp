CC      = gcc
CFLAGS  = -Wall -Wextra -g -O2

SRC = src/tunnel.c
BIN = tunnel

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BIN)

# Run both ends in namespaces in one command
dev: all
	./setup-dev-env.sh
	ip netns exec server  ./tunnel 192.168.100.1 5000 5000 &
	ip netns exec client  ./tunnel 192.168.100.2 5000 5001

# Tear everything down
clean-dev:
	ip netns del client 2>/dev/null || true
	ip netns del server 2>/dev/null || true
