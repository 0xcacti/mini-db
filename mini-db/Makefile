# Variables
TARGET_SRV = bin/dbserver
TARGET_CLI = bin/dbcli
SRC_SRV = $(wildcard src/srv/*.c)
OBJ_SRV = $(SRC_SRV:src/srv/%.c=obj/srv/%.o)
SRC_CLI = $(wildcard src/cli/*.c)
OBJ_CLI = $(SRC_CLI:src/cli/%.c=obj/cli/%.o)

default: $(TARGET_SRV) $(TARGET_CLI)

$(TARGET_SRV): $(OBJ_SRV) | bin
	gcc -o $@ $^

$(TARGET_CLI): $(OBJ_CLI) | bin
	gcc -o $@ $^

$(OBJ_SRV): obj/srv/%.o: src/srv/%.c | obj/srv
	gcc -c $< -o $@ -Iinclude

$(OBJ_CLI): obj/cli/%.o: src/cli/%.c | obj/cli
	gcc -c $< -o $@ -Iinclude

# Directory creation
bin obj/srv obj/cli:
	mkdir -p $@

run: clean default
	./$(TARGET_SRV) -f ./mynewdb.db -n -p 8080 &
	./$(TARGET_CLI) 127.0.0.1
	kill -9 $$(pidof dbserver)

clean:
	rm -rf obj bin
	rm -f *.db

cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean default
	@echo "✓ compile_commands.json regenerated"

.PHONY: default run clean cdb
