/*
 * chat_servidor.c — Servidor de Chat Multicliente
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

/* Definições de constantes para facilitar a manutenção */
#define PORTA         7070
#define MAX_CLIENTES  2
#define BUFFER_SIZE   2048  /* Tamanho máximo da mensagem */
#define NOME_SIZE     32    /* Tamanho máximo do nome do usuário */

/* * Estrutura Cliente: Como o servidor não tem interface gráfica, 
 * precisamos guardar os metadados de quem está conectado para 
 * conseguir identificar quem enviou o quê.
 */
typedef struct {
    int   fd;                /* File Descriptor: o "ID" do canal de comunicação no Linux */
    char  nome[NOME_SIZE];   
    char  ip[INET_ADDRSTRLEN]; 
    int   porta;             
} Cliente;

/* * Variáveis Globais Estáticas: 
 * O array 'clientes' é o nosso "banco de dados" em memória RAM.
 */
static Cliente clientes[MAX_CLIENTES];
static int      num_clientes = 0;

/* * BROADCAST: O coração do chat. 
 * Ele percorre a lista de clientes e tenta "empurrar" a string para o socket de cada um.
 */
void broadcast(const char *mensagem, int excluir_fd)
{
    for (int i = 0; i < num_clientes; i++) {
        /* Não envia a mensagem de volta para quem a escreveu (excluir_fd) */
        if (clientes[i].fd != excluir_fd) {
            if (send(clientes[i].fd, mensagem, strlen(mensagem), 0) == -1) {
                perror("Erro ao enviar broadcast");
            }
        }
    }
}

/* * ADICIONAR CLIENTE: Insere na próxima posição vaga do array.
 */
int adicionar_cliente(int fd, const char *nome, const char *ip, int porta)
{
    if (num_clientes >= MAX_CLIENTES) return -1;

    clientes[num_clientes].fd = fd;
    clientes[num_clientes].porta = porta;
    strncpy(clientes[num_clientes].nome, nome, NOME_SIZE - 1);
    clientes[num_clientes].nome[NOME_SIZE - 1] = '\0';
    strncpy(clientes[num_clientes].ip, ip, INET_ADDRSTRLEN - 1);
    
    num_clientes++;
    return 0;
}

/* * REMOVER CLIENTE: Quando alguém sai, "puxamos" o último cliente da lista
 * para a vaga que ficou aberta, mantendo o array compacto.
 */
void remover_cliente(int fd, char *nome_out, size_t nome_out_size)
{
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].fd == fd) {
            strncpy(nome_out, clientes[i].nome, nome_out_size - 1);
            nome_out[nome_out_size - 1] = '\0';

            /* Técnica de Swap: substitui o atual pelo último e decrementa o contador */
            clientes[i] = clientes[num_clientes - 1];
            num_clientes--;
            return;
        }
    }
    strncpy(nome_out, "Desconhecido", nome_out_size - 1);
}

const char *nome_por_fd(int fd)
{
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].fd == fd) return clientes[i].nome;
    }
    return "Desconhecido";
}

