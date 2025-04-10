#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include "utils.h"


#define PORT 8080
#define MAX 1024

int main() {

    fclose(fopen("server.log", "w")); 

    FILE *log_file = fopen("server.log", "a");

    // Crear el socket de escucha.
    int sockfd = setup_server_socket(log_file);

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