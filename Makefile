# Variables first
TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
CFLAGS = -Iinclude

# Setup commands
$(shell mkdir -p obj bin)

# Dependencies
$(OBJ): $(wildcard include/*.h)

# Default target first
default: $(TARGET)

# Main build rules
$(TARGET): $(OBJ)
	gcc $(CFLAGS) -o $@ $^

obj/%.o : src/%.c
	gcc $(CFLAGS) -c $< -o $@

# Utility targets
run: clean default
	./$(TARGET) -f ./mynewdb.db -n 
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean default
	@echo "✓ compile_commands.json regenerated"

.PHONY: default run clean cdb
