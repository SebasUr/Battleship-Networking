#include "game_manager.h"
#include <stdio.h>
#include <string.h>

// Stub para el proceso de login.
int game_manager_process_login(int game_id, const char* username,
    char* initial_info, size_t initial_info_size,
    int* turn) {
// Asignamos turno = 1 para el primer cliente (la lógica real debería basarse en el orden de conexión)
*turn = 1;
// Información inicial (puedes extenderla según sea necesario)
snprintf(initial_info, initial_info_size, "1,2,3");
return 0;
}

int game_manager_process_attack(int game_id, const char* attacker, const char* enemy, int attackValue,
                                char* attackerResponse, size_t attackerResponseSize,
                                char* enemyResponse, size_t enemyResponseSize) {
    if (attackValue == 1) { 
        snprintf(attackerResponse, attackerResponseSize, "%s|0", attacker);
        snprintf(enemyResponse, enemyResponseSize, "%s|1", enemy);
    } else if (attackValue == 3) {
        snprintf(attackerResponse, attackerResponseSize, "%s|1", attacker);
        snprintf(enemyResponse, enemyResponseSize, "%s|0", enemy);
    } else {
        snprintf(attackerResponse, attackerResponseSize, "Error|0");
        snprintf(enemyResponse, enemyResponseSize, "Error|0");
    }
    return 2;
}
