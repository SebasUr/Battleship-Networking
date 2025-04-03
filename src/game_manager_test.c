#include "game_manager.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VACIO '.'
#define BARCO 'B'
#define ATAQUE 'X'


// Variables auxiliares
static char usuario_aux[20];
static tablero valor_aux;
static bool primer_usuario_guardado = false;
static game_manager *manager = NULL;

// complet: si true se inicializa con ambos usuarios, sino se añade solo el primer usuario
void server_start(const char *usuario) {
    if (primer_usuario_guardado==false) {
        // Guardamos temporalmente en variables auxiliares
        strcpy(usuario_aux, usuario);
        valor_aux = generarTablero();
        primer_elemento_guardado = true;

    } else {
        if (!primer_elemento_guardado) {
            printf("Error: El primer usuario no ha sido guardado.\n");
            return;
        }

        // Crear el diccionario con memoria dinámica
        manager = malloc(2 * sizeof(game_manager));
        if (!lista_usuarios) {
            printf("Error de asignación de memoria.\n");
            return;
        }

        // Insertar el primer elemento almacenado
        strcpy(manager[0].usuario, usuario_aux);
        manager[0].tablero = valor_aux;

        // Insertar el segundo elemento recibido
        strcpy(manager[1].usuario, usuario);
        manager[1].tablero = generarTablero();
        
    }
}

// Función para obtener el valor dado una clave
tablero* obtener_tablero(const char *usuario) {

    for (int i = 0; i < 2; i++) {
        if (strcmp(manager[i].usuario, usuario) == 0) {
            return &manager[i].tablero;
        }
    }

    printf("Usuario '%s' no encontrado.\n", usuario);
    return NULL;
}

void game_manager_attack(int game_id, int x, int y, const char* user0, const char* user1) {
    char msg_error[50];
    char result[10];
    int  win;
    //printf("game_manager_attack: game_id=%d, x=%d, y=%d\n", game_id, x, y);
    if (x < 0 || y < 0 || x > FILAS || y > COLUMNAS){

        snprintf(msg_error, sizeof(msg_error), "ERROR|%d|Invalid move: out of bounds", game_id);
        parse_and_handle_message(msg_error);
    }
    else{
        
        tablero tablero = obtener_tablero(user1);
        //Condiciòn para ataque duplicado
        if (tablero->grid[x][y]==ATAQUE){
            snprintf(msg_error, sizeof(msg_error), "ERROR|%d|Duplicate attack", game_id);
            
        }
        //Verificar si acertó
        else 
        {

            if (tablero[x][y]==BARCO){
                strcpy(result, "HIT");

                tablero->grid[x][y] = ATAQUE;
                tablero->numero_barcos -= 1;
            }
            else
            {
                strcpy(result, "NO HIT");
            }

            

            //Verificar si CONTINUE o WIN
            if (tablero->numero_barcos==0)
            {
                win = 1;
            }
            else{
                win = 0;
            }
            
            
            //Definir mensajes de result y update
            char msg_result[30];
            char msg_update[30];
            
            snprintf(msg_result, "RESULT|%d|%s|%d,%d|%d", game_id, result, x, y, win);

            snprintf(msg_result, "RESULT|%d|ENEMY %s|%d,%d|%d", game_id, result, x, y, win);
                
            parse_and_handle_message(msg_result);
            parse_and_handle_message(msg_update);
            
            
        }
    
    }
}

void server_result(int game_id, const char* result, int x, int y) {
    printf("server_result: game_id=%d, x=%d, y=%d, result=%s\n", game_id, x, y, result);
}

void server_connect(int game_id, const char* data) {
    printf("server_connect: game_id=%d, data=%s\n", game_id, data);
}

void server_update(int game_id, const char* data) {
    printf("server_update: game_id=%d, data=%s\n", game_id, data);
}

void server_start(int game_id) {
    printf("server_start: game_id=%d\n", game_id);
}

void server_end(tablero* game_manager, int game_id) {
    // Aquí se podría modificar el estado del game_manager si es necesario.
    printf("server_end: game_id=%d\n", game_id);
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
}

