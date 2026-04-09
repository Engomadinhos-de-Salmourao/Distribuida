#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "protocolo.h"
#include "jogo.h"

/* ---------- estruturas ---------- */

typedef struct {
    int  fd;
    char nome[MAX_NOME];
    char endereco[INET_ADDRSTRLEN];
    int  porta;
} Jogador;

typedef struct {
    Jogador j[NUM_JOGADORES];
    int     id;
} Partida;

/* ---------- fila de espera ---------- */

static pthread_mutex_t fila_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  fila_cond  = PTHREAD_COND_INITIALIZER;

static int   fila_fd[NUM_JOGADORES];
static int   fila_count = 0;
static int   partida_id = 0;

/* ---------- funções auxiliares ---------- */

/* Lê o nome do jogador após o servidor enviar NOME| */
static int pedir_nome(Jogador *j)
{
    char buf[MAX_MSG];

    if (enviar_msg(j->fd, "%s|\n", P_NOME_REQ) < 0)
        return -1;

    int n = receber_com_timeout(j->fd, buf, sizeof(buf), 30);
    if (n <= 0)
        return -1;

    /* Espera NOME|nome */
    if (strncmp(buf, P_NOME_REQ "|", strlen(P_NOME_REQ) + 1) != 0)
        return -1;

    const char *nome = buf + strlen(P_NOME_REQ) + 1;
    if (nome[0] == '\0')
        snprintf(j->nome, sizeof(j->nome), "Jogador");
    else
        snprintf(j->nome, sizeof(j->nome), "%s", nome);

    return 0;
}

static void executar_rodada(Partida *p, int rodada, int *pts)
{
    char letra = gerar_letra();

    printf("  [Rodada %d] Letra: %c\n", rodada, letra);

    /* Notifica ambos */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        enviar_msg(p->j[i].fd, "%s|%d|%c|%d\n",
                   P_RODADA, rodada, letra, TEMPO_RODADA);
    }

    char palavras[NUM_JOGADORES][MAX_PALAVRA];
    int  valida[NUM_JOGADORES];
    int  timeout_j[NUM_JOGADORES];

    /* Recebe respostas com timeout */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        char buf[MAX_MSG];
        palavras[i][0] = '\0';
        valida[i]      = 0;
        timeout_j[i]   = 0;

        int n = receber_com_timeout(p->j[i].fd, buf, sizeof(buf), TEMPO_RODADA + 2);
        if (n <= 0) {
            timeout_j[i] = 1;
            continue;
        }

        /* Pode ser PALAVRA|xxx ou TIMEOUT| */
        if (strncmp(buf, P_TIMEOUT_C "|", strlen(P_TIMEOUT_C) + 1) == 0) {
            timeout_j[i] = 1;
        } else if (strncmp(buf, P_PALAVRA "|", strlen(P_PALAVRA) + 1) == 0) {
            const char *p_word = buf + strlen(P_PALAVRA) + 1;
            snprintf(palavras[i], sizeof(palavras[i]), "%s", p_word);
        }
    }

    /* Valida palavras */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        if (!timeout_j[i] && palavras[i][0] != '\0')
            valida[i] = validar_palavra(palavras[i], letra);
    }

    /* Palavras iguais → ninguém pontua */
    if (valida[0] && valida[1]) {
        char w0[MAX_PALAVRA], w1[MAX_PALAVRA];
        /* comparação case insensitive */
        snprintf(w0, sizeof(w0), "%s", palavras[0]);
        snprintf(w1, sizeof(w1), "%s", palavras[1]);
        for (int k = 0; w0[k]; k++) w0[k] = (char)tolower((unsigned char)w0[k]);
        for (int k = 0; w1[k]; k++) w1[k] = (char)tolower((unsigned char)w1[k]);
        if (strcmp(w0, w1) == 0) {
            valida[0] = valida[1] = 0;
            /* Envia resultado de empate/repetição */
            for (int i = 0; i < NUM_JOGADORES; i++) {
                enviar_msg(p->j[i].fd,
                           "%s|Palavras iguais (\"%s\")! Nenhum ponto.\n",
                           P_RESULTADO, palavras[i]);
            }
            printf("  [Rodada %d] Palavras iguais: nenhum ponto\n", rodada);
            goto placar;
        }
    }

    /* Pontua e envia resultado individual */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        int op = 1 - i;
        if (valida[i]) {
            pts[i]++;
            if (palavras[op][0] != '\0')
                enviar_msg(p->j[i].fd,
                           "%s|Palavra \"%s\" valida! +1 ponto. [%s enviou: \"%s\"]\n",
                           P_RESULTADO, palavras[i], p->j[op].nome, palavras[op]);
            else
                enviar_msg(p->j[i].fd,
                           "%s|Palavra \"%s\" valida! +1 ponto. [%s nao enviou]\n",
                           P_RESULTADO, palavras[i], p->j[op].nome);
        } else {
            if (timeout_j[i])
                enviar_msg(p->j[i].fd,
                           "%s|Tempo esgotado! 0 pontos.\n", P_RESULTADO);
            else if (palavras[i][0] == '\0')
                enviar_msg(p->j[i].fd,
                           "%s|Sem palavra! 0 pontos.\n", P_RESULTADO);
            else
                enviar_msg(p->j[i].fd,
                           "%s|Palavra \"%s\" invalida! 0 pontos.\n",
                           P_RESULTADO, palavras[i]);
        }
    }

    printf("  [Rodada %d] %s=\"%s\"(%s) | %s=\"%s\"(%s) | Placar: %d x %d\n",
           rodada,
           p->j[0].nome, palavras[0], valida[0] ? "ok" : "x",
           p->j[1].nome, palavras[1], valida[1] ? "ok" : "x",
           pts[0], pts[1]);

