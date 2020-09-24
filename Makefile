COMPILER = g++

FLAGS = -std=c++1y -pedantic -Wall

GL_FLAGS = -lglfw3 -lGL -lm -lX11 -lpthread -lXi -lXrandr -ldl

FILES = $(wildcard src/*.cpp) $(wildcard src/*.c)

APP_NAME = main

all: main

main: $(FILES) 
	$(COMPILER) $(FLAGS) $(FILES) -o $(APP_NAME) $(GL_FLAGS)

.PHONY: clean run

clean: 
	rm $(APP_NAME)

run: ./$(APP_NAME)
