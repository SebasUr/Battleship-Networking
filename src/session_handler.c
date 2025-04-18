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

// Función para procesar mensajes de usuarios
void process_messages(GameManager *gm, int sock1, int sock2, int game_id, const char* username1, const char* username2, ProtocolMessage msg, int* end, FILE *log_file) {
    int x, y;
    *end = 0;
    if (sscanf(msg.data, "%d,%d", &x, &y) != 2) {
        printf("Error al parsear ATTACK de %s.\n", username1);
    } else {
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

            fprintf(log_file, "%s \"%s\" - %s\n", get_timestamp(), responseStr, username1);
            fflush(log_file);
            printf("%s \"%s\" - %s\n", get_timestamp(), responseStr, username1);
        }

        if (decision == 2) { // Ataque válido
            responseMsg.type = MSG_RESULT;
            responseMsg.game_id = game_id;
            strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
            responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
            format_message(responseMsg, responseStr, MAX);
            write(sock1, responseStr, strlen(responseStr));
            fprintf(log_file, "%s \"%s\" - SERVER\n", get_timestamp(), responseStr);
            fflush(log_file);
            printf("%s \"%s\" - SERVER\n", get_timestamp(), responseStr);

            responseMsg.type = MSG_UPDATE;
            strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
            responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
            format_message(responseMsg, responseStr, MAX);

            int retries = 0;
            while (retries < 2) { 
                write(sock2, responseStr, strlen(responseStr));
                fprintf(log_file, "%s \"%s\" - SERVER\n", get_timestamp(), responseStr);
                printf("%s \"%s\" - SERVER\n", get_timestamp(), responseStr);
                fflush(log_file);
    
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(sock2, &rfds);
                struct timeval tmo = { .tv_sec = 3, .tv_usec = 0 };
                int sel = select(sock2 + 1, &rfds, NULL, NULL, &tmo);
                if (sel > 0 && FD_ISSET(sock2, &rfds)) {
                    char ackbuf[MAX];
                    int len = read(sock2, ackbuf, sizeof(ackbuf));
                    if (len > 0) {
                        ProtocolMessage ackmsg;
                        if (parse_message(ackbuf, &ackmsg) && ackmsg.type == MSG_ACK && ackmsg.game_id == game_id && strcmp(ackmsg.data, "1") == 0){
                            fprintf(log_file, "%s \"ACK RECEIVED/ UPDATE SUCCESSFUL \" - %s\n", get_timestamp(), username2);
                            printf(   "%s \"ACK RECEIVED/ UPDATE SUCCESSFUL\" - %s\n", get_timestamp(), username2);
                            fflush(log_file);
                            break;
                        }
                    }
                }
                retries++;
            }
        }

        if (decision == 3) { // Ataque decisivo (fin del juego)
            responseMsg.type = MSG_END;
            responseMsg.game_id = game_id;
            strncpy(responseMsg.data, attackerResp, sizeof(responseMsg.data)-1);
            responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
            format_message(responseMsg, responseStr, MAX);
            write(sock1, responseStr, strlen(responseStr));
            fprintf(log_file, "%s \"%s\" - SERVER\n", get_timestamp(), responseStr);
            fflush(log_file);
            printf("%s \"%s\" - SERVER\n", get_timestamp(), responseStr);

            responseMsg.type = MSG_END;
            strncpy(responseMsg.data, enemyResp, sizeof(responseMsg.data)-1);
            responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
            format_message(responseMsg, responseStr, MAX);
            write(sock2, responseStr, strlen(responseStr));
            fprintf(log_file, "%s \"%s\" - SERVER\n", get_timestamp(), responseStr);
            fflush(log_file);
            printf("%s \"%s\" - SERVER\n", get_timestamp(), responseStr);
            *end = 1;
        }
    }
}

