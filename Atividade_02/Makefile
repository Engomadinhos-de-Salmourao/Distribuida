CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11
LDFLAGS_SRV = -lpthread

all: servidor cliente

servidor: servidor.c jogo.c jogo.h protocolo.h
	$(CC) $(CFLAGS) -o servidor servidor.c jogo.c $(LDFLAGS_SRV)

cliente: cliente.c jogo.c jogo.h protocolo.h
	$(CC) $(CFLAGS) -o cliente cliente.c jogo.c

clean:
	rm -f servidor cliente

.PHONY: all clean
