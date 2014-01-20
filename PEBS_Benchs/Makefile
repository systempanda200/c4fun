#
# Flags d'erreurs:
#
ERROR_FLAGS = -std=gnu99 -Wall -Werror -O0

#
# Flags pour le compilateur:
#
CFLAGS = $(ERROR_FLAGS)

#
# Flags pour l'editeur de liens:
#
LDFLAGS = $(ERROR_FLAGS) -lnuma

#
# Construction des programmes:
#
all: clean mem_bound mem_bound_infinite

mem_bound: mem_bound.c
	gcc $(CFLAGS) -c mem_bound.c
	gcc -o mem_bound mem_bound.o $(LDFLAGS)

mem_bound_infinite: mem_bound_infinite.c
	gcc $(CFLAGS) -c mem_bound_infinite.c
	gcc -o mem_bound_infinite mem_bound_infinite.o $(LDFLAGS)

#
# Nettoyage:
#
clean:
	rm -f *.o cachegrind.out.* perf.data* *~ core mem_bound mem_bound_infinite
