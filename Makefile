LIBS=-lGL -lGLU -lglut -lGLEW
CC=g++ -I/opt/local/include -L/opt/local/lib -g -std=c++11

SOURCES=main.cpp tree.cpp branch.cpp tga.cpp clock.cpp leaf.cpp spherical_coordinates.cpp sky.cpp fallinfo.cpp
HEADERS=tree.h branch.h tga.h clock.h leaf.h spherical_coordinates.h sky.h fallinfo.h
OBJECTS=$(SOURCES:.cpp=.o)

all: tree

tree: $(OBJECTS) 
	$(CC) -o $@ $(addprefix bin/, $(OBJECTS)) $(LIBS)

$(OBJECTS): %.o: src/%.cpp $(addprefix src/, $(HEADERS))
	mkdir -p bin/
	$(CC) -c $< -o bin/$@
	
clean:
	rm -rf bin/ tree
