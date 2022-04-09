CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -std=c++14

OBJS = magic.o elf_names.o

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

magic : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

magic.o : magic.cpp elf_names.h magic.h

elf_names.o : elf_names.cpp elf_names.h

solution.zip : Makefile magic.cpp elf_names.cpp elf_names.h magic.h README.txt
	zip -9r $@ Makefile magic.cpp elf_names.cpp elf_names.h magic.h README.txt

clean :
	rm -f *.o magic solution.zip
