OBJS := stubs.o objectgraph.o scanner.o exprparser.o analyzer.o executor.o main.o
CC := g++
DEFINES := -D_DEBUG
CFLAGS := -O2 -Wall -pthread
INCLUDES := -I../skill -I../header -I../header/include

.PHONY: all clean

all: compiler

compiler: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CC) -c $(INCLUDES) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o compiler
