COMPILER = g++

FLAGS = -std=c++1y -pedantic -Wall -Wl -rpath

GL_FLAGS = -I -lGL -lglfw -ldl

FILES = $(wildcard src/*.cpp) $(wildcard src/*.c)

APP_NAME = main

all: main

main: $(FILES) 
	$(COMPILER) $(FLAGS) '$ORIGIN' $(FILES) -o $(APP_NAME) $(GL_FLAGS)

.PHONY: clean run

clean: 
	rm -rvf *.o $(APP_NAME)

run: ./$(APP_NAME)