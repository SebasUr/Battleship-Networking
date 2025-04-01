#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "session_handler.h"

#define MAX 1024

void *session_handler(void *arg) {
    session_pair_t *session = (session_pair_t *)arg;
    int sock1 = session->client_sock1;
    int sock2 = session->client_sock2;
    int client_id1 = session->client_id1;
    int client_id2 = session->client_id2;
    char username1[50], username2[50];
    strncpy(username1, session->username1, sizeof(username1));
    username1[sizeof(username1)-1] = '\0';
    strncpy(username2, session->username2, sizeof(username2));
    username2[sizeof(username2)-1] = '\0';
    free(session);
    char buff[MAX];
    int n;
    
    printf("Iniciando sesión de chat entre %s y %s...\n", username1, username2);
    
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        int max_sd = (sock1 > sock2) ? sock1 : sock2;
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }
        if (FD_ISSET(sock1, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock1, buff, MAX);
            if (n <= 0) {
                printf("%s desconectado.\n", username1);
                break;
            }
            printf("%s: %s", username1, buff);
            if (strncmp(buff, "exit", 4) == 0) {
                write(sock2, buff, n);
                break;
            }
            write(sock2, buff, n);
        }
        if (FD_ISSET(sock2, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock2, buff, MAX);
            if (n <= 0) {
                printf("%s desconectado.\n", username2);
                break;
            }
            printf("%s: %s", username2, buff);
            if (strncmp(buff, "exit", 4) == 0) {
                write(sock1, buff, n);
                break;
            }
            write(sock1, buff, n);
        }
    }
    
    close(sock1);
    close(sock2);
    printf("Sesión de chat finalizada entre %s y %s.\n", username1, username2);
    return NULL;
}
