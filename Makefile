CXXFLAGS = -g -Wall -pedantic -std=c++11 -ferror-limit=50
CXX = clang++

dbms: main.cpp
	${CXX} ${CXXFLAGS} main.cpp -o dbms
