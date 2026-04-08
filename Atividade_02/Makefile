# Variáveis
CC = mpicc
CFLAGS = -Wall

# Instala as dependências no Ubuntu/Debian (Codespaces)
install:
	sudo apt-get update && sudo apt-get install -y openmpi-bin libopenmpi-dev

# Compila o programa
build:
	$(CC) $(CFLAGS) MPI_sqr.c -o programa

# Atalho para rodar com 4 processos
run:
	mpirun --allow-run-as-root -np 4 ./programa