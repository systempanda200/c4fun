#
# Variables
#
EXECUTABLE_NAME = fifo
ERROR_CFLAGS = --std=gnu99 -Wall

#
# Construction du programme:
#

default: fifo.c fifo.h main.c
	gcc $(ERROR_CFLAGS) -c fifo.c
	gcc $(ERROR_CFLAGS) -c main.c
	gcc -o $(EXECUTABLE_NAME) main.o fifo.o -lpthread


#
# Nettoyage:
#
clean:
	rm -f *.s *.o *~ $(EXECUTABLE_NAME)
