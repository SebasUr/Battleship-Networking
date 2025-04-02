#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "session_handler.h"
#include "protocol.h"
#include "game_manager.h"

#define MAX 1024

// Supongamos que la sesión ahora procesa mensajes y, según el tipo, delega al game_handler.
void *session_handler(void *arg) {
    session_pair_t *session = (session_pair_t *)arg;
    int sock1 = session->client_sock1;
    int sock2 = session->client_sock2;
    int game_id = session->game_id;
    char username1[50], username2[50];
    strncpy(username1, session->username1, sizeof(username1));
    username1[sizeof(username1)-1] = '\0';
    strncpy(username2, session->username2, sizeof(username2));
    username2[sizeof(username2)-1] = '\0';
    free(session);
    
    char buff[MAX];
    int n;
    
    printf("Iniciando sesión de chat entre %s y %s en partida %d...\n", username1, username2, game_id);
    
    // Bucle principal de la sesión.
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        int max_sd = (sock1 > sock2) ? sock1 : sock2;
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) { perror("select error"); break; }
        
        // Procesar mensajes del primer cliente.
        if (FD_ISSET(sock1, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock1, buff, MAX);
            if (n <= 0) { printf("%s desconectado.\n", username1); break; }
            printf("%s envía: %s", username1, buff);
            
            // Si es un mensaje de ataque, procesarlo.
            ProtocolMessage msg;
            if (parse_message(buff, &msg) && msg.type == MSG_ATTACK) {
                // Se espera que msg.data contenga "enemyUsername|attackValue"
                char enemy[50];
                int attackValue;
                if (sscanf(msg.data, "%49[^|]|%d", enemy, &attackValue) != 2) {
                    printf("Error al parsear ATTACK de %s.\n", username1);
                } else {
                    // Llamar al game_manager para procesar el ataque.
                    char attackerResp[256], enemyResp[256];
                    game_manager_process_attack(game_id, username1, enemy, attackValue,
                                                attackerResp, sizeof(attackerResp),
                                                enemyResp, sizeof(enemyResp));
                    
                    // Dependiendo del valor de attackValue, decidimos qué comando enviar.
                    ProtocolMessage responseMsg;
                    char responseStr[MAX];
                    if (attackValue == 1) { // Ataque normal.
                        // Enviar RESULT al atacante.
                        responseMsg.type = MSG_RESULT;
                        responseMsg.game_id = game_id;
                        strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock1, responseStr, strlen(responseStr));
                        
                        // Enviar UPDATE al defensor.
                        responseMsg.type = MSG_UPDATE;
                        strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock2, responseStr, strlen(responseStr));
                    } else if (attackValue == 3) { // Ataque decisivo (fin del juego).
                        // Enviar END al ganador (atacante).
                        responseMsg.type = MSG_END;
                        responseMsg.game_id = game_id;
                        strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock1, responseStr, strlen(responseStr));
                        
                        // Enviar END al perdedor (defensor).
                        responseMsg.type = MSG_END;
                        strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock2, responseStr, strlen(responseStr));
                    }
                }
            } else {
                // Para otros mensajes, simplemente se reenvían (o se procesan según corresponda).
                write(sock2, buff, n);
            }
        }
        
        // Procesar mensajes del segundo cliente (similar).
        if (FD_ISSET(sock2, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock2, buff, MAX);
            if (n <= 0) { printf("%s desconectado.\n", username2); break; }
            printf("%s envía: %s", username2, buff);
            
            ProtocolMessage msg;
            if (parse_message(buff, &msg) && msg.type == MSG_ATTACK) {
                char enemy[50];
                int attackValue;
                if (sscanf(msg.data, "%49[^|]|%d", enemy, &attackValue) != 2) {
                    printf("Error al parsear ATTACK de %s.\n", username2);
                } else {
                    char attackerResp[256], enemyResp[256];
                    game_manager_process_attack(game_id, username2, enemy, attackValue,
                                                attackerResp, sizeof(attackerResp),
                                                enemyResp, sizeof(enemyResp));
                    ProtocolMessage responseMsg;
                    char responseStr[MAX];
                    if (attackValue == 1) {
                        responseMsg.type = MSG_RESULT;
                        responseMsg.game_id = game_id;
                        strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock2, responseStr, strlen(responseStr));
                        
                        responseMsg.type = MSG_UPDATE;
                        strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock1, responseStr, strlen(responseStr));
                    } else if (attackValue == 3) {
                        responseMsg.type = MSG_END;
                        responseMsg.game_id = game_id;
                        strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock2, responseStr, strlen(responseStr));
                        
                        responseMsg.type = MSG_END;
                        strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
                        responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                        format_message(responseMsg, responseStr, MAX);
                        write(sock1, responseStr, strlen(responseStr));
                    }
                }
            } else {
                write(sock1, buff, n);
            }
        }
    }
    
    close(sock1);
    close(sock2);
    printf("Sesión de chat finalizada entre %s y %s en partida %d.\n", username1, username2, game_id);
    return NULL;
}
