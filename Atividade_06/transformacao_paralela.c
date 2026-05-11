/*
    INTEGRANTES:                        RAs:
    Edmilson Li Quansang                10425514
    João Paulo B. Massabki              10425593
    Pietro Caffettani                   10425628

    Instruções de Execução:
    
    1. Localmente (Máquina própria):
       Pode rodar diretamente conforme o enunciado:
       mpirun -np 5 ./transformacao_paralela
    
    2. No GitHub Codespaces:
       Como o ambiente possui limitação de 4 cores virtuais, adaptamos o comando para 
       funcionar via Makefile. Embora o enunciado peça 5 processos, o MPI identifica 
       a falta de núcleos físicos e barraria a execução. Usamos as flags de suporte 
       para contornar essa limitação.
       
       Comandos:
       cd Atividade_06            // Entrar no diretório correto
       make install               // Instalar as dependências do MPI (apenas uma vez)
       make build                 // Compilar o programa transformacao_paralela.c
       make run                   // Executar com 5 processos
       
    Nota: O 'make run' utiliza --allow-run-as-root e --oversubscribe. 
    A flag --oversubscribe é vital aqui: ela força o MPI a criar os 5 processos 
    compartilhando os mesmos núcleos virtuais, ignorando a trava de hardware do ambiente.
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
