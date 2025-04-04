#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H
#define FILAS 10
#define COLUMNAS 10

#include "protocol.h"


// Procesa el login. Recibe:
// - game_id: ID de la partida.
// - username: El nombre de usuario.
// - initial_info: Buffer para almacenar la información inicial (por ejemplo, "1,2,3").
// - initial_info_size: Tamaño del buffer.
// - turn: Salida para indicar el turno asignado (1 para el primero, 0 para el segundo).
// La función retorna 0 si todo sale bien o un código de error.


// Estructura para gestionar el estado del juego.
// Por ahora se deja vacío, pero en el futuro puedes añadir variables y datos relevantes.

// Definiciòn de los tableros para seguimiento de cantidad de barcos
typedef struct {
    char grid[FILAS][COLUMNAS];
    int numero_barcos;
 }tablero;


// Estructura para seguir el estado de la partida de ambos usuarios
typedef struct {
    char *usuario;
    tablero tablero;
} game_manager;


// Definición de los tipos de barcos
typedef struct {
    int tamano;
    int cantidad;
    char nombre[20];
    char simbolo;

} TipoBarco;

// Lista de barcos con sus tamaños y cantidades
TipoBarco barcos[] = {
    {5, 1, "Portaaviones", 'P'},
    {4, 1, "Buque de guerra", 'B'},
    {3, 2, "Crucero", 'C'},
    {2, 2, "Destructor", 'D'},
    {1, 3, "Submarino", 'S'}
};


// Prototipos de funciones (stubs) para manejar los comandos del juego.

// Procesa un ataque. Se recibe el id de partida, coordenadas x e y.
void game_manager_attack(int game_id, int x, int y, const char* user0, const char* user1);

// Procesa el resultado de un ataque, por ejemplo, para informar al atacante.
void server_result(int game_id, const char* result, int x, int y);

// Procesa la conexión de un cliente. 'data' puede ser, por ejemplo, el username u otra información.
void server_connect(int game_id, const char* data);

// Procesa una actualización (update) del estado del juego.
void server_update(int game_id, const char* data);

// Inicia la partida.
void server_start(int game_id);

// Finaliza la partida. Recibe un puntero a GameManager para modificar el estado global si es necesario.
void server_end(tablero* game_manager, int game_id);

// Inicializa el tablero vacío.
void inicializarTablero(tablero *tablero);

// Ubica aleatoriamente los barcos del tablero.
void generarTablero(tablero *tablero);

// Imprime el tablero.
void imprimirTablero(tablero *tablero);

// Agrega un usuario.
void agregarUsuario(char *username);

// Obtiene el tablero de un usuario
GameManager* obtenerTablero(char *username);

#endif // GAME_MANAGER_H
