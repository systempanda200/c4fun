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
all: clean perf_event_open

perf_event_open: perf_event_open.c
	$(CC) $(CFLAGS) -c perf_event_open.c
	$(CC) -o perf_event_open perf_event_open.o $(LDFLAGS)

#
# Nettoyage:
#
clean:
	rm -f *.o *~ core perf_event_open
