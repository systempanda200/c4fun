#
# Variables:
#
CC = gcc
BIN = numa-expe
SRC = numa-expe.c
OBJ = numa-expe.o


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
# Construction du programme:
#
all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)
	$(CC) -o $(BIN) $(OBJ) $(LDFLAGS)


#
# Nettoyage:
#
clean:
	rm -f *.o *~ core $(BIN)
