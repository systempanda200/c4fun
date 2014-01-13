ERROR_FLAGS = -std=gnu99 -Wall -Werror
CFLAGS = $(ERROR_FLAGS) -D_REENTRANT -DLinux -D_GNU_SOURCE

all: example_dynamic

dynamic: numemp.c numemp.h
	gcc $(CFLAGS) -fpic -c numemp.c
	gcc -shared -o libnumemp.so numemp.o -lnuma -lpfm

example_dynamic: dynamic example.c
	gcc $(CFLAGS) -c example.c
	gcc $(CFLAGS) -o example example.o -L. -lnumemp

clean:
	rm -f *.o *.a *.so* example
