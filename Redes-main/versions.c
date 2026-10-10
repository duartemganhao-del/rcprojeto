#include <stdio.h>
#include <string.h>

#include "versions.h"
#include "tcp.h"
#include "validation.h"

int versions(const AppConfig *config, const char *filename)
{
    char message[128];
    char response[4096];
    char response_type[10], status[10];

    /* 1. Validar sintaxe do filename */
    if (!validate_filename(filename)) {
        printf("invalid filename syntax\n");
        return -1;
    }

    /* 2. Construir mensagem TCP: VRS filename\n */
    snprintf(message, sizeof(message), "VRS %s\n", filename);

    /* 3. Enviar via TCP e receber resposta */
    if (send_tcp_request(config, message, response, sizeof(response)) != 0) {
        printf("error communicating with Directory Server\n");
        return -1;
    }

    /* 4. Parse do tipo de resposta e status */
    if (sscanf(response, "%9s %9s", response_type, status) < 2) {
        printf("invalid response from Directory Server\n");
        return -1;
    }

    if (strcmp(response_type, "RVR") != 0) {
        printf("unexpected response from Directory Server\n");
        return -1;
    }

    /* 5. Se o status for NOK, nao ha versoes */
    if (strcmp(status, "NOK") == 0) {
        printf("no versions available for this resource\n");
        return 0;
    }

    if (strcmp(status, "OK") == 0) {
        /* Avancar o apontador para a frente de "RVR OK" */
        char *ptr = strstr(response, "OK");
        if (ptr == NULL)
            return -1;

        ptr += 2; /* Saltar "OK" */

        char peer_uid[16];
        long fsize;
        char label[64];
        char pub_time[32];
        char availability[10];
        int bytes_consumed = 0;
        int count = 0;

         /* Cada versao contem 5 campos: UID Fsize label publication_time availability */
        while (sscanf(ptr, "%15s %ld %63s %31s %9s%n",
                      peer_uid, &fsize, label, pub_time, availability, &bytes_consumed) == 5) {
            if (count == 0) {
                printf("Available versions for '%s':\n", filename);
                printf("%-4s %-10s %-10s %-18s %-20s %-10s\n",
                       "No.", "UID", "Size(B)", "Label", "Date-Time", "Status");
                printf("--------------------------------------------------------------------------\n");
            }
            count++;

            const char *status_str = (strcmp(availability, "AVL") == 0) ? "online" : "offline";

            printf("%-4d %-10s %-10ld %-18s %-20s %-10s\n",
                   count, peer_uid, fsize, label, pub_time, status_str);

            ptr += bytes_consumed;
        }

        if (count == 0) {
            printf("no versions available for this resource\n");
        }

        return 0;
    }

    printf("error retrieving versions\n");
    return -1;
}