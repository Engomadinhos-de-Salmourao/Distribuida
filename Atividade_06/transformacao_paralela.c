/*
    INTEGRANTES:                        RAs:
    Edmilson Li Quansang                10425514
    João Paulo B. Massabki              10425593
    Pietro Caffettani                   10425628

    Ps: rodamos o código em um codespace do Github com 4 cores. A lógica é exatamente a mesma,
    mas adaptamos o mpirun para funcionar (na prática estamos forçando a criação de 4 processos em um único core,
    já que os cores do codespace são virtuais e o MPI identifica isso e barra.)

    Se for baixar o código na sua máquina, pode rodar com o 
    mpirun -np 4 ./soma_quadrados
    indicado no readme do enunciado da atividade
    ou se for criar um codespace no nosso repositório, usar os comandos:
    cd Atividade_04             // Entrar no diretório correto
    make install                // Baixar o MPI
    make build                  // compilar (opcional)
    make run                    // executar
    
    em make run temos --allow-run-as-root e --oversubscribe pra permitir rodar no codespace.
*/

#include <mpi.h>     // Inclui a biblioteca do MPI, necessária para funções de comunicação paralela
#include <stdio.h>   // Inclui a biblioteca padrão de entrada e saída, para usar printf
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N (100)

// Protótipos
int quadrado(int n);

int main (int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    // Para usar o Scatter, o root não pode *criar* o vetor. Interpretamos "criar" como "inicializar os valores"
    int vetor[N];
    int labor = N / size;


    // Inicializando vetor no processo rank 0
    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            vetor[i] = i + 1;
        }
    }  

    // Criando sub_vetores de cada rank
    int sub_vetor[labor];
    int sub_vetor_T[labor];

    // Dividindo o vetor em sub_vetores
    MPI_Scatter(vetor, labor, MPI_INT, sub_vetor, labor, MPI_INT, 0, MPI_COMM_WORLD);

    printf("[Processo %d] Vetor original: ", rank);
    for (int i = 0; i < labor; i++) {
        int n = sub_vetor[i];
        printf("%d ", n);
    }
    printf("\n");

    printf("[Processo %d] Vetor transformado: ", rank);
    for (int i = 0; i < labor; i++) {
        int n = sub_vetor[i] * sub_vetor[i];
        sub_vetor_T[i] = n;
        printf("%d ", n);
    }
    printf("\n");

    // Recebendo em root
    MPI_Gather(sub_vetor_T, labor, MPI_INT, vetor, labor, MPI_INT, 0, MPI_COMM_WORLD);

    // Output final, com valor encontrado + valor esperado
    if (rank == 0) {
        printf("\n\n[Processo %d] Vetor original:\n", rank);
        for (int i = 0; i < N; i++) {
            printf("%5d ", i+1);
            if ((i + 1) % 10 == 0) printf("\n");
        }

        printf("\n\n[Processo %d] Vetor transformado:\n", rank);
        for (int i = 0; i < N; i++) {
            int n = vetor[i];
            printf("%5d ", n);
            if ((i + 1) % 10 == 0) printf("\n");
        }
    }

    // Fechando :D
    MPI_Finalize();
    return 0;
}
