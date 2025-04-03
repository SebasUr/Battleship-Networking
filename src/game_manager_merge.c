#include "game_manager.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define VACIO '.'
#define BARCO 'B'
#define ATAQUE 'X'

static char user_aux[20];
static tablero aux_value;
static bool first_user_saved = false;
static game_manager *manager = NULL;

// Stub para el proceso de login.
int game_manager_process_login(int game_id, const char* username, char* initial_info, size_t initial_info_size,int* turn) {
        // Asignamos turno = 1 para el primer cliente (la lógica real debería basarse en el orden de conexión)
        *turn = 1;
        // Información inicial 
        if (first_user_saved==false) {
            // Guardamos temporalmente en variables auxiliares
            strcpy user_aux, username);
            aux_value = generarTablero();
            first_user_saved = true;
            // Pasar el tablero a string
            char initial_info[100];
            
            
            snprintf(initial_info, initial_info_size, "1,2,3");
        } else {
            if (!first_user_saved) {
                printf("Error: El primer usuario no ha sido guardado.\n");
                return;
            }
    
            // Crear el diccionario con memoria dinámica
            manager = (game_manager *)malloc(2 * sizeof(game_manager));
            if (!manager) {
                printf("Error de asignación de memoria.\n");
                return;
            }
    
            // Insertar el primer elemento almacenado
            strcpy(manager[0].user, user_aux);
            manager[0].board = aux_value;
    
            // Insertar el segundo elemento recibido
            strcpy(manager[1].user, username);
            manager[1].board = generarTablero();
            
        }

        
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


// Inicializar el tablero vacío
void inicializarTablero() {
    tablero tablero;
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            tablero->grid[i][j] = VACIO;
        }
    }
}

// Verificar si un barco se puede colocar en una posición
int puedeColocar(tablero *tablero, int x, int y, int tamano, char orientacion) {
    if (orientacion == 'H') {
        if (y + tamano > COLUMNAS) return 0;
        for (int j = 0; j < tamano; j++) {
            if (tablero->grid[x][y + j] != VACIO) return 0;
        }
    } else {
        if (x + tamano > FILAS) return 0;
        for (int i = 0; i < tamano; i++) {
            if (tablero->grid[x + i][y] != VACIO) return 0;
        }
    }
    return 1;
}

// Colocar un barco en el tablero
void colocarBarco(tablero *tablero, int tamano) {
    int x, y;
    char orientacion;
    do {
        x = rand() % FILAS;
        y = rand() % COLUMNAS;
        orientacion = (rand() % 2) ? 'H' : 'V';
    } while (!puedeColocar(tablero, x, y, tamano, orientacion));

    // Colocar barco
    for (int i = 0; i < tamano; i++) {
        if (orientacion == 'H') {
            tablero->numero_barcos +=1;
            tablero->grid[x][y + i] = BARCO;
        } else {
            tablero->numero_barcos +=1;
            tablero->grid[x + i][y] = BARCO;
        }
    }
}

// Generar tablero con barcos colocados aleatoriamente
void generarTablero() {
    inicializarTablero();
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < barcos[i].cantidad; j++) {
            colocarBarco(tablero, barcos[i].tamano);
        }
    }
}

// Imprimir el tablero en consola
void imprimirTablero(tablero *tablero) {
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            printf("%c ", tablero->grid[i][j]);
        }
        printf("\n");
    }