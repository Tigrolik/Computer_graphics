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

COMP_OPTIONS = -m64 -Wall -Wextra -Wshadow -pedantic -Werror
CXXFLAGS = -std=c++11 -Weffc++ $(COMP_OPTIONS)
DEBUG_FLAGS = -O0 -g -D _DEBUG
RELEASE_FLAGS = -O3 -D NDEBUG

# Flags for OpenGL (added -lSOIL for images)
LIBS = -lGL -lGLU -lGLEW -lGLEW -lglfw -lXxf86vm -ldl -lX11 -lpthread\
	-lXrandr -lXi -lXinerama -lXcursor -lSOIL -lassimp


SOURCES := $(wildcard *.cpp)
TARGET := main

debug:
	$(CXX) $(SOURCES) $(CXXFLAGS) $(DEBUG_FLAGS) $(LIBS) -o $(TARGET)

release:
	$(CXX) $(SOURCES) $(CXXFLAGS) $(RELEASE_FLAGS) $(LIBS) -o $(TARGET)

clean:
	@-rm -f $(TARGET)

distclean: clean
	@-rm -f *~

