
CC = gcc
CFLAGS = -c
SOURCES = sim.c cli.c list.c node_list.c
OBJECTS = $(SOURCES:.c=.o)
EXE = sim

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ -lpthread
	cp sim ../test/test_sim

%.o:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o
	rm -rf ../test/test_sim/sim
