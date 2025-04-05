#ifndef SESSION_HANDLER_H
#define SESSION_HANDLER_H
#include "protocol.h"

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
void process_messages(int sock1, int sock2, int game_id, const char* username1, const char* username2,  ProtocolMessage msg);

#endif // SESSION_HANDLER_H
