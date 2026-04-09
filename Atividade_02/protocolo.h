#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#define PORTA_PADRAO 7070
#define NUM_RODADAS  5
#define TEMPO_RODADA 10
#define MIN_CHARS    5
#define MAX_NOME     64
#define MAX_PALAVRA  128
#define MAX_MSG      512
#define NUM_JOGADORES 2

/* Prefixos servidor -> cliente */
#define P_MSG       "MSG"
#define P_NOME_REQ  "NOME"
#define P_AGUARDE   "AGUARDE"
#define P_RODADA    "RODADA"
#define P_RESULTADO "RESULTADO"
#define P_PLACAR    "PLACAR"
#define P_FIM       "FIM"

/* Prefixos cliente -> servidor */
/* P_NOME_REQ já usado para a resposta também */
#define P_PALAVRA   "PALAVRA"
#define P_TIMEOUT_C "TIMEOUT"

#endif /* PROTOCOLO_H */
