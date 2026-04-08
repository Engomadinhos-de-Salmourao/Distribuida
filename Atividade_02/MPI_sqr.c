#include <mpi.h>     // Inclui a biblioteca do MPI, necessária para funções de comunicação paralela
#include <stdio.h>   // Inclui a biblioteca padrão de entrada e saída, para usar printf
#include <string.h>

#define N (40)

int main (int argc, char **argv) {
    
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int vetor[N]; // Para usar o Scatter, o root não pode *criar* o vetor. Interpretamos "criar" como "inicializar os valores"
    int labor = N / size;

    // Inicializando vetor no processo rank 0
    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            vetor[i] = i + 1;
        }
    }  

    // Criando sub_vetores de cada rank
    int sub_vetor[labor];
    // Dividindo o vetor em sub_vetores
    MPI_Scatter(vetor, labor, MPI_INT, sub_vetor, labor, MPI_INT, 0, MPI_COMM_WORLD);

    int sub_total = 0;

    for (int i = 0; i < labor; i++) {
        int n = sub_vetor[i];
        sub_total += ( n * (n + 1) * ((2 * n) + 1) ) / 6;
    }

    int total = 0;

    MPI_Reduce(&sub_total, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("A soma total do vetor e: %d\n", total);
        // Para conferir: a soma de 1 a 40 é (40 * 41) / 2 = 820
    }

    MPI_Finalize();
    return 0;
    // Encerra o programa com sucesso
}