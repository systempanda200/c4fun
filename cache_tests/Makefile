ERROR_FLAGS = -std=gnu99 -Wall -Werror
CFLAGS = $(ERROR_FLAGS) -g -O0 -I../mem_alloc

cache_tests: cache_tests.c
	gcc $(CFLAGS) -c cache_tests.c
	gcc -o cache_tests cache_tests.o ../mem_alloc/mem_alloc.o
clean:
	rm -f *.o cache_tests
