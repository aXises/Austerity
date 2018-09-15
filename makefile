OPTS=-std=gnu99 --pedantic -Wall -Werror -g
TARGETS = austerity shenzi

all: $(TARGETS)

austerity: austerity.c shared.o
	gcc $(OPTS) austerity.c shared.o -o austerity

shenzi: shenzi.c shared.o player.o
	gcc $(OPTS) shenzi.c shared.o player.o -o shenzi

shared.o: shared.c shared.h
	gcc $(OPTS) -c shared.c -o shared.o

player.o: player.c player.h
	gcc $(OPTS) -c player.c -o player.o
	
clean:
	rm -f *.o austerity
