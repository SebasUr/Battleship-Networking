#include "game_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Constantes para definir el tablero.
#define EMPTY '.'
#define HIT 'X'

// Variables estáticas para guardar información temporal de login.
static char user_aux[20];
static Board aux_board;
static int first_user_saved = 0;  // 0 = false, 1 = true.
static GameManager *manager = NULL;

/*
 * Función: game_manager_process_login
 * ----------------------------
 * Procesa el login de un usuario.
 *
 * Se utiliza para asignar el turno y generar la información inicial.
 * - Si es el primer usuario que se conecta para ese MatchID, se guarda su username y se genera un tablero.
 * - Si es el segundo, se crea el GameManager con dos estados.
 *
 * Parámetros:
 *   game_id: ID de la partida.
 *   username: Nombre del usuario que se conecta.
 *   initial_info: Buffer donde se almacenará el tablero convertido a string.
 *   initial_info_size: Tamaño del buffer.
 *   turn: Salida para asignar el turno (1 para el primer cliente, 0 para el segundo).
 *
 * Retorna: 0 si todo sale bien.
 */
int game_manager_process_login(int game_id, const char* username,
                               char* initial_info, size_t initial_info_size,
                               int* turn) {
    // Si es el primer usuario que se conecta, se guarda su información.
    if (first_user_saved == 0) {
        *turn = 1; // Primer usuario tiene turno 1.
        strcpy(user_aux, username);
        aux_board = generateBoard();  // Generamos un tablero aleatorio.
        first_user_saved = 1;
        
        // Convertimos el tablero a string.
        char board_str[1000];
        boardToString(aux_board, board_str, sizeof(board_str));
        snprintf(initial_info, initial_info_size, "%s", board_str);
        printBoard(aux_board);  // Imprimimos el tablero para verificar.
    } else {
        // Creamos el GameManager con memoria dinámica para 2 estados.
        manager = (GameManager *)malloc(sizeof(GameManager));
        if (!manager) {
            printf("Error de asignación de memoria.\n");
            return -1;
        }
        manager->states = (GameState *)malloc(2 * sizeof(GameState));
        if (!manager->states) {
            printf("Error de asignación de memoria para estados.\n");
            free(manager);
            return -1;
        }
        
        // Insertamos el primer estado con la información guardada.
        strcpy(manager->states[0].user, user_aux);
        manager->states[0].board = aux_board;
        
        // Insertamos el segundo estado con el nuevo usuario.
        strcpy(manager->states[1].user, username);
        // Para este ejemplo, generamos otro tablero aleatorio para el segundo jugador.
        manager->states[1].board = generateBoard();
        
        // Convertimos el tablero del segundo jugador a string para enviarlo.
        char board_str[1000];
        boardToString(manager->states[1].board, board_str, sizeof(board_str));
        printBoard(manager->states[1].board);  // Imprimimos el tablero para verificar.
        snprintf(initial_info, initial_info_size, "%s", board_str);
    }
    
    return 0;
}

// 0,1 Corresponde a error por Bounds y Ataque Duplicado
// 1 Corresponde a ataque existoso
void game_manager_process_attack(int game_id, const char* attacker, const char* enemy, int x, int y,
    char* attackerResponse, size_t attackerResponseSize,
    char* enemyResponse, size_t enemyResponseSize, 
    int* decision) {
    
    char msg_error[50];
    char result[10];
    
    if (x < 0 || y < 0 || x >= ROWS || y >= COLUMNS) {
        snprintf(attackerResponse, attackerResponseSize, "OOB|1");
        *decision = 0;
    } else {
        GameState* state = getBoard(enemy);
        if (state == NULL) {
            snprintf(attackerResponse, attackerResponseSize, "Error|NoBoard");
            *decision = 0;
            return;
        }
        Board *board = &state->board;
        
        
        if (board->grid[x][y] == HIT) {
            snprintf(msg_error, sizeof(msg_error), "DA|1");
            strncpy(attackerResponse, msg_error, attackerResponseSize-1);
            attackerResponse[attackerResponseSize-1] = '\0';
            *decision = 1;
        } else {
            if (board->grid[x][y] != HIT && board->grid[x][y] != EMPTY) {
                strncpy(result, "HIT", sizeof(result));
                result[sizeof(result)-1] = '\0';
                board->grid[x][y] = HIT;
                board->num_ships -= 1;
            } else {
                strncpy(result, "NOHIT", sizeof(result));
                result[sizeof(result)-1] = '\0';
            }
            
            if (board->num_ships == 0) {
                snprintf(attackerResponse, attackerResponseSize, "%s|%s", result, attacker);
                snprintf(enemyResponse, enemyResponseSize, "%s|%s", result, attacker);
                *decision = 3;
            } else {
                snprintf(attackerResponse, attackerResponseSize, "%s|0", result);
                snprintf(enemyResponse, enemyResponseSize, "%s|1", result);
                *decision = 2;
            }
        }
        printBoard(*board);
    }

      // Imprimimos el tablero para verificar.
}

