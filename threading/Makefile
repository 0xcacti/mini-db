SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
CFLAGS = -Iinclude -Wall -Wextra -O2

TARGET = $(BIN)/main

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

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
