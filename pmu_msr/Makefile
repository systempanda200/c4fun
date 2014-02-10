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
all: clean pmu_msr

pmu_msr: pmu_msr.c
	gcc $(CFLAGS) -c pmu_msr.c -I../mem_alloc
	gcc -o pmu_msr pmu_msr.o ../mem_alloc/mem_alloc.o $(LDFLAGS)

#
# Nettoyage:
#
clean:
	rm -f *.o cachegrind.out.* perf.data* *~ core mem_bound mem_bound_infinite out pmu_msr
