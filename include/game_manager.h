#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

// Estructura para gestionar el estado del juego.
// Por ahora se deja vacío, pero en el futuro puedes añadir variables y datos relevantes.
typedef struct {
    // Estado del juego, jugadores, etc.
} GameManager;

// Prototipos de funciones (stubs) para manejar los comandos del juego.

// Procesa un ataque. Se recibe el id de partida, coordenadas x e y.
void game_manager_attack(int game_id, int x, int y);

// Procesa el resultado de un ataque, por ejemplo, para informar al atacante.
void server_result(int game_id, const char* result, int x, int y);

// Procesa la conexión de un cliente. 'data' puede ser, por ejemplo, el username u otra información.
void server_connect(int game_id, const char* data);

// Procesa una actualización (update) del estado del juego.
void server_update(int game_id, const char* data);

// Inicia la partida.
void server_start(int game_id);

// Finaliza la partida. Recibe un puntero a GameManager para modificar el estado global si es necesario.
void server_end(GameManager* game_manager, int game_id);

#endif // GAME_MANAGER_H
