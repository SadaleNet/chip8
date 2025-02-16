PROJECT=chip8
CC=gcc
CFLAGS=-Wall -Werror -pedantic -g -Og -I `pkg-config sdl2 --cflags`
LDFLAGS=`pkg-config sdl2 --libs`
SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

$(BIN_DIR)/$(PROJECT): $(OBJ_FILES)
	mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -R $(BIN_DIR)
	$(RM) -R $(OBJ_DIR)
