/*
    INTEGRANTES:                        RAs:
    Edmilson Li Quansang                10425514
    João Paulo B. Massabki              10425593
    Pietro Caffettani                   10425628

    Ps: rodamos o código em um codespace do Github com 4 cores. A lógica é exatamente a mesma,
    mas adaptamos o mpirun para funcionar (na prática estamos forçando a criação de 4 processos em um único core,
    já que os cores do codespace são virtuais e o MPI identifica isso e barra.)

    Instruções de Execução:
    1. Localmente (Máquina própria):
       Pode rodar diretamente com o comando:
       mpirun -np 4 ./calculo_media
    
    2. No GitHub Codespaces:
       Para garantir que o ambiente e as permissões estejam corretos, utilize o Makefile:
       
       cd Atividade_05            // Entrar no diretório correto
       make install               // Instalar as dependências do MPI (necessário apenas uma vez)
       make build                 // Compilar o programa calculo_media.c
       make run                   // Executar com 4 processos
       
    Nota: O comando 'make run' já inclui as flags --allow-run-as-root e --oversubscribe,
    necessárias para a execução estável dentro do ambiente de containers do Codespace.
*/

#include <mpi.h>     // Inclui a biblioteca do MPI, necessária para funções de comunicação paralela
#include <stdio.h>   // Inclui a biblioteca padrão de entrada e saída, para usar printf
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N (1000)

int main (int argc, char **argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    float vetor_local[N];
    float soma_local = 0;
    
    srand(time(NULL) + rank); // Para que os processos tenham seeds diferentes
    
    for(int i = 0; i < N; i++) {
        vetor_local[i] = rand() / (float)RAND_MAX;
        soma_local += vetor_local[i];
    }
    float media_local = soma_local / N;

   printf("[Processo %d] Soma local: %f || Média local: %f\n", rank, soma_local, media_local);

    // Preparando pra receber em root
    float soma_total = 0;
    float media_total = 0;
    // Recebendo em root
    MPI_Reduce(&soma_local, &soma_total, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Output final, com valores encontrados
    if (rank == 0) {
        media_total = soma_total / (N * size);
        
        printf("\n[Soma Global] %f\n[Média Global] %f\n\n", soma_total, media_total);

    }

    // Fechando :D
    MPI_Finalize();
    return 0;
}