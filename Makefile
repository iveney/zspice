CC=g++
SRC=util.cpp net.cpp io.cpp node.cpp dc_linear.cpp
HDR=$(SRC:.cpp=.h)
OBJ=$(SRC:.cpp=.o)
BIN=zspice
DBG=debug
CFLAGS=
OPT=-Wall -g
SPARSE=SuiteSparse
LIBS=$(SPARSE)/UMFPACK/Lib/libumfpack.a \
     $(SPARSE)/AMD/Lib/libamd.a \
     $(SPARSE)/CHOLMOD/Lib/libcholmod.a \
     $(SPARSE)/COLAMD/Lib/libcolamd.a \
     $(SPARSE)/CCOLAMD/Lib/libccolamd.a \
     $(SPARSE)/CAMD/Lib/libcamd.a \
     $(SPARSE)/metis-4.0/libmetis.a \
     $(SPARSE)/GotoBLAS2/libgoto2.a
INC=-I$(SPARSE)/AMD/Include \
    -I$(SPARSE)/UMFPACK/Include\
    -I$(SPARSE)/UFconfig
CSCOPEFILES=cscope.files cscope.out cscope.po.out

main: $(OBJ) main.o tags
	@echo "Making zspice..."
	$(CC) $(OPT) $(CFLAGS) $(INC) -o $(BIN) $(OBJ) $(LIBS) main.o 

all: main debug test
	@echo "Making all..."

debug: $(OBJ) main.cpp main.h
	@echo "Making debug..."
	$(CC) -c $(OPT) $(CFLAGS) -DDEBUG $(SRC) main.cpp
	$(CC) $(OPT) $(CFLAGS) $(INC) -o $(DBG) $(OBJ) $(LIBS) main.o 

test: test.cpp
	@echo "Making test"
	$(CC) $(OPT) $(CFLAGS) $(INC) -o test $(LIBS) test.cpp

%.o: %.cpp  %.h
	$(CC) -c $< $(OPT) -o $@

tags: $(SRC) $(HDR) main.cpp main.h
	@echo "Generating tags..."
	@find . -maxdepth 1 -name "*.h" -o -name "*.c" \
		-o -name "*.cpp" > cscope.files
	@cscope -bkq -i cscope.files
	@ctags -L cscope.files --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q

.PHONY : clean
clean:
	@echo "Cleaning all..."
	rm -rf *.o $(OBJ) $(DBG) $(BIN) tags $(CSCOPEFILES)
