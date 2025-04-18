# communication.py

import socket
import termios
import threading
import sys
import time
from .logging_utils import log_transaction
from .game_logic import (interpret_message, update_board, update_turn, 
                         print_boards_side_by_side, fill_board_from_logged_message)
from .gamestate import GameState
from .config import MAX, PORT, SERVER_IP, DEBUG

def read_msg(sock, match_id, state):
    client_ip = socket.gethostbyname(socket.gethostname())
    while not state.exit_flag:
        try:
            buff = sock.recv(1024).decode()
            if not buff:
                print("\nDesconexión del servidor.")
                state.exit_flag = True
                break

            if DEBUG:
                print(f"[DEBUG] Recibido: {buff}")

            response_ip = sock.getpeername()[0]
            log_transaction(buff, response_ip, client_ip, " - SERVER")

            if buff.startswith("UPDATE"):
                ack_msg = f"ACK|{match_id}|1"
                sock.sendall(ack_msg.encode())
                log_transaction(ack_msg, response_ip, client_ip, " - CLIENT")
                
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
            try:
                termios.tcflush(sys.stdin, termios.TCIFLUSH)
            except Exception:
                pass  # Si no es UNIX, ignoramos
            coords = input("Ingresa coordenadas (fila,columna): ").strip()
            
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

def run_client():
    state = GameState()

    username = input("Username: ").strip()
    if not username:
        print("Username vacío.")
        return

    while True:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((socket.gethostbyname(SERVER_IP), PORT))
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
            buff = sock.recv(1024).decode()
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
