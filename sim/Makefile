
CC = gcc
CFLAGS = -c
SOURCES = sim.c cli.c list.c node_list.c
OBJECTS = $(SOURCES:.c=.o)
EXE = sim.exe

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) -lpthread $(OBJECTS) -o $@
	cp sim.exe ../test/test_sim

%.o:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
