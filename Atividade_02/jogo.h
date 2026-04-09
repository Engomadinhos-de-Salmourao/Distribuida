#ifndef JOGO_H
#define JOGO_H

#include "protocolo.h"

/* Valida se a palavra atende às regras da rodada.
 * Retorna 1 se válida, 0 se inválida. */
int validar_palavra(const char *palavra, char letra_rodada);

/* Gera uma letra aleatória (A-Z). */
char gerar_letra(void);

/* Envia uma mensagem formatada pelo protocolo para o fd.
 * formato: "PREFIXO|campo1|campo2\n"
 * Retorna bytes enviados ou -1 em erro. */
int enviar_msg(int fd, const char *fmt, ...);

/* Recebe uma linha do fd com timeout em segundos.
 * buf deve ter tamanho bufsz.
 * Retorna bytes lidos, 0 em timeout/desconexão, -1 em erro. */
int receber_com_timeout(int fd, char *buf, int bufsz, int timeout_seg);

#endif /* JOGO_H */
