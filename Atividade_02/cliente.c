#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <time.h>

#include "protocolo.h"
#include "jogo.h"

/* Lê uma linha do stdin com timeout usando select().
 * Retorna bytes lidos (sem \n), 0 em timeout, -1 em erro/EOF. */
static int ler_stdin_timeout(char *buf, int bufsz, int timeout_seg)
{
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    tv.tv_sec  = timeout_seg;
    tv.tv_usec = 0;

    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0)
        return 0; /* timeout */

    if (!fgets(buf, bufsz, stdin))
        return -1; /* EOF */

    int len = (int)strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';

    return len;
}

/* Recebe uma linha do servidor (bloqueante com timeout curto) */
static int receber_linha(int fd, char *buf, int bufsz)
{
    return receber_com_timeout(fd, buf, bufsz, 60);
}

/* Extrai campo N (0-based) de uma mensagem separada por '|' */
static const char *campo(const char *msg, int n)
{
    static char tmp[MAX_MSG];
    snprintf(tmp, sizeof(tmp), "%s", msg);

    char *tok = strtok(tmp, "|");
    for (int i = 0; i < n && tok; i++)
        tok = strtok(NULL, "|");

    return tok ? tok : "";
}

/* Processa RODADA e lida com input do usuário */
static void processar_rodada(int fd, const char *msg)
{
    /* RODADA|num|letra|tempo */
    int  num   = atoi(campo(msg, 1));
    char letra = campo(msg, 2)[0];
    int  tempo = atoi(campo(msg, 3));

    printf("\n--- RODADA %d de %d ---\n", num, NUM_RODADAS);
    printf("Letra: [%c]   Tempo: %d segundos   Minimo: %d caracteres\n",
           letra, tempo, MIN_CHARS);
    printf("Sua palavra: ");
    fflush(stdout);

    char buf[MAX_PALAVRA];
    int n = ler_stdin_timeout(buf, sizeof(buf), tempo);

    if (n <= 0) {
        printf("\n[Tempo esgotado!]\n");
        enviar_msg(fd, "%s|\n", P_TIMEOUT_C);
        return;
    }

    /* Remove espaços das bordas */
    char *p = buf;
    while (*p == ' ') p++;
    int len = (int)strlen(p);
    while (len > 0 && p[len - 1] == ' ') p[--len] = '\0';

    printf("Enviando: \"%s\" — aguardando resultado...\n", p);
    enviar_msg(fd, "%s|%s\n", P_PALAVRA, p);
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    const char *host  = "127.0.0.1";
    int         porta = PORTA_PADRAO;

    if (argc >= 2) host  = argv[1];
    if (argc >= 3) porta = atoi(argv[2]);

    printf("BATALHA DE PALAVRAS -- Cliente\n");
    printf("Conectando a %s:%d...\n", host, porta);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons((uint16_t)porta);
    if (inet_pton(AF_INET, host, &srv.sin_addr) <= 0) {
        fprintf(stderr, "Endereco invalido: %s\n", host);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("connect"); return 1;
    }

    printf("Conectado!\n\n");

    char buf[MAX_MSG];

    /* Loop principal de mensagens */
    while (1) {
        int n = receber_linha(fd, buf, sizeof(buf));
        if (n <= 0) {
            printf("\n[Conexao encerrada pelo servidor.]\n");
            break;
        }

        /* Identifica o tipo da mensagem pelo prefixo */
        if (strncmp(buf, P_NOME_REQ "|", strlen(P_NOME_REQ) + 1) == 0) {
            /* Solicita nome */
            printf("Digite seu nome: ");
            fflush(stdout);
            char nome[MAX_NOME];
            if (!fgets(nome, sizeof(nome), stdin)) break;
            int l = (int)strlen(nome);
            while (l > 0 && (nome[l-1] == '\n' || nome[l-1] == '\r'))
                nome[--l] = '\0';
            if (nome[0] == '\0')
                snprintf(nome, sizeof(nome), "Jogador");
            printf("Bem-vindo, %s!\n", nome);
            enviar_msg(fd, "%s|%s\n", P_NOME_REQ, nome);

        } else if (strncmp(buf, P_AGUARDE "|", strlen(P_AGUARDE) + 1) == 0) {
            const char *texto = buf + strlen(P_AGUARDE) + 1;
            printf("%s\n", texto);

        } else if (strncmp(buf, P_MSG "|", strlen(P_MSG) + 1) == 0) {
            const char *texto = buf + strlen(P_MSG) + 1;
            printf("\n%s\n", texto);

        } else if (strncmp(buf, P_RODADA "|", strlen(P_RODADA) + 1) == 0) {
            processar_rodada(fd, buf);

        } else if (strncmp(buf, P_RESULTADO "|", strlen(P_RESULTADO) + 1) == 0) {
            const char *texto = buf + strlen(P_RESULTADO) + 1;
            printf("%s\n", texto);

        } else if (strncmp(buf, P_PLACAR "|", strlen(P_PLACAR) + 1) == 0) {
            /* PLACAR|nome1|pts1|nome2|pts2 */
            /* campo() usa strtok estático — fazemos a extração manual */
            char tmp[MAX_MSG];
            snprintf(tmp, sizeof(tmp), "%s", buf + strlen(P_PLACAR) + 1);
            char *nome1 = strtok(tmp,  "|");
            char *pts1s = strtok(NULL, "|");
            char *nome2 = strtok(NULL, "|");
            char *pts2s = strtok(NULL, "|");
            if (nome1 && pts1s && nome2 && pts2s)
                printf("PLACAR: %s %s x %s %s\n", nome1, pts1s, pts2s, nome2);

        } else if (strncmp(buf, P_FIM "|", strlen(P_FIM) + 1) == 0) {
            const char *texto = buf + strlen(P_FIM) + 1;
            printf("\n=== FIM DE JOGO ===\n%s\n\n", texto);
            break;

        } else {
            /* Mensagem desconhecida — exibe crua */
            printf("[?] %s\n", buf);
        }
    }

    close(fd);
    return 0;
}
