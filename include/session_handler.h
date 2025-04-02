#ifndef SESSION_HANDLER_H
#define SESSION_HANDLER_H

typedef struct {
    int client_sock1;
    int client_sock2;
    int client_id1;
    int client_id2;
    int game_id;           // Nuevo: ID de partida
    char username1[50];
    char username2[50];
} session_pair_t;

// Función que será ejecutada por cada hilo para manejar la sesión de chat.
void *session_handler(void *arg);

#endif // SESSION_HANDLER_H
