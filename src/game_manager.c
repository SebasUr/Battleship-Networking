#include "game_manager.h"
#include <stdio.h>

int boundx = 100;
int boundy = 100;

void game_manager_attack(int game_id, int x, int y) {
    char msg_error[300];
    printf("game_manager_attack: game_id=%d, x=%d, y=%d\n", game_id, x, y);
    if (x < 0 || y < 0 || x > boundx || y > boundy){
        //Enviar mensaje ERROR|id|"Invalid move: out of bounds"
        parse_and_handle_message(msg_error);
    }
    //Condiciòn para ataque duplicado

    //Verificar si acertò o no

    //Definir mensajes de result y update
    //Verificar si CONTINUE o WIN
    char msg_result[300];
    char msg_update[300];
    
    parse_and_handle_message(msg_result);
    parse_and_handle_update(msg_result);
}

void server_result(int game_id, const char* result, int x, int y) {
    printf("server_result: game_id=%d, x=%d, y=%d, result=%s\n", game_id, x, y, result);
}

void server_connect(int game_id, const char* data) {
    printf("server_connect: game_id=%d, data=%s\n", game_id, data);
}

void server_update(int game_id, const char* data) {
    printf("server_update: game_id=%d, data=%s\n", game_id, data);
}

void server_start(int game_id) {
    printf("server_start: game_id=%d\n", game_id);
}

void server_end(GameManager* game_manager, int game_id) {
    // Aquí se podría modificar el estado del game_manager si es necesario.
    printf("server_end: game_id=%d\n", game_id);
}