/*
 * Función: initializeBoard
 * ----------------------------
 * Inicializa un tablero vacío asignando el carácter EMPTY a cada posición.
 */
void initializeBoard(Board *board) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            board->grid[i][j] = EMPTY;
        }
    }
    board->num_ships = 0;
}

/*
 * Función: canPlace
 * ----------------------------
 * Verifica si se puede colocar un barco en el tablero en la posición (x, y) con un tamaño dado y una orientación.
 * La orientación es 'H' para horizontal y 'V' para vertical.
 * Retorna 1 si se puede colocar, 0 en caso contrario.
 */
int canPlace(Board *board, int x, int y, int size, char orientation) {
    if (orientation == 'H') {
        if (y + size > COLUMNS) return 0;
        for (int j = 0; j < size; j++) {
            if (board->grid[x][y + j] != EMPTY) return 0;
        }
    } else { // Vertical
        if (x + size > ROWS) return 0;
        for (int i = 0; i < size; i++) {
            if (board->grid[x + i][y] != EMPTY) return 0;
        }
    }
    return 1;
}

/*
 * Función: placeShip
 * ----------------------------
 * Coloca un barco en el tablero con el tamaño indicado y usando el símbolo dado.
 * Selecciona aleatoriamente la posición y la orientación, verificando que se pueda colocar.
 */
void placeShip(Board *board, int size, char symbol) {
    int x, y;
    char orientation;
    do {
        x = rand() % ROWS;
        y = rand() % COLUMNS;
        orientation = (rand() % 2) ? 'H' : 'V';
    } while (!canPlace(board, x, y, size, orientation));
    
    for (int i = 0; i < size; i++) {
        if (orientation == 'H') {
            board->grid[x][y + i] = symbol;
            board->num_ships++;
        } else {
            board->grid[x + i][y] = symbol;
            board->num_ships++;
        }
    }
}

/*
 * Función: generateBoard
 * ----------------------------
 * Genera un tablero con barcos colocados aleatoriamente.
 * Se inicializa el tablero y se colocan barcos según un arreglo predefinido.
 */
Board generateBoard() {
    Board board;
    // Reservar espacio para el tablero no es necesario ya que es una estructura.
    initializeBoard(&board);
    
    // Supongamos que tenemos 5 tipos de barcos con tamaños y cantidades fijas.
    // Se utiliza un arreglo de TipoShip definido de forma estática.
    typedef struct {
        int size;
        int quantity;
        char name[20];
        char symbol;
    } TipoShip;
    
    TipoShip ships[] = {
        {5, 1, "Aircraft Carrier", 'P'},
        {4, 1, "Warship", 'B'},
        {3, 2, "Cruiser", 'C'},
        {2, 2, "Destroyer", 'D'},
        {1, 3, "Submarine", 'S'}
    };
    int numShips = sizeof(ships) / sizeof(ships[0]);
    
    //     for (int i = 0; i < numShips; i++) {

    // Colocar cada tipo de barco.
    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < ships[i].quantity; j++) {
            placeShip(&board, ships[i].size, ships[i].symbol);
        }
    }
    
    return board;
}

/*
 * Función: boardToString
 * ----------------------------
 * Convierte el tablero a una cadena de caracteres.
 * Recorre el tablero y para cada celda que no esté vacía, añade "i,j,symbol;" al buffer.
 */
void boardToString(Board board, char *buffer, int buffer_size) {
    int offset = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            if (board.grid[i][j] != EMPTY) {
                offset += snprintf(buffer + offset, buffer_size - offset, "%d,%d,%c;", i, j, board.grid[i][j]);
                if (offset >= buffer_size) return;
            }
        }
    }
}

/*
 * Función: printBoard
 * ----------------------------
 * Imprime el tablero en la consola.
 */
void printBoard(Board board) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            printf("%c ", board.grid[i][j]);
        }
        printf("\n");
    }
}

// Devuelve un puntero al GameState correspondiente al usuario dado.
// Si no se encuentra, retorna NULL.
GameState* getBoard(const char *username) {
    if (manager == NULL || manager->states == NULL) {
        printf("El GameManager no ha sido inicializado.\n");
        return NULL;
    }
    
    // Se asume que hay 2 estados almacenados en el GameManager.
    for (int i = 0; i < 2; i++) {
        if (strcmp(manager->states[i].user, username) == 0) {
            return &manager->states[i];
        }
    }
    
    printf("No se encontró un GameState para el usuario: %s\n", username);
    return NULL;
}
