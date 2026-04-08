# Variáveis
CC = mpicc
CFLAGS = -Wall

# Instala as dependências no Ubuntu/Debian (Codespaces)
install:
	sudo apt-get update && sudo apt-get install -y openmpi-bin libopenmpi-dev

# Compila o programa
build:
	$(CC) $(CFLAGS) soma_quadrados.c -o soma_quadrados

# Atalho para rodar com 4 processos
run:
	mpirun --allow-run-as-root --oversubscribe -np 4 ./programa