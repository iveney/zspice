CC=g++
SRC=util.cpp
HDR=$(SRC:.cpp=.h)
OBJ=$(SRC:.cpp=.o)
BIN=zspice
DBG=debug
OPT=-Wall -g

main: $(OBJ) main.o tags
	echo "Making zspice..."
	$(CC) $(OPT) -o $(BIN) $(OBJ) main.o

all: main debug
	@echo "Making all..."

debug: $(OBJ) main.cpp main.h
	@echo "Making debug..."
	$(CC) -c $(OPT) -DDEBUG $(SRC) main.cpp
	$(CC) $(OPT) -o $(DBG) $(OBJ) main.o

%.o: %.cpp  %.h
	$(CC) -c $< $(OPT) -o $@

tags: $(SRC) $(HDR) main.cpp main.h
	@echo "Making tags..."
	cscopegen

.PHONY : clean
clean:
	@echo "Cleaning all..."
	rm -rf *.o $(OBJ) $(DBG) $(BIN)
