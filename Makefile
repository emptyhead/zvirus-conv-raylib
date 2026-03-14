CC ?= cc
CFLAGS ?= -std=c11 -O2 -g -Wall -Wextra -Wpedantic

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib)

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

BIN := zvirus

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(RAYLIB_LIBS) -lm

HDR := $(wildcard src/*.h)

src/%.o: src/%.c $(HDR)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) $(OBJ)

