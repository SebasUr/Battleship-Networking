#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

const char* message_type_to_string(MessageType type) {
    const char* MessageTypeStr[] = {
        "LOGIN",
        "LOGGED",
        "ATTACK",
        "RESULT",
        "UPDATE",
        "GAME_END",
        "ERROR",
        "INVALID",
        "TIMEOUT"
        // "GAME_START"  // Opcional
    };
    if (type >= 0 && type < (int)(sizeof(MessageTypeStr)/sizeof(MessageTypeStr[0])))
        return MessageTypeStr[type];
    return "INVALID";
}

MessageType string_to_message_type(const char* str) {
    const char* MessageTypeStr[] = {
        "LOGIN",
        "LOGGED",
        "ATTACK",
        "RESULT",
        "UPDATE",
        "GAME_END",
        "ERROR",
        "INVALID",
        "TIMEOUT"
        // "GAME_START"  // Opcional.
    };

    for (int i = 0; i < (int)(sizeof(MessageTypeStr)/sizeof(MessageTypeStr[0])); i++) {
        if (strcmp(str, MessageTypeStr[i]) == 0)
            return (MessageType)i;
    }
    return MSG_INVALID;
}

// Parsea un mensaje en formato "COMANDO|game_id|datos"
bool parse_message(const char* input, ProtocolMessage* msg) {
    if (!input || !msg) return false;
    char type_str[50];
    char data_buffer[256];
    int scanned = sscanf(input, "%49[^|]|%d|%255[^\n]", type_str, &msg->game_id, data_buffer);
    if (scanned < 2) return false;
    msg->type = string_to_message_type(type_str);
    if (scanned == 3) {
        strncpy(msg->data, data_buffer, sizeof(msg->data) - 1);
        msg->data[sizeof(msg->data) - 1] = '\0';
    } else {
        msg->data[0] = '\0';
    }
    return true;
}

// Formatea un mensaje en "COMANDO|game_id|datos"
void format_message(ProtocolMessage msg, char* output, size_t size) {
    if (!output) return;
    snprintf(output, size, "%s|%d|%s", message_type_to_string(msg.type), msg.game_id, msg.data);
}
