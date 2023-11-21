OBJS=raid6.o main.o logger.o galois.o
CC=g++
CFLAGS=-c -std=c++17 -O3

main:$(OBJS)
	$(CC) $^ -o main -O3
%.o:source/%.cpp
	$(CC) $^ $(CFLAGS)

clean:
	$(RM) *.o main