placar:
    /* Envia placar atualizado */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        enviar_msg(p->j[i].fd, "%s|%s|%d|%s|%d\n",
                   P_PLACAR,
                   p->j[0].nome, pts[0],
                   p->j[1].nome, pts[1]);
    }
}

static void *thread_partida(void *arg)
{
    Partida *p = (Partida *)arg;

    /* Pede nomes */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        if (pedir_nome(&p->j[i]) < 0) {
            snprintf(p->j[i].nome, sizeof(p->j[i].nome), "Jogador%d", i + 1);
        }
    }

    printf("[Partida #%d] Jogadores: %s vs %s\n",
           p->id, p->j[0].nome, p->j[1].nome);

    /* Anuncia início */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        enviar_msg(p->j[i].fd,
                   "%s|Batalha de Palavras! %s vs %s — %d rodadas. Boa sorte!\n",
                   P_MSG, p->j[0].nome, p->j[1].nome, NUM_RODADAS);
    }

    int pts[NUM_JOGADORES] = {0, 0};

    for (int r = 1; r <= NUM_RODADAS; r++) {
        executar_rodada(p, r, pts);
    }

    /* Resultado final */
    char resultado[MAX_MSG];
    if (pts[0] > pts[1])
        snprintf(resultado, sizeof(resultado),
                 "%s venceu! Placar final: %s %d x %d %s",
                 p->j[0].nome, p->j[0].nome, pts[0], pts[1], p->j[1].nome);
    else if (pts[1] > pts[0])
        snprintf(resultado, sizeof(resultado),
                 "%s venceu! Placar final: %s %d x %d %s",
                 p->j[1].nome, p->j[0].nome, pts[0], pts[1], p->j[1].nome);
    else
        snprintf(resultado, sizeof(resultado),
                 "Empate! Placar final: %s %d x %d %s",
                 p->j[0].nome, pts[0], pts[1], p->j[1].nome);

    for (int i = 0; i < NUM_JOGADORES; i++) {
        enviar_msg(p->j[i].fd, "%s|%s\n", P_FIM, resultado);
        close(p->j[i].fd);
    }

    printf("[Partida #%d] %s\n", p->id, resultado);

    free(p);
    return NULL;
}

/* Thread que aguarda par e dispara partida */
static void *thread_espera(void *arg)
{
    int fd = *((int *)arg);
    free(arg);

    /* Informa que está aguardando */
    enviar_msg(fd, "%s|Conectado! Aguardando outro jogador para iniciar...\n", P_AGUARDE);

    pthread_mutex_lock(&fila_mutex);

    /* Entra na fila */
    fila_fd[fila_count]  = fd;
    /* endereço já preenchido antes de criar a thread — copiado via arg seria
       trabalhoso; usamos placeholder já que logamos antes */
    fila_count++;

    if (fila_count < NUM_JOGADORES) {
        /* Aguarda segundo jogador */
        pthread_cond_wait(&fila_cond, &fila_mutex);
        pthread_mutex_unlock(&fila_mutex);
        return NULL; /* primeira thread sai; segunda monta a partida */
    }

    /* Somos o segundo jogador — monta partida */
    Partida *p = (Partida *)malloc(sizeof(Partida));
    if (!p) {
        pthread_mutex_unlock(&fila_mutex);
        return NULL;
    }

    for (int i = 0; i < NUM_JOGADORES; i++) {
        p->j[i].fd = fila_fd[i];
    }
    p->id = ++partida_id;
    fila_count = 0;

    pthread_cond_signal(&fila_cond); /* acorda a primeira thread para ela sair */
    pthread_mutex_unlock(&fila_mutex);

    pthread_t tid;
    pthread_create(&tid, NULL, thread_partida, p);
    pthread_detach(tid);

    return NULL;
}

/* ---------- main ---------- */

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    srand((unsigned)time(NULL));

    int porta = PORTA_PADRAO;
    if (argc >= 2)
        porta = atoi(argv[1]);

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)porta);

    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(srv); return 1;
    }
    if (listen(srv, 10) < 0) {
        perror("listen"); close(srv); return 1;
    }

    printf("BATALHA DE PALAVRAS -- Servidor\n");
    printf("Porta: %d\n", porta);
    printf("Aguardando jogadores (pares de 2)...\n\n");

    int conn_count = 0;

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int cfd = accept(srv, (struct sockaddr *)&cli_addr, &cli_len);
        if (cfd < 0) { perror("accept"); continue; }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        int cli_porta = ntohs(cli_addr.sin_port);

        printf("[+] Jogador conectou: %s:%d (fd=%d)\n", ip, cli_porta, cfd);
        conn_count++;

        pthread_mutex_lock(&fila_mutex);
        int na_fila = fila_count + 1; /* +1 pois ainda não entrou */
        pthread_mutex_unlock(&fila_mutex);

        if (na_fila < NUM_JOGADORES)
            printf("[*] Aguardando mais %d jogador(es)...\n",
                   NUM_JOGADORES - na_fila);

        int *fdp = (int *)malloc(sizeof(int));
        *fdp = cfd;

        pthread_t tid;
        pthread_create(&tid, NULL, thread_espera, fdp);
        pthread_detach(tid);
    }

    close(srv);
    return 0;
}
