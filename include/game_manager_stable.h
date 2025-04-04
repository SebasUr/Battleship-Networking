#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "protocol.h"

// Procesa el login. Recibe:
// - game_id: ID de la partida.
// - username: El nombre de usuario.
// - initial_info: Buffer para almacenar la información inicial (por ejemplo, "1,2,3").
// - initial_info_size: Tamaño del buffer.
// - turn: Salida para indicar el turno asignado (1 para el primero, 0 para el segundo).
// La función retorna 0 si todo sale bien o un código de error.
int game_manager_process_login(int game_id, const char* username,
                               char* initial_info, size_t initial_info_size,
                               int* turn);

// Procesa un ataque. (Ya definido en versiones anteriores)
int game_manager_process_attack(int game_id, const char* attacker, const char* enemy, int attackValue,
                                char* attackerResponse, size_t attackerResponseSize,
                                char* enemyResponse, size_t enemyResponseSize);

#endif // GAME_MANAGER_H
