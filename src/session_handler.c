#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "session_handler.h"
#include "protocol.h"
#include "game_manager.h"
#include "utils.h"

#define MAX 1024


//Función para procesar mensajes de usuarios
void process_messages(GameManager *gm, int sock1, int sock2, int game_id, const char* username1, const char* username2,  ProtocolMessage msg, int* end, FILE *log_file){
        char message[1500];
        // Se espera que msg.data contenga dos enteros "x,y"
        int x, y;
        *end = 0;
        if (sscanf(msg.data, "%d,%d", &x, &y) != 2) {
            printf("Error al parsear ATTACK de %s.\n", username1);
        } else {
            // Llamar al game_manager para procesar el ataque.
            char attackerResp[256], enemyResp[256];
            int decision;
            game_manager_process_attack(gm, game_id, username1, username2, x, y,
                                        attackerResp, sizeof(attackerResp),
                                        enemyResp, sizeof(enemyResp),
                                        &decision);
                    
            ProtocolMessage responseMsg;
            char responseStr[MAX];

            if (decision == 0 || decision == 1) { // Caso: Error (OutOfBounds o Ataque duplicado)
                responseMsg.type = MSG_ERROR;
                responseMsg.game_id = game_id;
                strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);
                write(sock1, responseStr, strlen(responseStr));

                snprintf(message, sizeof(message), "%s", responseStr);
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
            }

            if (decision == 2) { // Ataque válido
                responseMsg.type = MSG_RESULT;
                responseMsg.game_id = game_id;
                strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);
                write(sock1, responseStr, strlen(responseStr));
                snprintf(message, sizeof(message), "%s", responseStr);
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
                        
                // Enviar UPDATE al defensor.
                responseMsg.type = MSG_UPDATE;
                strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
                responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);
                write(sock2, responseStr, strlen(responseStr));
                snprintf(message, sizeof(message), "%s", responseStr);
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
            }

            if (decision == 3) { // Ataque decisivo (fin del juego)
                responseMsg.type = MSG_END;
                responseMsg.game_id = game_id;
                strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
                responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);
                write(sock1, responseStr, strlen(responseStr));
                snprintf(message, sizeof(message), "%s", responseStr);
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
                        
                responseMsg.type = MSG_END;
                strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
                responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);
                write(sock2, responseStr, strlen(responseStr));
                snprintf(message, sizeof(message), "%s", responseStr);
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
                *end = 1;
            }
        }
}

