SOURCES = $(wildcard *.cpp)
OBJECTS = ${SOURCES:.cpp=.o}
HEADERS  = $(wildcard *.h)
CXX = ccache clang++
CXX_ARGS = -g -Wall -std=c++17 


all: libtetradactyl-qt.so

libtetradactyl-qt.so: $(OBJECTS)
	$(CXX) $(CXX_ARGS) -shared -fpic $(OBJECTS) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXX_ARGS) -c -fpic $< -o $@  $(shell pkg-config --cflags Qt6Widgets Qt6Core) 

run-calculator:
	LD_PRELOAD=./libtetradactyl-qt.so  /home/psacawa/scratch/QT/examples/widgets/widgets/calculator/calculator
