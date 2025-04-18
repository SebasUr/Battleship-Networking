import socket
import threading
import sys
import time
from datetime import datetime

# Constantes globales permitidas.
MAX = 1024
PORT = 8080
SERVER_IP = "127.0.0.1"
DEBUG = True  # Modo debug: imprime todos los mensajes recibidos

# ----------------------------------------------------
# Clase que encapsula el estado del juego
# ----------------------------------------------------
class GameState:
    def __init__(self):
        self.exit_flag = False
        self.my_turn = False
        self.last_attack_coords = None
        self.my_board = [[0 for _ in range(10)] for _ in range(10)]
        self.enemy_board = [[0 for _ in range(10)] for _ in range(10)]

# ----------------------------------------------------
# Función de logging de transacciones
# ----------------------------------------------------
def log_transaction(query, response_id, client_id, server_response):
    now = datetime.now()
    date_str = now.strftime("%Y-%m-%d")
    time_str = now.strftime("%H:%M:%S")
    log_message = f"{date_str} {time_str} {client_id} {query} {response_id} {server_response}"
    with open("transaction_log.txt", "a") as log_file:
        log_file.write(log_message + "\n")

# ----------------------------------------------------
# Funciones que manejan el estado del juego
# ----------------------------------------------------
def fill_board_from_logged_message(message, state):
    parts = message.split('|')
    if len(parts) >= 5:
        for pos in parts[4].split(';'):
            if not pos:
                continue
            try:
                r_str, c_str, letter = pos.split(',')
                r, c = int(r_str), int(c_str)
                if 0 <= r < 10 and 0 <= c < 10:
                    state.my_board[r][c] = letter
            except ValueError:
                print(f"Error procesando coordenada: {pos}")

def print_boards_side_by_side(state, title1, title2):
    header1 = f"{title1}".ljust(24)
    header2 = title2
    print(f"{header1}    {header2}")
    cols = "   " + " ".join(str(i) for i in range(10))
    print(f"{cols.ljust(24)}    {cols}")
    sep = "  " + "-" * 21
    print(f"{sep.ljust(24)}    {sep}")
    for r in range(10):
        row1 = f"{r}| "
        row2 = f"{r}| "
        for c in range(10):
            # Tablero propio
            cell1 = state.my_board[r][c]
            if cell1 == 0:
                sym1 = '~ '
            elif cell1 == 2:
                sym1 = 'O '
            elif cell1 == 3:
                sym1 = 'X '
            elif isinstance(cell1, str):
                sym1 = f"{cell1} "
            else:
                sym1 = '? '
            row1 += sym1
            # Tablero enemigo
            cell2 = state.enemy_board[r][c]
            if cell2 == 0:
                sym2 = '~ '
            elif cell2 == 2:
                sym2 = 'O '
            elif cell2 == 3:
                sym2 = 'X '
            elif isinstance(cell2, str):
                sym2 = f"{cell2} "
            else:
                sym2 = '? '
            row2 += sym2
        print(f"{row1.ljust(24)}    {row2}")
    print("\n")

# Funciones para acceder/modificar last_attack_coords, usando el state
def set_last_attack_coords(r, c, state):
    state.last_attack_coords = (r, c)

def get_last_attack_coords(state):
    return state.last_attack_coords

# ----------------------------------------------------
# Funciones de procesamiento de mensajes
# ----------------------------------------------------
def interpret_message(message):
    if message.startswith("RESULT"):
        _, _, res = message.split('|', 2)
        if res == "HIT":
            print("\n¡IMPACTO! Has golpeado un barco enemigo.")
        elif res == "NOHIT":
            print("\nAGUA. No has golpeado ningún barco.")
    elif message.startswith("ERROR"):
        _, _, err, _ = message.split('|', 3)
        if err == "OOB":
            print("\nError: Ataque fuera de los límites.")
        elif err == "DA":
            print("\nError: Ya atacaste esa posición. Vuelve a intentarlo.")

def update_board(message, state):
    if message.startswith("RESULT"):
        parts = message.split('|')
        if len(parts) >= 3:
            r, c = get_last_attack_coords(state)
            state.enemy_board[r][c] = 3 if parts[2] == "HIT" else 2
            if parts[2] == "HIT":
                print(f"\n¡Has impactado en ({r},{c})!")
            else:
                print(f"\nTu ataque a ({r},{c}) fue al AGUA.")
    elif message.startswith("UPDATE"):
        parts = message.split('|')
        if len(parts) >= 5:
            res = parts[2]
            coords = parts[3].split(',')
            try:
                r, c = int(coords[0]), int(coords[1])
                state.my_board[r][c] = 'X' if res == "HIT" else 'O'
                if res == "HIT":
                    print(f"\n¡Has recibido un IMPACTO en ({r},{c})!")
                else:
                    print(f"\nTu oponente disparó en ({r},{c}) y fue AGUA.")
            except ValueError:
                print(f"Error procesando coords de UPDATE: {parts[3]}")
    print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")

