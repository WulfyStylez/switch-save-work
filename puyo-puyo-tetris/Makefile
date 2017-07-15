OBJS = main.o utils.o
LIBS = -static-libstdc++ -static
CXXFLAGS = -I. 
CFLAGS = -std=gnu11 -Wall -Wno-unused-variable -Wno-unused-but-set-variable -O2 -I.
OUTPUT = puyocrc
CC = gcc

main: $(OBJS) $(ALGO_OBJS)
	g++ -o $(OUTPUT) $(LIBS) $(OBJS)

clean:
	rm -rf $(OUTPUT) $(OBJS)
