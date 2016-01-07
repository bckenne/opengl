CPP=g++
CFLAGS= -L/usr/lib6 -O2 -lX11 -std=gnu++0x -lGL -lGLU -lGLEW -lglut -lm -lXmu

BINS = main
all: $(BINS)

main: main.cpp
	$(CPP) -o main main.cpp $(CFLAGS)
	
clean:
	rm $(BINS)
