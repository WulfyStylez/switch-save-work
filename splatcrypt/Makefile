OBJS = main.o utils.o
LIBS = -static-libstdc++ -lcrypto
CXXFLAGS = -I.
CFLAGS = -std=gnu11 -Wall -Wno-unused-variable -Wno-unused-but-set-variable -O2 -I.
OUTPUT = splatcrypt
CC = gcc

main: $(OBJS)
	$(CC) -o $(OUTPUT) $(OBJS) $(LIBS)

clean:
	rm -rf $(OUTPUT) $(OBJS)
