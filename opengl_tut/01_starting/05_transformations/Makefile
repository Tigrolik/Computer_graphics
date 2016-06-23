PLATFORM = $(shell uname)
CXX = g++

# Flags:
#	 -O0 = reduce compilation time and make debugging produce the expected
#	 results (default, see man g++)
#	 -g = does not affect code generation, only the symbol table and debug
#	 metadata are changed (from stackoverflow)
#	 -Wall = turns on all optional warnings
#	 -Wextra = enables some extra warning flags that are not enabled by -Wall
#	 -Wshadow = warn whenever a local variable or type declaration shadows
#	 another (warns if a local var shadows an explicit typedef, but not if it
#	 shadows a struct/class/enum)
#	 -Weffc++ = warn about violations of the style guidelines from Scott
#	 Meyers' Effective C++ series of books

CXXFLAGS = -O3 -g -std=c++11 -Wall -Wextra -Wshadow -pedantic -Werror -Weffc++

# Flags for OpenGL (added -lSOIL for images)
LIBS = -lGL -lGLU -lGLEW -lGLEW -lglfw -lXxf86vm -ldl -lX11 -lpthread\
	-lXrandr -lXi -lXinerama -lXcursor -lSOIL

SOURCES := $(wildcard *.cpp)
TARGETS := main

all:
	$(CXX) $(SOURCES) $(CXXFLAGS) $(LIBS) -o $(TARGETS)

clean:
	@-rm -f $(TARGETS)

distclean: clean
	@-rm -f *~

