#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Se agregan los nuevos tipos de mensaje.
typedef enum {
    MSG_LOGIN,   // Mensaje de login, enviado por el cliente con su username.
    MSG_LOGGED,  // ACK de login, enviado por el servidor.
    MSG_ATTACK,
    MSG_RESULT,
    MSG_UPDATE,
    MSG_END,
    MSG_ERROR,
    MSG_INVALID,  // Para errores.
    MSG_START
} MessageType;

// Mapeo de strings para los mensajes.
static const char* MessageTypeStr[] = {
    "LOGIN",
    "LOGGED",
    "ATTACK",
    "RESULT",
    "UPDATE",
    "GAME_END",
    "ERROR",
    "INVALID"
    "GAME_START", // Posiblemente se borrará. 
};

// Estructura para representar un mensaje del protocolo.
typedef struct {
    MessageType type;  // Tipo de mensaje.
    int game_id;       // ID de la partida (para comandos de juego, 0 en login).
    char data[256];    // Datos adicionales (por ejemplo, el username en el login).
} ProtocolMessage;

// Funciones para manejar mensajes del protocolo.
const char* message_type_to_string(MessageType type);
MessageType string_to_message_type(const char* str);
bool parse_message(const char* input, ProtocolMessage* msg);
void format_message(ProtocolMessage msg, char* output, size_t size);

#endif
