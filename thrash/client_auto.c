#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX 1024
#define PORT 8080

// Variable global que indica cuándo finalizar la comunicación.
volatile int exit_flag = 0;

// Hilo encargado de leer mensajes que le llegan del servidor.
void *read_msg(void *arg) {
    int sockfd = *(int *)arg;
    char buff[MAX];
    int n;
    
    while (!exit_flag) {
        memset(buff, 0, MAX);               // Limpia el buffer.
        n = read(sockfd, buff, MAX);        // Lee hasta MAX bytes del socket.
        if (n <= 0) {
            printf("Desconexión o error en recepción.\n");
            exit_flag = 1;
            break;
        }
        // Imprime el mensaje recibido.
        printf("Mensaje recibido: %s", buff);
    }
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    pthread_t r_thread;
    
    // Crear el socket (TCP/IPv4).
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket del cliente creado.\n");
    
    // Configurar la dirección del servidor.
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                   // IPv4.
    serv_addr.sin_port = htons(PORT);                 // Puerto, convertido al orden de red.
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección del servidor.
    
    // Conectar con el servidor.
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error en connect");
        exit(EXIT_FAILURE);
    }
    printf("Conectado al servidor.\n");
    
    // Crear un hilo para leer mensajes entrantes.
    if (pthread_create(&r_thread, NULL, read_msg, (void *)&sockfd) != 0) {
        perror("Error al crear el hilo de lectura");
        exit(EXIT_FAILURE);
    }
    
    // Array de mensajes predefinidos (en este ejemplo, 3 mensajes).
    const char *messages[] = {
        "Mensaje 1: Hola desde cliente.\n",
        "Mensaje 2: Este es otro mensaje.\n",
        "exit\n"  // El mensaje "exit" se utiliza para finalizar la sesión.
    };
    
    int num_messages = sizeof(messages) / sizeof(messages[0]);
    
    // Enviar cada mensaje al servidor.
    for (int i = 0; i < num_messages; i++) {
        write(sockfd, messages[i], strlen(messages[i]));
        printf("Mensaje enviado: %s", messages[i]);
        sleep(1);  // Retraso para simular un intervalo entre mensajes.
    }
    
    // Espera a que el hilo de lectura termine.
    pthread_join(r_thread, NULL);
    
    // Cerrar el socket.
    close(sockfd);
    return 0;
}
