#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include "utils.h"

#define DEFAULT_PORT    8080
#define DEFAULT_LOG     "server.log"
#define DEFAULT_IP      "0.0.0.0"
#define MAX 1024

int main(int argc, char *argv[]) {
    const char *bind_ip   = DEFAULT_IP;
    int         bind_port = DEFAULT_PORT;
    const char *log_path  = DEFAULT_LOG;

    // Leer argumentos
    if (argc > 1) bind_ip = argv[1]; 
    if (argc > 2) bind_port = atoi(argv[2]); 
    if (argc > 3) log_path = argv[3];

    fclose(fopen(log_path, "w")); 
    FILE *log_file = fopen(log_path, "a");
    if (!log_file) {
        perror("No se pudo abrir log_file");
        exit(EXIT_FAILURE);
    }

    // Crear el socket de escucha.
    int sockfd = setup_server_socket(bind_ip, bind_port, log_file);

    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int client_counter = 0;
    waiting_client_t *waiting_list = NULL;
    rooms *rooms_list = NULL;
    
    while (1) {
        int client_sock = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (client_sock < 0) {
            perror("Error en accept");
            continue;
        }
        int client_id = ++client_counter;
        handle_client_connection(client_sock, client_id, cli_addr, &waiting_list, &rooms_list, log_file);
    }

    fclose(log_file); 
    close(sockfd);
    return 0;
}