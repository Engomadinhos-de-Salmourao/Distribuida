#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>

#include "jogo.h"
#include "protocolo.h"

int validar_palavra(const char *palavra, char letra_rodada)
{
    if (!palavra || palavra[0] == '\0')
        return 0;

    /* Verifica comprimento mínimo */
    int len = 0;
    for (int i = 0; palavra[i] != '\0'; i++) {
        if (!isalpha((unsigned char)palavra[i]))
            return 0;
        len++;
    }
    if (len < MIN_CHARS)
        return 0;

    /* Verifica letra inicial (case insensitive) */
    if (tolower((unsigned char)palavra[0]) != tolower((unsigned char)letra_rodada))
        return 0;

    return 1;
}

char gerar_letra(void)
{
    return (char)('A' + rand() % 26);
}

int enviar_msg(int fd, const char *fmt, ...)
{
    char buf[MAX_MSG];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);

    if (n < 0) return -1;

    /* Garante terminação com \n */
    if (n == 0 || buf[n - 1] != '\n') {
        buf[n] = '\n';
        buf[n + 1] = '\0';
        n++;
    }

    return (int)send(fd, buf, (size_t)n, MSG_NOSIGNAL);
}

int receber_com_timeout(int fd, char *buf, int bufsz, int timeout_seg)
{
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec  = timeout_seg;
    tv.tv_usec = 0;

    int ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0)
        return 0; /* timeout ou erro */

    /* Lê byte a byte até '\n' para mensagem completa */
    int total = 0;
    while (total < bufsz - 1) {
        char c;
        int r = (int)recv(fd, &c, 1, 0);
        if (r <= 0)
            return 0;
        buf[total++] = c;
        if (c == '\n')
            break;
    }
    buf[total] = '\0';

    /* Remove \r\n do final */
    while (total > 0 && (buf[total - 1] == '\n' || buf[total - 1] == '\r')) {
        buf[--total] = '\0';
    }

    return total;
}
