# CXX = g++
CXX = clang++

CXXFLAGS = -fPIC -O3 $(shell mapnik-config --cflags)

LIBS = -lyajl $(shell mapnik-config --libs --ldflags) -licuuc

SRC = $(wildcard *.cpp)

OBJ = $(SRC:.cpp=.o)

BIN = geojson.input

all : $(SRC) $(BIN)

$(BIN) : $(OBJ)
	$(CXX) -shared $(OBJ) $(LIBS) -o $@  

.cpp.o :
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY : clean

clean: 
	rm -f $(OBJ)
	rm -f $(BIN)

deploy:
	cp geojson.input $(shell mapnik-config --input-plugins)

test:
	mapnik-render.js test.xml test.png

do: clean all deploy test
