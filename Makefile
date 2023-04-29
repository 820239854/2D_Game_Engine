################################################################################
# Declare some Makefile variables
################################################################################
CC = g++
LANG_STD = -std=c++17
COMPILER_FLAGS = -Wall -Wfatal-errors -g
INCLUDE_PATH = -I"./libs/"
SRC_FILES = src/*.cpp \
			src/Game/*.cpp \
			src/Logger/*.cpp \
			src/ECS/*.cpp
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua 
OBJ_NAME = main

################################################################################
# Declare some Makefile rules
################################################################################
all: build

build:
	$(CC) $(COMPILER_FLAGS) $(LANG_STD) $(INCLUDE_PATH) $(SRC_FILES) $(LINKER_FLAGS) -o ./out/$(OBJ_NAME)

run:
	./out/$(OBJ_NAME)