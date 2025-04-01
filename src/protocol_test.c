#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "game_manager.h"  // Se usa para invocar funciones de la lógica del juego

// Convierte un MessageType a string
const char* message_type_to_string(MessageType type) {
    if (type >= 0 && type < (int)(sizeof(MessageTypeStr)/sizeof(MessageTypeStr[0]))) {
        return MessageTypeStr[type];
    }
    return "UNKNOWN";
}

// Convierte un string a MessageType
MessageType string_to_message_type(const char* str) {
    for (int i = 0; i < (int)(sizeof(MessageTypeStr)/sizeof(MessageTypeStr[0])); i++) {
        if (strcmp(str, MessageTypeStr[i]) == 0) {
            return (MessageType)i;
        }
    }
    return MSG_INVALID;
}

// Función que maneja el mensaje recibido y llama a la función adecuada del game_manager
// Cada caso del switch representa un comando del protocolo.
void handle_message(ProtocolMessage* msg) {
    // Variables auxiliares para parseo de datos (si son necesarios)
    int x, y;
    char result[20];
    char email[50];
    char user0[20];
    char user1[20];
    
    switch (msg->type) {
        case MSG_LOGIN:
            // Caso de login: se espera que msg->data contenga el username.
            // Aquí se invocaría la función de autenticación, por ejemplo:
            // if (authenticate_user(msg->data)) { ... }
            printf("LOGIN recibido. Username: %s\n", msg->data);
            // En este ejemplo, simplemente se asume éxito.
            // La respuesta (ACK) se generará en el flujo de login en server_main.
            break;

        case MSG_ATTACK:
            // Parsear coordenadas de ataque: se espera "x,y"
            if (sscanf(msg->data, "%d,%d", &x, &y) != 2) {
                printf("Error al parsear ATTACK: datos inválidos (%s)\n", msg->data);
                break;
            }
            printf("ATTACK recibido: x=%d, y=%d\n", x, y);
            // Llamada al game_manager (comentada para ejemplo)
            // game_manager_attack(msg->game_id, x, y);
            break;

        case MSG_RESULT:
            // Se espera en msg->data: "x,y|result"
            if (sscanf(msg->data, "%d,%d|%19[^\n]", &x, &y, result) != 3) {
                printf("Error al parsear RESULT: datos inválidos (%s)\n", msg->data);
                break;
            }
            printf("RESULT recibido: x=%d, y=%d, result=%s\n", x, y, result);
            // Llamada al server (comentada para ejemplo)
            // server_result(msg->game_id, result, x, y);
            break;

        case MSG_CONNECT:
            // Por ejemplo, se puede usar para establecer una conexión o identificar al usuario.
            printf("CONNECT recibido: datos=%s\n", msg->data);
            if(sscanf(msg->data, "%99[^,],%99s", email, user0) != 2){
                printf("Error al parsear CONNECT: datos inválidos (%s)\n", msg->data);
            }

            // Ejemplo de llamada:
            // server_connect(msg->game_id, email, user0);
            break;

        case MSG_UPDATE:
            // Se espera en msg->data: "x,y|update_info"
            if (sscanf(msg->data, "%d,%d|%19[^\n]", &x, &y, result) != 3) {
                printf("Error al parsear UPDATE: datos inválidos (%s)\n", msg->data);
                break;
            }
            printf("UPDATE recibido: x=%d, y=%d, update=%s\n", x, y, result);
            // Ejemplo de llamada:
            // server_update(msg->game_id, result, x, y);
            break;

        case MSG_START:
            // Indica que se inicia la partida
            printf("GAME_START recibido para game_id %d\n", msg->game_id);
            // Ejemplo de llamada:
            // server_start(msg->game_id);
            break;

        case MSG_END:
            // Indica el final de la partida
            printf("GAME_END recibido para game_id %d\n", msg->game_id);
            // Ejemplo de llamada:
            // server_end(msg->game_id);
            break;

        case MSG_LOGGED:
            // Este caso lo utilizaría el servidor para enviar ACK de login,
            // por lo que normalmente no se procesa en el lado del servidor.
            printf("LOGGED recibido (ACK): %s\n", msg->data);
            break;

        default:
            printf("Mensaje desconocido recibido.\n");
            break;
    }
}

// Función que parsea un mensaje en formato "Tipo|game_id|datos" y, si es válido, lo procesa.
// Se asume que el input es una cadena terminada en '\n'.
void parse_and_handle_message(const char* input) {
    if (!input) return;

    ProtocolMessage msg;
    char type_str[50];
    char data_buffer[256];

    // Extraer las partes del mensaje:
    // Se espera el formato: "Tipo|game_id|datos"
    int scanned = sscanf(input, "%49[^|]|%d|%255[^\n]", type_str, &msg.game_id, data_buffer);
    if (scanned < 2) {
        printf("Error: Mensaje mal formado (%s).\n", input);
        return;
    }
    msg.type = string_to_message_type(type_str);
    if (msg.type == MSG_INVALID) {
        printf("Error: Tipo de mensaje desconocido (%s).\n", type_str);
        return;
    }
    if (scanned == 3) {
        strncpy(msg.data, data_buffer, sizeof(msg.data) - 1);
        msg.data[sizeof(msg.data) - 1] = '\0';
    } else {
        msg.data[0] = '\0';
    }
    // Procesa el mensaje, invocando la función correspondiente del game_manager.
    handle_message(&msg);
}

// Función para formatear un mensaje de protocolo en una cadena.
// El formato es: "Tipo|game_id|datos"
void format_message(ProtocolMessage msg, char* output, size_t size) {
    if (!output) return;
    snprintf(output, size, "%s|%d|%s", message_type_to_string(msg.type), msg.game_id, msg.data);
}
