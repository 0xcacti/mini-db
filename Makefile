CC = gcc 
CFLAGS = -Wall -Wextra -Iinclude -g

BIN_DIR = bin
OBJ_DIR = obj
TARGET = bin/main
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=obj/%.o)

default: $(TARGET)

$(TARGET): $(OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

$( BIN_DIR ):
	mkdir -p $( BIN_DIR )
$( OBJ_DIR ):
	mkdir -p $( OBJ_DIR )

clean: 
	rm -rf obj bin

cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean default
	@echo "✓ compile_commands.json regenerated"

.PHONY: default clean cdb


