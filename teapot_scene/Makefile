CPP=g++
CFLAGS= -g -L/usr/lib6 -lX11 -std=gnu++0x -lGL -lGLU -lGLEW -lglut -lm -lXmu

BINS = run
all: $(BINS)

run: teapot.cpp ppmRead.cpp fileParser.cpp
	$(CPP) -o run teapot.cpp ppmRead.cpp fileParser.cpp $(CFLAGS)
	
clean:
	rm $(BINS)
