#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "protocol.h"

#define MAX 1024
#define PORT 8080

volatile int exit_flag = 0;

void *read_msg(void *arg) {
    int sockfd = *(int*)arg;
    char buff[MAX];
    int n;
    while (!exit_flag) {
        memset(buff, 0, MAX);
        n = read(sockfd, buff, MAX);
        if (n <= 0) {
            printf("\nDesconexión o error en la recepción.\n");
            exit_flag = 1;
            break;
        }
        printf("\nMensaje recibido: %s", buff);
        if (strncmp(buff, "exit", 4) == 0) {
            printf("El otro usuario ha cerrado la sesión.\n");
            exit_flag = 1;
            break;
        }
    }
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buff[MAX];
    pthread_t r_thread;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket del cliente creado.\n");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error en connect");
        exit(EXIT_FAILURE);
    }
    printf("Conectado al servidor.\n");
    
    // Solicitar el username al usuario.
    char username[50];
    printf("Ingrese su username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf("Error leyendo username.\n");
        exit(EXIT_FAILURE);
    }
    username[strcspn(username, "\n")] = '\0';
    
    // Construir y enviar el mensaje de login.
    ProtocolMessage loginMsg;
    loginMsg.type = MSG_LOGIN;
    loginMsg.game_id = 0;
    strncpy(loginMsg.data, username, sizeof(loginMsg.data) - 1);
    loginMsg.data[sizeof(loginMsg.data) - 1] = '\0';
    
    char login_str[MAX];
    format_message(loginMsg, login_str, MAX);
    write(sockfd, login_str, strlen(login_str));
    
    // Esperar el ACK de login.
    memset(buff, 0, MAX);
    int n = read(sockfd, buff, MAX);
    if (n <= 0) {
        perror("Error al recibir ACK de login");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Login exitoso: %s\n", buff);
    
    // Crear hilo para leer mensajes.
    if (pthread_create(&r_thread, NULL, read_msg, (void *)&sockfd) != 0) {
        perror("Error al crear el hilo de lectura");
        exit(EXIT_FAILURE);
    }
    
    // Enviar mensajes en el hilo principal.
    while (!exit_flag) {
        printf("Escribe el mensaje: ");
        memset(buff, 0, MAX);
        if (fgets(buff, MAX, stdin) == NULL) break;
        write(sockfd, buff, strlen(buff));
        if (strncmp(buff, "exit", 4) == 0) {
            exit_flag = 1;
            break;
        }
    }
    
    pthread_join(r_thread, NULL);
    close(sockfd);
    return 0;
}
