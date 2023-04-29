files=src/*.cpp  src/Game/*.cpp src/Logger/*.cpp
libs=-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua

all: build

build:
	g++ -o ./out/main -std=c++17 -Wall -g  ${files} ${libs}

run:
	./out/main.exe