#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include "session_handler.h"
#include "protocol.h"

#define PORT 8080
#define MAX 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    
    // Crear el socket de escucha.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket del servidor creado.\n");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }
    printf("Socket enlazado a puerto %d.\n", PORT);
    
    if (listen(sockfd, 5) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }
    printf("Servidor escuchando...\n");
    
    int client_counter = 0;
    
    while (1) {
        int client_sock1, client_sock2;
        int client_id1, client_id2;
        char username1[50], username2[50];
        char buff[MAX];
        int n;
        ProtocolMessage msg;
        char ack[MAX];
        
        // Esperar al primer cliente.
        printf("Esperando al primer cliente...\n");
        client_sock1 = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (client_sock1 < 0) {
            perror("Error en accept (cliente 1)");
            continue;
        }
        client_id1 = ++client_counter;

        // Recibir mensaje de login del primer cliente.
        memset(buff, 0, MAX);
        n = read(client_sock1, buff, MAX);
        if (n <= 0) {
            perror("Error al leer login del cliente 1");
            close(client_sock1);
            continue;
        }
        if (!parse_message(buff, &msg) || msg.type != MSG_LOGIN) {
            printf("Cliente %d: Login mal formado o tipo incorrecto.\n", client_id1);
            close(client_sock1);
            continue;
        }
        
        strncpy(username1, msg.data, sizeof(username1) - 1);
        username1[sizeof(username1) - 1] = '\0';
        // Enviar ACK de login.
        ProtocolMessage ackMsg;
        ackMsg.type = MSG_LOGGED;
        ackMsg.game_id = 0;
        strncpy(ackMsg.data, "Ok", sizeof(ackMsg.data) - 1);
        ackMsg.data[sizeof(ackMsg.data) - 1] = '\0';
        format_message(ackMsg, ack, MAX);
        write(client_sock1, ack, strlen(ack));
        printf("Cliente %d autenticado como %s.\n", client_id1, username1);
        
        // Esperar al segundo cliente.
        printf("Esperando al segundo cliente...\n");
        client_sock2 = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (client_sock2 < 0) {
            perror("Error en accept (cliente 2)");
            close(client_sock1);
            continue;
        }
        client_id2 = ++client_counter;
        memset(buff, 0, MAX);
        n = read(client_sock2, buff, MAX);
        if (n <= 0) {
            perror("Error al leer login del cliente 2");
            close(client_sock1);
            close(client_sock2);
            continue;
        }
        if (!parse_message(buff, &msg) || msg.type != MSG_LOGIN) {
            printf("Cliente %d: Login mal formado o tipo incorrecto.\n", client_id2);
            close(client_sock1);
            close(client_sock2);
            continue;
        }
        strncpy(username2, msg.data, sizeof(username2) - 1);
        username2[sizeof(username2) - 1] = '\0';
        // Enviar ACK de login.
        ackMsg.type = MSG_LOGGED;
        ackMsg.game_id = 0;
        strncpy(ackMsg.data, "Ok", sizeof(ackMsg.data) - 1);
        ackMsg.data[sizeof(ackMsg.data) - 1] = '\0';
        format_message(ackMsg, ack, MAX);
        write(client_sock2, ack, strlen(ack));
        printf("Cliente %d autenticado como %s.\n", client_id2, username2);
        
        // Crear la sesión y asignar sockets, IDs y usernames.
        session_pair_t *session = malloc(sizeof(session_pair_t));
        if (session == NULL) {
            perror("Error al asignar memoria para la sesión");
            close(client_sock1);
            close(client_sock2);
            continue;
        }
        session->client_sock1 = client_sock1;
        session->client_sock2 = client_sock2;
        session->client_id1 = client_id1;
        session->client_id2 = client_id2;
        strncpy(session->username1, username1, sizeof(session->username1) - 1);
        session->username1[sizeof(session->username1) - 1] = '\0';
        strncpy(session->username2, username2, sizeof(session->username2) - 1);
        session->username2[sizeof(session->username2) - 1] = '\0';
        
        // Crear un hilo para manejar la sesión entre estos dos clientes.
        pthread_t tid;
        if (pthread_create(&tid, NULL, session_handler, (void*)session) != 0) {
            perror("Error al crear el hilo");
            free(session);
            close(client_sock1);
            close(client_sock2);
            continue;
        }
        pthread_detach(tid);
    }
    
    close(sockfd);
    return 0;
}
