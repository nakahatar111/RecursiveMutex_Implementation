all: r_mutex
r_mutex: r_mutex.o
	g++ -pthread r_mutex.o -o run
r_mutex.o: r_mutex.cpp
	g++ -c r_mutex.cpp