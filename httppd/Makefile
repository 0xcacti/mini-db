CC = gcc 
CFLAGS = -Wall -Wextra -Iinclude -g

SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
BIN_DIR = bin

LD_FLAGS = -L./lib -lcjson

TARGET = $(BIN_DIR)/myhttpd

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -g $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean all
	@echo "✓ compile_commands.json regenerated"

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean cdb



# BIN_DIR = bin
# OBJ_DIR = obj
# TARGET = bin/main
# SRC = $(wildcard src/*.c)
# OBJ = $(SRC:src/%.c=obj/%.o)
# 
# default: $(TARGET)
# 
# $(TARGET): $(OBJ) | bin
# 	$(CC) $(CFLAGS) -o $@ $^
# 
# $(OBJ): obj/%.o: src/%.c | obj
# 	$(CC) $(CFLAGS) -c $< -o $@
# 
# $( BIN_DIR ):
# 	mkdir -p $( BIN_DIR )
# $( OBJ_DIR ):
# 	mkdir -p $( OBJ_DIR )
# 
# clean: 
# 	rm -rf obj bin
# 
# 
# .PHONY: default clean cdb


