#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Definimos los tipos de mensajes.
typedef enum {
    MSG_LOGIN,   // Enviado por el cliente para login.
    MSG_LOGGED,  // ACK de login.
    MSG_ATTACK,  // Ataque enviado por el cliente.
    MSG_RESULT,  // Respuesta para el atacante.
    MSG_UPDATE,  // Respuesta para el defensor.
    MSG_END,     // Fin de partida.
    MSG_ERROR,   // Mensaje de error.
    MSG_INVALID, // Tipo inválido.
    MSG_START    // (Opcional)
} MessageType;

// Mapeo de strings para cada tipo.
static const char* MessageTypeStr[] = {
    "LOGIN",
    "LOGGED",
    "ATTACK",
    "RESULT",
    "UPDATE",
    "GAME_END",
    "ERROR",
    "INVALID"
    // "GAME_START"  // Opcional.
};

// Estructura para representar un mensaje.
typedef struct {
    MessageType type;
    int game_id;       // ID de partida.
    char data[256];    // Para username, parámetros de ataque, etc.
} ProtocolMessage;

const char* message_type_to_string(MessageType type);
MessageType string_to_message_type(const char* str);
bool parse_message(const char* input, ProtocolMessage* msg);
void format_message(ProtocolMessage msg, char* output, size_t size);

#endif