// Función de manejo de sesión: procesa mensajes entre ambos clientes.
void *session_handler(void *arg) {
    session_pair_t *session = (session_pair_t *)arg;
    FILE *log_file = session->log_file;
    int sock1 = session->client_sock1, sock2 = session->client_sock2;
    int game_id = session->game_id;
    char username1[50], username2[50];
    strncpy(username1, session->username1, sizeof(username1));
    username1[sizeof(username1)-1] = '\0';
    strncpy(username2, session->username2, sizeof(username2));
    username2[sizeof(username2)-1] = '\0';
    GameManager *gm = session->gm;
    rooms **rooms_list = session->room;
    free(session);

    ProtocolMessage ackMsg;
    ackMsg.type = MSG_LOGGED;
    ackMsg.game_id = game_id;
    int turn1, turn2;
    char initial_info1[100], initial_info2[100];
    char buf[MAX];

    // Procesar login para el primer usuario
    game_manager_process_login(gm, game_id, username1, initial_info1, sizeof(initial_info1), &turn1);
    turn1 = 1;
    snprintf(buf, sizeof(buf), "Ok|%d|%s", turn1, initial_info1);
    strncpy(ackMsg.data, buf, sizeof(ackMsg.data)-1);
    ackMsg.data[sizeof(ackMsg.data)-1] = '\0';
    format_message(ackMsg, buf, MAX);
    write(sock1, buf, strlen(buf));
    fprintf(log_file, "%s \"%s\" - SERVER\n", get_timestamp(), buf);
    fflush(log_file);
    printf("%s \"%s\" - SERVER\n", get_timestamp(), buf);

    // Procesar login para el segundo usuario
    game_manager_process_login(gm, game_id, username2, initial_info2, sizeof(initial_info2), &turn2);
    turn2 = 0;
    snprintf(buf, sizeof(buf), "Ok|%d|%s", turn2, initial_info2);
    strncpy(ackMsg.data, buf, sizeof(ackMsg.data)-1);
    ackMsg.data[sizeof(ackMsg.data)-1] = '\0';
    format_message(ackMsg, buf, MAX);
    write(sock2, buf, strlen(buf));
    fprintf(log_file, "%s \"Iniciando sesión de chat entre %s y %s en partida %d...\" - SERVER\n", get_timestamp(), username1, username2, game_id);
    fflush(log_file);
    printf("%s \"Iniciando sesión de chat entre %s y %s en partida %d...\" - SERVER\n", get_timestamp(), username1, username2, game_id);

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

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0) { perror("select error"); break; }
        if (activity == 0) {
            fprintf(log_file, "%s \"TIMEOUT: CAMBIANDO TURNOS.\" - SERVER\n", get_timestamp());
            fflush(log_file);
            printf("%s \"TIMEOUT: CAMBIANDO TURNOS.\" - SERVER\n", get_timestamp());

            ProtocolMessage responseMsg;
            char responseStr[MAX];
            responseMsg.type = MSG_TIMEOUT;
            responseMsg.game_id = game_id;
            strncpy(responseMsg.data, "-1", sizeof(responseMsg.data)-1);
            responseMsg.data[sizeof(responseMsg.data)-1] = '\0';
            format_message(responseMsg, responseStr, MAX);
            write(sock1, responseStr, strlen(responseStr));
            write(sock2, responseStr, strlen(responseStr));
            continue;
        }

        if (FD_ISSET(sock1, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock1, buff, MAX);
            if (n <= 0) { printf("%s desconectado.\n", username1); break; }
            fprintf(log_file, "%s \"%s\" - %s\n", get_timestamp(), buff, username1);
            fflush(log_file);
            printf("%s \"%s\" - %s\n", get_timestamp(), buff, username1);

            ProtocolMessage msg;
            if (parse_message(buff, &msg) && msg.type == MSG_ATTACK) {
                process_messages(gm, sock1, sock2, game_id, username1, username2, msg, &end, log_file);
                if (end == 1) { search_room(rooms_list, game_id, true); break; }
            } else if (parse_message(buff, &msg) && msg.type == MSG_FF) {
                ProtocolMessage responseMsg;
                char responseStr[MAX];
                responseMsg.type = MSG_END; responseMsg.game_id = game_id;
                snprintf(responseMsg.data, sizeof(responseMsg.data), "FF|%s", username2);
                format_message(responseMsg, responseStr, MAX);

                fprintf(log_file, "%s \"%s se rinde ante %s\" - %s\n", get_timestamp(), username1, username2, username1);
                write(sock1, responseStr, strlen(responseStr));
                write(sock2, responseStr, strlen(responseStr));
                fflush(log_file);
                end = 1;
                search_room(rooms_list, game_id, true);
            } else {
                write(sock2, buff, n);
            }
        }

        if (FD_ISSET(sock2, &readfds)) {
            memset(buff, 0, MAX);
            n = read(sock2, buff, MAX);
            if (n <= 0) { printf("%s desconectado.\n", username2); break; }
            fprintf(log_file, "%s \"%s\" - %s\n", get_timestamp(), buff, username2);
            fflush(log_file);
            printf("%s \"%s\" - %s\n", get_timestamp(), buff, username2);

            ProtocolMessage msg;
            if (parse_message(buff, &msg) && msg.type == MSG_ATTACK) {
                process_messages(gm, sock2, sock1, game_id, username2, username1, msg, &end, log_file);
                if (end == 1) { search_room(rooms_list, game_id, true); break; }
            } else if (parse_message(buff, &msg) && msg.type == MSG_FF) {
                ProtocolMessage responseMsg;
                char responseStr[MAX];
                responseMsg.type = MSG_END; responseMsg.game_id = game_id;
                snprintf(responseMsg.data, sizeof(responseMsg.data), "FF|%s", username1);
                format_message(responseMsg, responseStr, MAX);

                fprintf(log_file, "%s \"%s se rinde ante %s\" - %s\n", get_timestamp(), username2, username1, username2);
                write(sock1, responseStr, strlen(responseStr));
                write(sock2, responseStr, strlen(responseStr));
                fflush(log_file);
                end = 1;
                search_room(rooms_list, game_id, true);
            } else {
                write(sock1, buff, n);
            }
        }
    }

    close(sock1);
    close(sock2);
    fprintf(log_file, "%s \"Sesión de chat finalizada entre %s y %s en partida %d.\" - SERVER\n", get_timestamp(), username1, username2, game_id);
    fflush(log_file);
    return NULL;
}
