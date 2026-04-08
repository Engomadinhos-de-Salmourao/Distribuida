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
#include <string.h>

#define N (40)

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

    // Dividindo o vetor em sub_vetores
    MPI_Scatter(vetor, labor, MPI_INT, sub_vetor, labor, MPI_INT, 0, MPI_COMM_WORLD);

    // Começando cálculo de quadrado parcial
    int sub_total = 0;

    printf("Processo %d recebeu: ", rank);
    for (int i = 0; i < labor; i++) {
        // Durante o cálculo da soma de quadrados parcial, realiza simultanemaente o output do código
            // (Quais valores cada processo recebeu)
        int n = sub_vetor[i];
        printf("%d ", n);
        sub_total += (n * n);
    }

    // Output do código, valor da soma de quadrados parcial
    printf("\nProcesso %d: Soma local dos quadrados = %d\n", rank, sub_total);

    // Preparando pra receber em root
    int total = 0;
    // Recebendo em root
    MPI_Reduce(&sub_total, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Output final, com valor encontrado + valor esperado
    if (rank == 0) {
        printf("\nA soma Paralela dos quadrados foi: %d\n", total);
        // Para conferir: a soma de 1 a 40 é (40 * 41) / 2 = 820
        int s_esp = (N * (N + 1) * ((2 * N) + 1)) / 6;
        printf("A soma sequencial esperada: %d\n", s_esp);
    }

    // Fechando :D
    MPI_Finalize();
    return 0;
}