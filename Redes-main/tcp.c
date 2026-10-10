#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>

#include "tcp.h"

int send_tcp_request(const AppConfig *config,
                     const char *message,
                     char *response,
                     int response_size)
{
    struct addrinfo hints, *res;
    int fd, errcode;
    char port[6];
    ssize_t nwritten, nread;
    int total_read = 0;

    snprintf(port, sizeof(port), "%d", config->ds_port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       /* IPv4 */
    hints.ai_socktype = SOCK_STREAM; /* TCP */

    errcode = getaddrinfo(config->ds_ip, port, &hints, &res);
    if (errcode != 0)
        return -1;

    /* 1. Criar socket TCP */
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        freeaddrinfo(res);
        return -1;
    }

    /* 2. Definir timeout de 5 segundos para nao bloquear indefinidamente */
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    /* 3. Ligar ao servidor (TCP connect) */
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    freeaddrinfo(res);

    /* 4. Enviar mensagem completa com loop (conforme ensinado nos slides) */
    size_t to_write = strlen(message);
    const char *ptr_w = message;
    while (to_write > 0) {
        nwritten = write(fd, ptr_w, to_write);
        if (nwritten <= 0) {
            close(fd);
            return -1;
        }
        to_write -= nwritten;
        ptr_w += nwritten;
    }

    /* 5. Ler resposta byte a byte ate encontrar '\n' */
    while (total_read < response_size - 1) {
        nread = read(fd, &response[total_read], 1);
        if (nread <= 0) {
            break; /* Conexao fechada ou timeout */
        }
        if (response[total_read] == '\n') {
            total_read++;
            break;
        }
        total_read++;
    }

    response[total_read] = '\0';
    close(fd);

    if (total_read == 0)
        return -1;

    return 0;
}