#
# Flags d'erreurs:
#
ERROR_FLAGS = -std=gnu99 -Wall -Werror -O0 -g

#
# Flags pour le compilateur:
#
CFLAGS = $(ERROR_FLAGS) -D_GNU_SOURCE

#
# Flags pour l'editeur de liens:
#
LDFLAGS = $(ERROR_FLAGS) -lnuma

#
# Construction des programmes:
#
all: clean pebs_bench

pebs_bench: pebs_bench_ui pebs_bench.c
	gcc $(CFLAGS) -c pebs_bench.c -I../mem_alloc
	gcc -o pebs_bench pebs_bench.o pebs_bench_ui.o ../mem_alloc/mem_alloc.o $(LDFLAGS)

pebs_bench_ui: pebs_bench_ui.c
	gcc $(CFLAGS) -c pebs_bench_ui.c

#
# Nettoyage:
#
clean:
	rm -f *.o cachegrind.out.* perf.data* *~ core mem_bound mem_bound_infinite out
