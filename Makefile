#
# Flags d'erreurs:
#
ERROR_CFLAGS = -Wall -W -pedantic

#
# Flags pour le compilateur:
#
CFLAGS = $(ERROR_FLAGS) -D_REENTRANT -DLinux -D_GNU_SOURCE

#
# Flags pour l'editeur de liens:
#
LDFLAGS = $(ERROR_FLAGS) -lpthread -lnuma

#
# Construction des programmes:
#
all: clean numa-expe cpuid perf_open

numa-expe: numa-expe.c
	$(CC) $(CFLAGS) -c numa-expe.c
	$(CC) -o numa-expe numa-expe.o $(LDFLAGS)

cpuid: cpuid.c
	$(CC) $(CFLAGS) -c cpuid.c
	$(CC) -o cpuid cpuid.o $(LDFLAGS)

perf_open: perf_open.c
	$(CC) $(CFLAGS) -c perf_open.c
	$(CC) -o perf_open perf_open.o $(LDFLAGS)


#
# Nettoyage:
#
clean:
	rm -f *.o *~ core numa-expe cpuid perf_open
