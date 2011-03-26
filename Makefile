OBJ=main.o
PROG=search_backend
CXXFLAGS=-std=c++0x

all: $(PROG)
$(PROG): $(OBJ)
	g++ $(CCFLAGS) $(OBJ) -lsqlite3 -o $(PROG)
main.o:	main.cpp packet.h header.h

clean:
	@rm -fv $(OBJ) $(PROG)