int main(void)
{
    int server_fd;
    struct sockaddr_in servidor_addr;
    int opt = 1;

    /* * PASSO 1: CRIAÇÃO DO SOCKET 
     * AF_INET = IPV4 | SOCK_STREAM = TCP 
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    /* SO_REUSEADDR: Evita o erro "Address already in use" se você reiniciar o servidor rápido demais */
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* * PASSO 2: CONFIGURAÇÃO DO ENDEREÇO 
     * Aqui definimos que o servidor vai aceitar conexões em qualquer IP da máquina (INADDR_ANY) na porta 8080.
     */
    memset(&servidor_addr, 0, sizeof(servidor_addr));
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_addr.s_addr = INADDR_ANY;
    servidor_addr.sin_port = htons(PORTA); /* htons: converte o número para o formato da rede */

    /* Bind: Vincula o socket à porta física */
    if (bind(server_fd, (struct sockaddr *)&servidor_addr, sizeof(servidor_addr)) == -1) {
        perror("Erro no bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Listen: Coloca o socket em modo passivo, esperando por clientes */
    if (listen(server_fd, 10) == -1) {
        perror("Erro no listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║       BATALHA DE PALAVRAS — Servidor         ║\n");
    printf("║   Porta: %d                                ║\n", PORTA);
    printf("║   Aguardando jogadores (pares de 2)...       ║\n");
    printf("╚══════════════════════════════════════════════╝\n");

    /* * PASSO 3: O LOOP MÁGICO DO SELECT 
     * O select() permite que um único processo cuide de vários sockets sem usar Threads.
     */
    while (1) {
        fd_set read_fds;  /* Um "bitmap" onde marcamos quais FDs queremos monitorar */
        int    max_fd;    /* O select precisa saber qual o maior número de ID de socket para fazer a busca */

        /* Limpa o conjunto e adiciona o servidor principal (ele avisa quando há novos clientes) */
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        /* Adiciona cada cliente já conectado ao conjunto de monitoramento */
        for (int i = 0; i < num_clientes; i++) {
            FD_SET(clientes[i].fd, &read_fds);
            if (clientes[i].fd > max_fd) max_fd = clientes[i].fd;
        }

        /* * SELECT(): O programa para aqui e dorme. O Sistema Operacional acorda ele quando:
         * 1. Alguém bate na porta (server_fd pronto)
         * 2. Alguém mandou mensagem ou desconectou (clientes[i].fd pronto)
         */
        int atividade = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (atividade == -1) {
            if (errno == EINTR) continue;
            perror("Erro no select");
            break;
        }

        /* -------------------------------------------------------
         * TRATANDO NOVA CONEXÃO (Novo Cliente batendo na porta)
         * ------------------------------------------------------- */
        if (FD_ISSET(server_fd, &read_fds)) {
            struct sockaddr_in cliente_addr;
            socklen_t cliente_len = sizeof(cliente_addr);

            /* Accept: Cria um NOVO socket exclusivo para este cliente específico */
            int novo_fd = accept(server_fd, (struct sockaddr *)&cliente_addr, &cliente_len);
            if (novo_fd == -1) {
                perror("Erro no accept");
                continue;
            }

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cliente_addr.sin_addr, ip_str, sizeof(ip_str));
            int porta_cliente = ntohs(cliente_addr.sin_port);

            /* O servidor espera que a primeira mensagem do cliente seja o seu NOME */
            char nome[NOME_SIZE] = {0};
            ssize_t n = recv(novo_fd, nome, NOME_SIZE - 1, 0);
            
            if (n <= 0) {
                close(novo_fd);
                continue;
            }
            nome[n] = '\0';
            
            /* Limpeza: remove o \n que alguns terminais enviam ao apertar Enter */
            char *newline = strchr(nome, '\n');
            if (newline) *newline = '\0';

            /* Adiciona o cliente na nossa lista interna */
            if (adicionar_cliente(novo_fd, nome, ip_str, porta_cliente) == -1) {
                const char *msg_cheio = "Servidor cheio.\n";
                send(novo_fd, msg_cheio, strlen(msg_cheio), 0);
                close(novo_fd);
                continue;
            }

            /* Notificação Global: Avisa a todos que alguém entrou */
            char aviso[BUFFER_SIZE];
            snprintf(aviso, sizeof(aviso), ">>> %s entrou no chat <<<\n", nome);
            broadcast(aviso, novo_fd);
            
            /* Confirmação para o cliente que acabou de conectar */
            send(novo_fd, "Conectado com sucesso!\n", 23, 0);
            printf("[+] %s (%s:%d) conectado.\n", nome, ip_str, porta_cliente);
        }

        /* -------------------------------------------------------
         * TRATANDO MENSAGENS DOS CLIENTES JÁ CONECTADOS
         * ------------------------------------------------------- */
        for (int i = num_clientes - 1; i >= 0; i--) {
            int cli_fd = clientes[i].fd;

            /* Se este cliente específico do array não mandou nada, pula para o próximo */
            if (!FD_ISSET(cli_fd, &read_fds)) continue;

            char buffer[BUFFER_SIZE] = {0};
            ssize_t bytes = recv(cli_fd, buffer, BUFFER_SIZE - 1, 0);

            if (bytes <= 0) {
                /* RECV retornou 0 ou -1: O cliente fechou a conexão ou caiu */
                char nome_saiu[NOME_SIZE];
                remover_cliente(cli_fd, nome_saiu, sizeof(nome_saiu));
                close(cli_fd);

                char aviso[BUFFER_SIZE];
                snprintf(aviso, sizeof(aviso), ">>> %s saiu do chat <<<\n", nome_saiu);
                broadcast(aviso, -1);
                printf("[-] %s desconectado.\n", nome_saiu);
            } else {
                /* MENSAGEM RECEBIDA: Formata e retransmite para os outros */
                buffer[bytes] = '\0';
                
                char *newline = strchr(buffer, '\n');
                if (newline) *newline = '\0';

                if (strlen(buffer) == 0) continue;

                const char *nome_remetente = nome_por_fd(cli_fd);
                char msg_formatada[BUFFER_SIZE];
                snprintf(msg_formatada, sizeof(msg_formatada), "[%s]: %s\n", nome_remetente, buffer);
                
                printf("%s", msg_formatada); /* Log no console do servidor */
                broadcast(msg_formatada, cli_fd); /* Envia para todos menos para o autor */
            }
        }
    }

    /* * FECHAMENTO: Boa prática para liberar os recursos do S.O. 
     */
    close(server_fd);
    return 0;
}