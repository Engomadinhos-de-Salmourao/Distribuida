# ============================================================
# Makefile — Chat Multicliente (Parte 2)
#
# Uso:
#   make          → compila servidor e cliente
#   make clean    → remove binários
#   make help     → mostra esta ajuda
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11

SERVIDOR = server
CLIENTE  = client

ALL = $(SERVIDOR) $(CLIENTE)

.PHONY: all clean help

all: $(ALL)
	@echo ""
	@echo "✅ Chat compilado com sucesso!"
	@echo ""
	@echo "Para testar:"
	@echo "  Terminal 1:  ./server"
	@echo "  Terminal 2:  ./client"
	@echo "  Terminal 3:  ./client"

$(SERVIDOR): server.c
	$(CC) $(CFLAGS) -o $@ $<

$(CLIENTE): client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(ALL)
	@echo "🧹 Binários removidos."

help:
	@echo "Alvos disponíveis:"
	@echo "  make          Compila servidor e cliente do chat"
	@echo "  make clean    Remove binários"
	@echo "  make help     Mostra esta ajuda"