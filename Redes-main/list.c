#include <stdio.h>
#include <string.h>

#include "list.h"
#include "udp.h"

int list_resources(const AppConfig *config)
{
    /* O buffer de resposta precisa de ter espaço para ate 50 nomes de ficheiro */
    char message[16];
    char response[2048];
    char response_type[10];
    char status[10];

    /* 1. Construir a mensagem UDP: LST\n */
    snprintf(message, sizeof(message), "LST\n");

    /* 2. Enviar o pedido ao Directory Server e receber a resposta */
    if (send_udp_request(config, message, response, sizeof(response)) != 0) {
        printf("error communicating with Directory Server\n");
        return -1;
    }

    /* 3. Fazer parse inicial do tipo e do status */
    if (sscanf(response, "%9s %9s", response_type, status) < 2) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    /* 4. Verificar se a resposta e RLS */
    if (strcmp(response_type, "RLS") != 0) {
        printf("unexpected response from Directory Server\n");
        return -1;
    }

    /* 5. Verificar o status */
    if (strcmp(status, "NOK") == 0) {
        printf("no resources currently available\n");
        return 0;
    }

    if (strcmp(status, "OK") == 0) {
        /* Extrair os nomes dos ficheiros da resposta a seguir a "RLS OK" */
        char filename[64];
        char *ptr = response;

        /* Saltar a primeira palavra (RLS) e a segunda (OK) */
        char dummy[10];
        if (sscanf(ptr, "%9s %9s", dummy, dummy) == 2) {
            /* Avancamos o apontador para depois de "RLS OK" */
            ptr = strstr(ptr, "OK");
            if (ptr != NULL) {
                ptr += 2; /* Avanca o tamanho de "OK" */
            }
        }

        int count = 0;
        int bytes_read = 0;

        /* Ler cada nome de ficheiro restante na linha */
        while (ptr != NULL && sscanf(ptr, "%63s%n", filename, &bytes_read) == 1) {
            if (count == 0) {
                printf("Available resources:\n");
            }
            count++;
            printf("  %d. %s\n", count, filename);
            ptr += bytes_read;
        }

        if (count == 0) {
            printf("no resources currently available\n");
        }

        return 0;
    }

    printf("error retrieving resource list\n");
    return -1;
}