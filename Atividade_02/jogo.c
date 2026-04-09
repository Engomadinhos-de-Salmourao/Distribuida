# include <stdio.h>
# include <stdlib.h>
# include <time.h>

char letra_inicial();
int validar_ponto(char* palavra, int size, char initial);
int palavras_diferentes(char* palavra1, char* palavra2, int size1, int size2);

int main() {
    printf("\n%c\n\n", letra_inicial());
}

char letra_inicial() {
    char letras[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    srand(time(NULL)); // Alimenta o gerador com a hora atual
    int numero = rand() % 26; // Gera de 0 a 99
    return letras[numero];
}

int palavras_diferentes(char* palavra1, char* palavra2, int size1, int size2) {
    if (size1 != size2) { return 1; }

    for (int i = 0; i < size1; i++) {
        if (palavra1[i] != palavra2[i]) {
            return 1;
        }
    }
    
    return 0;
}

int validar_ponto(char* palavra, int size, char initial) {
    if (size < 5) { return 0; }
    if (palavra[0] != initial) { return 0; }

    for (int i = 0; i < size; i++) {
        if (!isalpha(palavra[i])) {
            return 0;
        }
    }

    return 1;
}