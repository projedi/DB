CXXFLAGS = -g -Wall -pedantic -std=c++11 -ferror-limit=50
CXX = clang++

sources = pagemanager.cpp metadata.cpp
headers = pagemanager.h types.h metadata.h
objects = ${sources:.cpp=.o}

example: example.o ${objects}
	${CXX} example.o ${objects} -o example

example.o: main.cpp ${headers}
	${CXX} ${CXXFLAGS} -c main.cpp -o example.o

${objects}: ${sources} ${headers}
	${CXX} ${CXXFLAGS} -c ${sources}

clean:
	rm *.o
	rm example
