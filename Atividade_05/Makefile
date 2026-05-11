# ==========================================
# Variáveis de Compilação
# ==========================================
CC = mpicc
CFLAGS = -Wall -Wextra -std=c11
TARGET = calculo_media
SRC = calculo_media.c

# ==========================================
# Alvos (Targets)
# ==========================================

# 1. Instalação das dependências (Ubuntu/Debian)
# O '-' ignora erros se o repositório estiver parcialmente instável
install:
	-sudo apt-get update
	sudo apt-get install -y openmpi-bin libopenmpi-dev

# 2. Compilação
# Uso: 'make build' ou apenas 'make' (pois é o primeiro alvo após install)
build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# 3. Execução
# --allow-run-as-root: Necessário em alguns containers/codespaces
# --oversubscribe: Permite rodar mais processos do que núcleos de CPU disponíveis
run:
	mpirun --allow-run-as-root --oversubscribe -np 4 ./$(TARGET)

# 4. Limpeza
clean:
	rm -f $(TARGET)

# Alvo padrão quando você digita apenas 'make'
all: build