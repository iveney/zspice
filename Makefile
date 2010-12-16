CC=g++
SRC=util.cpp net.cpp io.cpp node.cpp triplet.cpp
HDR=$(SRC:.cpp=.h)
OBJ=$(SRC:.cpp=.o) dc_linear.o dc_nonlinear.o ac_analysis.o tran_analysis.o
BIN=zspice
DBG=debug
CFLAGS=
OPT=-Wall -g 
SPARSE=SuiteSparse
# comment this and uncomment the next line if want to use other lib location 
MYLIBS=./libs/*.a
#LIBS=$(SPARSE)/UMFPACK/Lib/libumfpack.a \
     $(SPARSE)/AMD/Lib/libamd.a \
     $(SPARSE)/CHOLMOD/Lib/libcholmod.a \
     $(SPARSE)/COLAMD/Lib/libcolamd.a \
     $(SPARSE)/CCOLAMD/Lib/libccolamd.a \
     $(SPARSE)/CAMD/Lib/libcamd.a \
     $(SPARSE)/metis-4.0/libmetis.a \
     $(SPARSE)/GotoBLAS2/libgoto2.a
AMD_HDR=$(SPARSE)/AMD/Include
UMFPACK_HDR=$(SPARSE)/UMFPACK/Include
SPARSE_HDR=$(SPARSE)/UFconfig
INC=-I$(AMD_HDR) \
    -I$(UMFPACK_HDR) \
    -I$(SPARSE_HDR)
MYINC=-I./headers
CSCOPEFILES=cscope.files cscope.out cscope.po.out

main: $(OBJ) main.o global.h 
	@echo "Making zspice..."
	$(CC) $(OPT) $(CFLAGS) $(INC) -lpthread -o $(BIN) $(OBJ) $(MYLIBS) main.o 
	@make tags

all: main debug test
	@echo "Making all..."

debug: $(OBJ) main.cpp main.h
	@echo "Making debug..."
	$(CC) -c $(OPT) $(CFLAGS) -DDEBUG $(SRC) main.cpp
	$(CC) $(OPT) $(CFLAGS) $(INC) -o $(DBG) $(OBJ) $(MYLIBS) main.o 

test: test.cpp
	@echo "Making test"
	$(CC) -c $(OPT) $(CFLAGS) $(INC) test.cpp
	$(CC) $(OPT) $(CFLAGS) $(INC) -o test $(MYLIBS) $(OBJ) test.o

%.o: %.cpp  %.h global.h
	$(CC) -c $< $(OPT) -o $@

dc_linear.o: dc_linear.cpp dc_linear.h
	$(CC) -c $(OPT) $(CFLAGS) $(INC) dc_linear.cpp

dc_nonlinear.o: dc_nonlinear.h dc_nonlinear.cpp
	$(CC) -c $(OPT) $(CFLAGS) $(INC) dc_nonlinear.cpp

ac_analysis.o: ac_analysis.h ac_analysis.cpp
	$(CC) -c $(OPT) $(CFLAGS) $(INC) ac_analysis.cpp

tran_analysis.o: tran_analysis.h tran_analysis.cpp
	$(CC) -c $(OPT) $(CFLAGS) $(INC) tran_analysis.cpp

copy_libs: $(LIBS) $(AMD_HDR) $(UMFPACK_HDR) $(SPARSE_HDR)
	mkdir -p ./headers
	mkdir -p ./libs
	cp -r $(AMD_HDR)/*.h ./headers/
	cp -r $(UMFPACK_HDR)/*.h ./headers/
	cp -r $(SPARSE_HDR)/*.h ./headers/
	for i in $(LIBS); do \
		cp $$i ./libs; \
	done

tags: $(SRC) $(HDR) main.cpp main.h
	@echo "Generating tags..."
	@find . -maxdepth 1 -name "*.h" -o -name "*.c" \
		-o -name "*.cpp" > cscope.files
	@cscope -bkq -i cscope.files
	@ctags -L cscope.files --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q

.PHONY : clean
clean:
	@echo "Cleaning all..."
	rm -rf *.o $(OBJ) $(DBG) $(BIN) test tags $(CSCOPEFILES)
