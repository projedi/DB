LIBS = -lboost_filesystem -lboost_system
#CPPFLAGS = -Wall -pedantic -std=c++11 -g
#CPPFLAGS = -Wall -pedantic -std=c++11 -O2 -g -pg
CPPFLAGS = -Wall -pedantic -std=c++11 -O2
CXX = clang++
#CXX = g++
LD = ${CXX}
LDFLAGS = 
#LDFLAGS = -pg
BUILDDIR = build

sources = database.cpp types.cpp page.cpp file.cpp cmdlist.cpp main.cpp hash.cpp table.cpp index.cpp
objects = ${sources:%.cpp=$(BUILDDIR)/%.o}
depends = ${sources:%.cpp=$(BUILDDIR)/%.d}

all: example

include ${depends}

example: ${objects}
	${LD} ${LDFLAGS} ${LIBS} -o $@ ${objects}

${BUILDDIR}/%.o: %.cpp
	${CXX} ${CPPFLAGS} -c -o $@ $<

${BUILDDIR}/%.d: %.cpp
	${CXX} ${CPPFLAGS} -MF"$@" -MG -MM -MP -MT"$@" -MT"$(<:%.cpp=$(BUILDDIR)/%.o)" "$<"

clean:
	rm ${BUILDDIR}/*
	rm example
