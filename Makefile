CFLAGS=-c -Wall $(EXTRA_CFLAGS)
EXECUTABLE=server.bin
CC=gcc

SOURCES=main.c common.c get.c post.c
OBJECTS=$(SOURCES:.c=.o)
LDFLAGS=-lpthread -lm

all: CFLAGS += -O3
all: $(EXECUTABLE) clean_objects

debug: CFLAGS += -g3 -gdwarf -DMEASURE_TIME
debug: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c common.h
	$(CC) $(CFLAGS) $<

get.o: get.c get.h common.h
	$(CC) $(CFLAGS) $<

post.o: post.c post.h common.h
	$(CC) $(CFLAGS) $<

.PHONY: run clean

run:
	./$(EXECUTABLE)
	
clean_objects:
	rm *.o

clean:
	rm *.o
	rm $(EXECUTABLE)
