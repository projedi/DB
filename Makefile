LIBS = -lboost_filesystem -lboost_system
CPPFLAGS = -g -Wall -pedantic -std=c++11 -O2
#CPPFLAGS = -Wall -pedantic -std=c++11 -O2 -g -pg
#CXX = clang++
CXX = g++
LD = ${CXX}
LDFLAGS = 
#LDFLAGS = -pg
BUILDDIR = build

sources = database.cpp types.cpp table.cpp page.cpp file.cpp insert.cpp select.cpp main.cpp
#headers = page.h page_impl.h file.h file_impl.h \
#			 database.h database_impl.h cache.h cache_impl.h
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
