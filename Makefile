TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
CFLAGS = -Iinclude
$(OBJ): $(wildcard include/*.h)

run: clean default
	./$(TARGET) -f ./mynewdb.db -n 
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db


$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude

cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean default
	@echo "✓ compile_commands.json regenerated"