// Función de manejo de sesión: recibe mensajes de ambos clientes y, si se trata de un ataque, llama al game_manager.
void *session_handler(void *arg) {
    char message[1500];
    session_pair_t *session = (session_pair_t *)arg;
    FILE *log_file = session->log_file;
    int sock1 = session->client_sock1; int sock2 = session->client_sock2;
    int game_id = session->game_id;
    char username1[50]; char username2[50];
    strncpy(username1, session->username1, sizeof(username1));
    username1[sizeof(username1)-1] = '\0';
    strncpy(username2, session->username2, sizeof(username2));
    username2[sizeof(username2)-1] = '\0';
    GameManager *gm = session->gm;
    rooms **rooms_list = session->room;
    free(session);

    // Preparar el ACK usando el protocolo para tener el prefijo "LOGGED"
    // El ACK tendrá el formato: "LOGGED|MatchID|Ok|<turn>|<initial_info>   

    ProtocolMessage ackMsg;
    ackMsg.type = MSG_LOGGED;
    ackMsg.game_id = game_id;
    int turn1; int turn2;
    char initial_info1[100]; char initial_info2[100];
    char buf[MAX];

    game_manager_process_login(gm, game_id, username1, initial_info1, sizeof(initial_info1), &turn1);
    turn1 = 1;
    snprintf(buf,sizeof(buf),"Ok|%d|%s",turn1,initial_info1);
    strncpy(ackMsg.data,buf,sizeof(ackMsg.data)-1); ackMsg.data[sizeof(ackMsg.data)-1]=0;
    format_message(ackMsg,buf,MAX); write(sock1,buf,strlen(buf));
    snprintf(message, sizeof(message), "Enviando a %s: %s\n",username1,buf);
    fprintf(log_file, "%s\n", message);
    fflush(log_file);
    printf("%s", message);

    game_manager_process_login(gm, game_id, username2, initial_info2, sizeof(initial_info2), &turn2);
    turn2 = 0;
    snprintf(buf,sizeof(buf),"Ok|%d|%s",turn2,initial_info2);
    strncpy(ackMsg.data,buf,sizeof(ackMsg.data)-1); ackMsg.data[sizeof(ackMsg.data)-1]=0;
    format_message(ackMsg,buf,MAX); write(sock2,buf,strlen(buf));
    snprintf(message, sizeof(message), "Iniciando sesión de chat entre %s y %s en partida %d...\n", username1, username2, game_id);
    fprintf(log_file, "%s\n", message);
    fflush(log_file);
    printf("%s", message);
    
    // Bucle principal de la sesión.
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        char buff[MAX];
        int n;
        int max_sd = (sock1 > sock2) ? sock1 : sock2;
        struct timeval timeout;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        int end = 0;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout); //Se añade el timeout de 30 segundos
        
        if (activity < 0) { perror("select error"); break; }
        if (activity == 0) {
            snprintf(message, sizeof(message), "Timeout: ningún mensaje recibido en 30 segundos. Cambiando turno automáticamente.\n");
            fprintf(log_file, "%s\n", message);
            fflush(log_file);
            printf("%s", message);

            char attackerResp[10], enemyResp[10];
            snprintf(attackerResp, sizeof(attackerResp), "-1");
            snprintf(enemyResp, sizeof(enemyResp), "-1");
            ProtocolMessage responseMsg;
            char responseStr[MAX];
            

            responseMsg.type = MSG_TIMEOUT;
            responseMsg.game_id = game_id;
            strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
            responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
            format_message(responseMsg, responseStr, MAX);
            write(sock1, responseStr, strlen(responseStr));
            write(sock2, responseStr, strlen(responseStr));
            continue;
        }
        // Procesar mensajes del primer cliente.
        if (FD_ISSET(sock1, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock1, buff, MAX);
            if (n <= 0) { printf("%s desconectado.\n", username1); break; }
            snprintf(message, sizeof(message), "%s envía: %s", username1, buff);
            fprintf(log_file, "%s\n", message);
            fflush(log_file);
            printf("%s", message);
            
            ProtocolMessage msg;
            if (parse_message(buff, &msg) && msg.type == MSG_ATTACK) {
                process_messages(gm, sock1, sock2, game_id, username1, username2, msg, &end, log_file);   
                if(end==1){
                    
                    search_room(rooms_list, game_id, true);
                    break;
                }
            } else if(parse_message(buff, &msg) && msg.type == MSG_FF) {
                ProtocolMessage responseMsg;
                char responseStr[MAX];

                responseMsg.type = MSG_END; responseMsg.game_id = game_id;
                char winner[MAX]; snprintf(winner, sizeof(winner), "%s|", username2);
                strncpy(responseMsg.data, winner, sizeof(responseMsg.data)-1); responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);

                snprintf(message, sizeof(message), "%s Se rinde ante %s", username1, username2);

                write(sock1, responseStr, strlen(responseStr));
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
                        
                write(sock2, responseStr, strlen(responseStr));
                fprintf(log_file, "%s\n", message);
                fflush(log_file);

                end = 1;
                search_room(rooms_list, game_id, true);
            } else{
                write(sock2, buff, n);
            } 
        }
        
        // Procesar mensajes del segundo cliente (lógica similar, roles invertidos).
        if (FD_ISSET(sock2, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock2, buff, MAX);
            if (n <= 0) { printf("%s desconectado.\n", username2); break; }
            snprintf(message, sizeof(message), "%s envía: %s", username2, buff);
            fprintf(log_file, "%s\n", message);
            fflush(log_file);
            printf("%s", message);
            
            ProtocolMessage msg;
            if (parse_message(buff, &msg) && msg.type == MSG_ATTACK) {
                process_messages(gm, sock2, sock1, game_id, username2, username1, msg, &end, log_file);
                if(end==1){
                    
                    search_room(rooms_list, game_id, true);
                    break;
                }
            } else if(parse_message(buff, &msg) && msg.type == MSG_FF) {
                ProtocolMessage responseMsg;
                char responseStr[MAX];

                responseMsg.type = MSG_END; responseMsg.game_id = game_id;
                char winner[MAX]; snprintf(winner, sizeof(winner), "%s|", username2);
                strncpy(responseMsg.data, winner, sizeof(responseMsg.data)-1); responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
                format_message(responseMsg, responseStr, MAX);

                snprintf(message, sizeof(message), "%s Se rinde ante %s", username2, username1);

                write(sock1, responseStr, strlen(responseStr));
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
                        
                write(sock2, responseStr, strlen(responseStr));
                fprintf(log_file, "%s\n", message);
                fflush(log_file);
                
                end = 1;
                search_room(rooms_list, game_id, true);
            } else{
                write(sock1, buff, n);
            } 
        }
    }
    
    close(sock1);
    close(sock2);
    snprintf(message, sizeof(message), "Sesión de chat finalizada entre %s y %s en partida %d.\n", username1, username2, game_id);
    search_room(rooms_list, game_id, true);   
    fprintf(log_file, "%s\n", message);
    fflush(log_file);
    printf("%s", message);
    return NULL;
}