def update_turn(message, sock, match_id, state):
    if message.startswith("TIMEOUT"):
        state.my_turn = not state.my_turn
        print("\n⏰ TIMEOUT recibido. Se invierte el turno.")
        if state.my_turn:
            print("➡️  Ahora SÍ es tu turno (el oponente se quedó sin tiempo).")
            print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")
        else:
            print("⛔ Ahora NO es tu turno (se te acabó el tiempo).\n\n", end='', flush=True)
        return

    if message.startswith("LOGGED"):
        fill_board_from_logged_message(message, state)
        log_transaction(message, sock.getpeername()[0],
                        socket.gethostbyname(socket.gethostname()), " - SERVER")
        parts = message.split('|')
        if len(parts) >= 4:
            try:
                state.my_turn = (int(parts[3]) == 1)
            except ValueError:
                state.my_turn = False
        print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")
        if state.my_turn:
            print("\n¡ES TU TURNO!")
        return

    if message.startswith("ERROR"):
        parts = message.split('|')
        if len(parts) >= 4:
            try:
                state.my_turn = (int(parts[3]) == 1)
            except ValueError:
                state.my_turn = False
            if state.my_turn:
                print("\n¡ES TU TURNO! Vuelve a ingresar coordenadas.")
                print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")
                time.sleep(2)
        return

    if any(k in message for k in ["RESULT", "UPDATE", "END"]):
        parts = message.split('|')
        if len(parts) >= 5:
            try:
                state.my_turn = (int(parts[4]) == 1)
            except ValueError:
                state.my_turn = False
            if state.my_turn:
                print("\n¡ES TU TURNO!")
                print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")
        return

# ----------------------------------------------------
# Funciones de comunicación con el servidor
# ----------------------------------------------------
def read_msg(sock, match_id, state):
    client_ip = socket.gethostbyname(socket.gethostname())
    while not state.exit_flag:
        try:
            buff = sock.recv(MAX).decode()
            if not buff:
                print("\nDesconexión del servidor.")
                state.exit_flag = True
                break

            if DEBUG:
                print(f"[DEBUG] Recibido: {buff}")

            response_ip = sock.getpeername()[0]
            log_transaction(buff, response_ip, client_ip, " - SERVER")
            if buff.startswith("GAME_END"):
                parts = buff.split('|')
                if len(parts) >= 4:
                    winner = parts[3]
                    if parts[2] == "FF":
                        print(f"\n🎉 Juego terminado por rendición. Ganador: {winner}")
                    else:
                        print(f"\n🎉 Juego terminado. Ganador: {winner}")
                state.exit_flag = True
                sock.close()
                break

            interpret_message(buff)
            update_board(buff, state)
            update_turn(buff, sock, match_id, state)

            if buff.startswith("exit"):
                print("El otro usuario cerró sesión.")
                state.exit_flag = True
        except Exception as e:
            print(f"Error en recepción: {e}")
            state.exit_flag = True

def handle_input(sock, match_id, state):
    while not state.exit_flag:
        if state.my_turn:
            print("\nIngresa coordenadas (fila,columna): ", end='', flush=True)
            coords = input("").strip()
            if not state.my_turn:
                print("🚫 Ya no es tu turno, no se envía ataque.")
                continue
            try:
                if coords == "FF":
                    print("Riendiendote...")
                    sock.sendall(f"FF|{match_id}".encode())
                    log_transaction(f"FF|{match_id}", sock.getpeername()[0],
                                    socket.gethostbyname(socket.gethostname()), " - CLIENT")
                else:
                    r, c = map(int, coords.split(','))
                    if 0 <= r < 10 and 0 <= c < 10:
                        state.last_attack_coords = (r, c)
                        sock.sendall(f"ATTACK|{match_id}|{r},{c}".encode())
                        print(f"Enviando ataque a ({r},{c})...")
                        log_transaction(f"ATTACK|{match_id}|{r},{c}", sock.getpeername()[0],
                                        socket.gethostbyname(socket.gethostname()), " - CLIENT")
                        state.my_turn = False
                    else:
                        print("Error: coordenadas entre 0 y 9.")
            except ValueError:
                print("Formato incorrecto. Usa 'fila,columna' (ej: 6,2).")
        else:
            threading.Event().wait(0.1)

# ----------------------------------------------------
# Función que encapsula todo el flujo del cliente
# ----------------------------------------------------
def run_client():
    state = GameState()

    username = input("Username: ").strip()
    if not username:
        print("Username vacío.")
        return

    while True:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((SERVER_IP, PORT))
            print("Conectado al servidor.")
        except Exception as e:
            print(f"Error de conexión: {e}")
            return

        game_id_input = input("ID de partida: ").strip()
        try:
            game_id = int(game_id_input)
        except ValueError:
            print("ID inválido.")
            sock.close()
            continue

        print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")
        sock.sendall(f"LOGIN|{game_id}|{username}".encode())
        try:
            buff = sock.recv(MAX).decode()
            if not buff:
                print("No llegó respuesta del login.")
                sock.close()
                return
            if DEBUG:
                print(f"[DEBUG] Recibido (login): {buff}")
        except Exception as e:
            print(f"Error al recibir login: {e}")
            sock.close()
            return

        parts = buff.split('|')
        if parts[0] == "LOGGED" and len(parts) >= 3 and parts[2].upper() == "NO":
            print("Sala ocupada, ingresa otro ID de partida.")
            sock.close()
            continue

        update_turn(buff, sock, game_id, state)
        break

    threading.Thread(target=read_msg, args=(sock, game_id, state), daemon=True).start()
    threading.Thread(target=handle_input, args=(sock, game_id, state), daemon=True).start()

    while not state.exit_flag:
        threading.Event().wait(1)
    sock.close()
    print("Fin de la partida.")

# ----------------------------------------------------
# Función principal con opción de volver a jugar
# ----------------------------------------------------
def main():
    while True:
        try:
            run_client()
        except Exception as e:
            print(f"Error durante la partida: {e}")
        # Se pide que el usuario ingrese solo 's' o 'n'
        while True:
            try:
                replay = input("¿Desea volver a jugar? (s/n): ").strip().lower()
            except Exception as e:
                print(f"Error al leer el input: {e}")
                continue
            if replay in ('s', 'n'):
                break
            else:
                print("Entrada inválida. Por favor, ingrese 's' o 'n'.")
        if replay != 's':
            break
    print("Cliente cerrado.")


if __name__ == "__main__":
    main()
