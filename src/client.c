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
volatile int myTurn = 0; // 1 = tiene turno, 0 = no tiene turno

// Actualiza el turno según los mensajes recibidos.
// Se espera que los mensajes ACK, RESULT, UPDATE o END incluyan el turno como el cuarto campo.
void update_turn(const char* message) {
    int turn;
    if (sscanf(message, "LOGGED|%*d|%*[^|]|%d|", &turn) == 1 ||
        sscanf(message, "RESULT|%*d|%*[^|]|%d", &turn) == 1 ||
        sscanf(message, "UPDATE|%*d|%*[^|]|%d", &turn) == 1 ||
        sscanf(message, "END|%*d|%*[^|]|%d", &turn) == 1) {
        myTurn = turn;
        printf("Turno actualizado a: %d\n", myTurn);
    }
}

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
        update_turn(buff);
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
    if (sockfd < 0) { perror("Error al crear el socket"); exit(EXIT_FAILURE); }
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
    
    // Solicitar username.
    char username[50];
    printf("Ingrese su username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf("Error leyendo username.\n");
        exit(EXIT_FAILURE);
    }
    username[strcspn(username, "\n")] = '\0';
    
    // Solicitar game_id.
    int game_id;
    printf("Ingrese el ID de partida: ");
    if (scanf("%d", &game_id) != 1) {
        printf("Error leyendo el ID de partida.\n");
        exit(EXIT_FAILURE);
    }
    while(getchar() != '\n'); // Limpiar buffer.
    
    // Construir y enviar mensaje de login.
    ProtocolMessage loginMsg;
    loginMsg.type = MSG_LOGIN;
    loginMsg.game_id = game_id;
    strncpy(loginMsg.data, username, sizeof(loginMsg.data)-1);
    loginMsg.data[sizeof(loginMsg.data)-1] = '\0';
    
    char login_str[MAX];
    format_message(loginMsg, login_str, MAX);
    write(sockfd, login_str, strlen(login_str));
    
    // Esperar ACK de login.
    memset(buff, 0, MAX);
    int n = read(sockfd, buff, MAX);
    if (n <= 0) {
        perror("Error al recibir ACK de login");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Login exitoso: %s\n", buff);
    update_turn(buff); // Actualiza el turno según ACK.
    
    // Crear hilo para leer mensajes.
    if (pthread_create(&r_thread, NULL, read_msg, (void *)&sockfd) != 0) {
        perror("Error al crear el hilo de lectura");
        exit(EXIT_FAILURE);
    }
    
    // Bucle principal: solo enviar mensajes si es su turno.
    while (!exit_flag) {
        if (!myTurn) {
            printf("No es tu turno. Esperando...\n");
            sleep(1);
            continue;
        }
        printf("Escribe el mensaje: ");
        memset(buff, 0, MAX);
        if (fgets(buff, MAX, stdin) == NULL) break;
        write(sockfd, buff, strlen(buff));
        // Después de enviar, se asume que el turno se cede (hasta que se actualice con respuesta del servidor).
        myTurn = 0;
        if (strncmp(buff, "exit", 4) == 0) {
            exit_flag = 1;
            break;
        }
    }
    
    pthread_join(r_thread, NULL);
    close(sockfd);
    return 0;
}
 