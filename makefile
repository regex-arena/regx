IN=tests/main.c
OUT=bin/main
CC=gcc -Wall -g
LIB_FILES=lib/regx.c
LIB_UPDATE_FILES=lib/regx.c lib/regx.h
LIB_NAME=arena
ifeq ($(OS), Windows_NT)
	echo "WINDOWS BUILD CURRENTLY UNIMPLEMENTED"
	LIB_TARGET = $(LIB_NAME).dll
else
	LIB_TARGET = lib$(LIB_NAME).so
endif

build: l
	$(CC) -c $(IN) -o $(OUT).o
	$(CC) $(OUT).o -o $(OUT) -l$(LIB_NAME) -L ./bin


l: $(LIB_UPDATE_FILES)
	$(CC) -o bin/$(LIB_TARGET) -fpic -shared $(LIB_FILES)

test: build
	LD_LIBRARY_PATH="./bin/" $(OUT)
tv: build
	LD_LIBRARY_PATH="./bin/" valgrind --leak-check=yes $(OUT)

clean:
	rm -rf ./bin/*
