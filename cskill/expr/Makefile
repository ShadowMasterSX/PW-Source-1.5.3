OBJS	=  stubs.o objectgraph.o scanner.o exprparser.o analyzer.o executor.o main.o
#OBJS	= scanner.o exprparser.o analyzer.o main.o 
CC		= g++
DEFINES	= -D_DEBUG
CFLAGS	= -O2 -Wall -pthread
#CFLAGS	= -O0 -ggdb -Wall
INCLUDES += -I../skill -I../header -I../header/include

all: compiler
	
compiler: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	
%.o : %.cpp	
	$(CC) -c $(INCLUDES) $(CFLAGS)	$< -o $@
	
clean:
	rm -rf *.o
	rm -rf compiler
