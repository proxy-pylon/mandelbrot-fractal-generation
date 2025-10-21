CXX = g++
MPICXX = mpicxx
CXXFLAGS = -O3 -std=c++11 -Wall
LDFLAGS = -lm

all: mandelbrot_serial mandelbrot_mpi

mandelbrot_serial: mandelbrot_serial.cpp
	$(CXX) $(CXXFLAGS) -o mandelbrot_serial mandelbrot_serial.cpp $(LDFLAGS)

mandelbrot_mpi: mandelbrot_mpi.cpp
	$(MPICXX) $(CXXFLAGS) -o mandelbrot_mpi mandelbrot_mpi.cpp $(LDFLAGS)

clean:
	rm -f mandelbrot_serial mandelbrot_mpi
	rm -f frame_*.ppm
	rm -f *.o

.PHONY: all clean
