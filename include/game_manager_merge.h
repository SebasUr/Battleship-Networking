#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "protocol.h"

typedef struct {
    char grid[FILAS][COLUMNAS];
    int numero_barcos;
} board;


// Estructura para seguir el estado de la partida de ambos usuarios
typedef struct {
    char *user;
    board board;
} game_manager;


// Definición de los tipos de barcos
typedef struct {
    int tamano;
    int cantidad;
    char nombre[20];
} TipoBarco;

// Lista de barcos con sus tamaños y cantidades
TipoBarco barcos[] = {
    {5, 1, "Portaaviones"},
    {4, 1, "Buque de guerra"},
    {3, 2, "Crucero"},
    {2, 2, "Destructor"},
    {1, 3, "Submarino"}
};

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

int game_manager_process_attack(int game_id, const char* attacker, const char* enemy, int attackValue,
                                char* attackerResponse, size_t attackerResponseSize,
                                char* enemyResponse, size_t enemyResponseSize);


void inicializarTablero(tablero *tablero);

// Ubica aleatoriamente los barcos del tablero.
void generarTablero(tablero *tablero);

// Imprime el tablero.
void imprimirTablero(tablero *tablero);

// Obtiene el tablero de un usuario
GameManager* obtenerTablero(char *username);

#endif // GAME_MANAGER_H
