#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include "session_handler.h"
#include "protocol.h"
#include "game_manager.h"
#include "Utils.h"

#define PORT 8080
#define MAX 1024

// Función auxiliar para agregar un cliente a la lista de espera.
void add_waiting_client(waiting_client_t **list, waiting_client_t *client) {
    client->next = *list;
    *list = client;
}

// Función auxiliar para buscar y remover un cliente de la lista de espera con el mismo game_id.
waiting_client_t* pop_waiting_client(waiting_client_t **list, int game_id) {
    waiting_client_t *prev = NULL, *curr = *list;
    while (curr != NULL) {
        if (curr->game_id == game_id) {
            if (prev == NULL) {
                *list = curr->next;
            } else {
                prev->next = curr->next;
            }
            curr->next = NULL;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

// Función auxiliar para agregar una sala como ocupada
void add_room(rooms **list, rooms *room){
    room->next = *list;
    *list = room;
}

//Función auxiliar para buscar y/o remover una sala de la lista de salas ocupadas
rooms* search_room(rooms **list, int game_id, bool remove) {
    rooms *prev = NULL, *curr = *list;
    while (curr != NULL) {
        if (curr->game_id == game_id) {
            if(remove){
                if (prev == NULL) {
                    *list = curr->next;
                } else {
                    prev->next = curr->next;
                }
                curr->next = NULL;
            }
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

// Inicialización del socket del servidor
int setup_server_socket() {
    int sockfd;
    struct sockaddr_in serv_addr;

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

    printf("Server escuchando...\n");
    return sockfd;
}

// Manejo de la conexión de un cliente
void handle_client_connection(int client_sock, int client_id, struct sockaddr_in cli_addr,
                              waiting_client_t **waiting_list, rooms **rooms_list) {
    char buff[MAX];
    ProtocolMessage msg;

    // Recibir mensaje de login del cliente.
    memset(buff, 0, MAX);
    int n = read(client_sock, buff, MAX);
    if (n <= 0) {
        perror("Error al leer login del cliente"); 
        close(client_sock);
        return;
    }

    if (!parse_message(buff, &msg) || msg.type != MSG_LOGIN) {
        printf("Cliente %d: Login mal formado o tipo incorrecto.\n", client_id);
        close(client_sock);
        return;
    }

    // Para el login: msg.game_id es el MatchID y msg.data es el username.
    int game_id = msg.game_id;
    char username[50];
    strncpy(username, msg.data, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';

     // Procesar el login mediante game_manager.
    ProtocolMessage ackMsg;
    ackMsg.type = MSG_LOGGED;
    ackMsg.game_id = game_id;
    char initial_info[100];
    int turn;

    // Buscar en la lista de salas si la sala con el game_id está ocupada.
    rooms *room = search_room(rooms_list, game_id, false);
    bool empty_room = true;
    if(room!=NULL){
        empty_room = false;
    }
    if(empty_room){
        if (game_manager_process_login(game_id, username, initial_info, sizeof(initial_info), &turn) != 0) {
            printf("Error en game_manager_process_login para Cliente %d.\n", client_id);
            close(client_sock);
            return;
        }
    }

    // Preparar el ACK usando el protocolo para tener el prefijo "LOGGED"
    // El ACK tendrá el formato: "LOGGED|MatchID|Ok|<turn>|<initial_info>
    char temp_data[150];
    if (empty_room) {
        snprintf(temp_data, sizeof(temp_data), "Ok|%d|%s", turn, initial_info);
    } else {
        turn = -1;
        snprintf(temp_data, sizeof(temp_data), "NO|%d", turn);
    }

    strncpy(ackMsg.data, temp_data, sizeof(ackMsg.data) - 1);
    ackMsg.data[sizeof(ackMsg.data) - 1] = '\0';

    waiting_client_t *waiting = pop_waiting_client(waiting_list, game_id);

    if (empty_room) {
        if (waiting == NULL) {
            // Primer cliente en la sala para este game_id.
            // El turno que devuelve game_manager ya debería ser 1.
            turn = 1;
            snprintf(temp_data, sizeof(temp_data), "Ok|%d|%s", turn, initial_info);
            strncpy(ackMsg.data, temp_data, sizeof(ackMsg.data)-1);
            ackMsg.data[sizeof(ackMsg.data)-1] = '\0';
            send_login_ack(client_sock, &ackMsg);
            printf("Cliente %d autenticado como %s en partida %d. (Turno: %d)\n", client_id, username, game_id, turn);

            // Agregar este cliente a la lista de espera.
            waiting_client_t *new_waiting = malloc(sizeof(waiting_client_t));
            if (!new_waiting) {
                perror("Error al asignar memoria para cliente en espera");
                close(client_sock);
                return;
            }

            new_waiting->sock = client_sock;
            new_waiting->client_id = client_id;
            new_waiting->game_id = game_id;
            strncpy(new_waiting->username, username, sizeof(new_waiting->username) - 1);
            new_waiting->username[sizeof(new_waiting->username) - 1] = '\0';
            new_waiting->next = NULL;
            add_waiting_client(waiting_list, new_waiting);

            printf("Cliente %d en espera para partida %d.\n", client_id, game_id);

        } else {
            // Se encontró un cliente esperando con el mismo game_id: este es el segundo.
            // Para el segundo cliente, forzamos turno = 0.
            turn = 0;
            snprintf(temp_data, sizeof(temp_data), "Ok|%d|%s", turn, initial_info);
            strncpy(ackMsg.data, temp_data, sizeof(ackMsg.data) - 1);
            ackMsg.data[sizeof(ackMsg.data) - 1] = '\0';
            send_login_ack(client_sock, &ackMsg);
            printf("Cliente %d autenticado como %s en partida %d. (Turno: %d)\n", client_id, username, game_id, turn);

            // Emparejar: el cliente que estaba en espera (primero) y este segundo cliente.
            create_session(waiting, client_sock, client_id, game_id, username, rooms_list);
        }
    } else {
        turn = -1;
        snprintf(temp_data, sizeof(temp_data), "NO|%d", turn);
        strncpy(ackMsg.data, temp_data, sizeof(ackMsg.data)-1);
        ackMsg.data[sizeof(ackMsg.data)-1] = '\0';
        send_login_ack(client_sock, &ackMsg);
        printf("Cliente %d (%s) no fue asignado a ninguna partida. El game_id %d ya està en uso.\n",
                    client_id, username, game_id);
    }
}

// Enviar ACK al cliente
void send_login_ack(int client_sock, ProtocolMessage *ackMsg) {
    char formatted_ack[MAX];
    format_message(*ackMsg, formatted_ack, MAX);
    write(client_sock, formatted_ack, strlen(formatted_ack));
    
}
// Crear una nueva sesión entre dos clientes
void create_session(waiting_client_t *waiting, int client_sock, int client_id, int game_id,
                    const char *username, rooms **rooms_list) {
    int client_sock1 = waiting->sock;
    int client_id1 = waiting->client_id;
    char username1[50];
    strncpy(username1, waiting->username, sizeof(username1));
    username1[sizeof(username1) - 1] = '\0';
    free(waiting);

    int client_sock2 = client_sock;
    int client_id2 = client_id;
    char username2[50];
    strncpy(username2, username, sizeof(username2));
    username2[sizeof(username2) - 1] = '\0';
    
    //Agregar la sala a la lista de salas ocupadas
    rooms *new_room = malloc(sizeof(rooms));
    if (!new_room) {
        perror("Error al asignar memoria para la sala");
        return;
    }

    new_room->game_id = game_id;
    new_room->next = NULL;
    add_room(rooms_list, new_room);

    // Crear la sesión y asignar datos.
    session_pair_t *session = malloc(sizeof(session_pair_t));
    if (session == NULL) {
        perror("Error al asignar memoria para la sesión");
        close(client_sock1);
        close(client_sock2);
        search_room(rooms_list, game_id, true);
        return;
    }

    session->client_sock1 = client_sock1;
    session->client_sock2 = client_sock2;
    session->client_id1 = client_id1;
    session->client_id2 = client_id2;
    session->game_id = game_id;
    strncpy(session->username1, username1, sizeof(session->username1) - 1);
    session->username1[sizeof(session->username1) - 1] = '\0';
    strncpy(session->username2, username2, sizeof(session->username2) - 1);
    session->username2[sizeof(session->username2) - 1] = '\0';

    // Crear un hilo para manejar la sesión.
    pthread_t tid;
    if (pthread_create(&tid, NULL, session_handler, (void *)session) != 0) {
        perror("Error al crear el hilo de sesión");
        free(session);
        close(client_sock1);
        close(client_sock2);
        search_room(rooms_list, game_id, true);
        return;
    }

    pthread_detach(tid);

    printf("Sesión creada entre Cliente %d (%s) y Cliente %d (%s) en partida %d.\n",
                    client_id1, username1, client_id2, username2, game_id);
}