# ==========================================
# Variáveis de Compilação
# ==========================================
CC = mpicc
CFLAGS = -Wall -Wextra -std=c11
TARGET = transformacao_paralela
SRC = transformacao_paralela.c

# ==========================================
# Alvos (Targets)
# ==========================================

# 1. Instalação das dependências (Caso o Codespace seja novo)
install:
	-sudo apt-get update
	sudo apt-get install -y openmpi-bin libopenmpi-dev

# 2. Compilação
build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# 3. Execução (Configurado para 5 processos conforme o enunciado)
# --allow-run-as-root: Para o ambiente de container
# --oversubscribe: Para garantir a execução mesmo com poucos núcleos de CPU
run:
	mpirun --allow-run-as-root --oversubscribe -np 5 ./$(TARGET)

# 4. Limpeza
clean:
	rm -f $(TARGET)

# Alvo padrão
all: build