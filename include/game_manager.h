#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#define ROWS 10
#define COLUMNS 10

#include "protocol.h"

// Definición del tipo board (tablero)
typedef struct {
    char grid[ROWS][COLUMNS];
    int num_ships;
} Board;

// Estructura para mantener el estado de la partida para ambos usuarios
typedef struct {
    char user[50];
    Board board;
} GameState;

// Declaración del game_manager; este se usará para guardar el estado de la partida.
// Por simplicidad, se usa un arreglo de 2 GameState.
typedef struct {
    GameState *states; // Se reserva memoria para 2 estados
} GameManager;

// --- Funciones del GameManager ---

// Procesa el login. Recibe:
// - game_id: ID de la partida.
// - username: El nombre de usuario.
// - initial_info: Buffer para almacenar la información inicial (por ejemplo, el tablero convertido a string).
// - initial_info_size: Tamaño del buffer.
// - turn: Salida para indicar el turno asignado (1 para el primero, 0 para el segundo).
// La función retorna 0 si todo sale bien o un código de error.
int game_manager_process_login(GameManager *gm, int game_id, const char* username,
                               char* initial_info, size_t initial_info_size,
                               int* turn);

// Procesa un ataque. (Ya definido en versiones anteriores)
void game_manager_process_attack(GameManager *gm, int game_id, const char* attacker, const char* enemy, int x, int y,
                                char* attackerResponse, size_t attackerResponseSize,
                                char* enemyResponse, size_t enemyResponseSize, 
                                int* decision);

// Funciones para el manejo del tablero:

// Inicializa el board (tablero) vacío.
void initializeBoard(Board *board);

// Verifica si se puede colocar un barco en una posición del tablero.
// Retorna 1 si es posible, 0 en caso contrario.
int canPlace(Board *board, int x, int y, int size, char orientation);

// Coloca un barco en el tablero.
void placeShip(Board *board, int size, char symbol);

// Genera un tablero con barcos colocados aleatoriamente.
Board generateBoard();

// Convierte el tablero a una cadena de caracteres.
// Se almacena el resultado en 'buffer', con tamaño 'buffer_size'.
void boardToString(Board board, char *buffer, int buffer_size);

// Imprime el tablero en la consola.
void printBoard(Board board);

// (Opcional) Obtiene el estado de la partida para un usuario (para extender en el futuro).
GameState* getBoard(GameManager *gm, const char *username);

#endif // GAME_MANAGER_H
