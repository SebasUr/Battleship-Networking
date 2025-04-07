import socket
import threading

MAX = 1024
PORT = 8080
SERVER_IP = "127.0.0.1"

exit_flag = False
my_turn = False
last_attack_coords = None
DEBUG = True  # Modo debug: imprime todos los mensajes recibidos

# Tableros
MY_BOARD = [[0 for _ in range(10)] for _ in range(10)]
ENEMY_BOARD = [[0 for _ in range(10)] for _ in range(10)]


def fill_board_from_logged_message(message):
    global MY_BOARD
    parts = message.split('|')
    if len(parts) >= 5:
        # parts[4] contiene la lista de posiciones separadas por ';'
        for pos in parts[4].split(';'):
            if not pos:
                continue
            try:
                r_str, c_str, letter = pos.split(',')
                r, c = int(r_str), int(c_str)
                if 0 <= r < 10 and 0 <= c < 10:
                    MY_BOARD[r][c] = letter
            except ValueError:
                print(f"Error procesando coordenada: {pos}")


def print_boards_side_by_side(b1, title1, b2, title2):
    # Imprime dos tableros uno al lado del otro
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
            # Tablero 1
            cell1 = b1[r][c]
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
            # Tablero 2
            cell2 = b2[r][c]
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
    print()


def set_last_attack_coords(r, c):
    global last_attack_coords
    last_attack_coords = (r, c)


def get_last_attack_coords():
    return last_attack_coords


def interpret_message(message):
    # Mensajes de resultado de ataque propio
    if message.startswith("RESULT"):
        _, _, res = message.split('|', 2)
        if res == "HIT":
            print("\n¡IMPACTO! Has golpeado un barco enemigo.")
        elif res == "NOHIT":
            print("\nAGUA. No has golpeado ningún barco.")
    elif message.startswith("ERROR"):
        _, _, err = message.split('|', 2)
        if err == "OOB":
            print("\nError: Ataque fuera de los límites.")
        elif err == "DA":
            print("\nError: Ya atacaste esa posición.")


def update_board(message):
    # Procesa RESULT para ataque propio y UPDATE para ataque recibido
    if message.startswith("RESULT"):
        parts = message.split('|')
        if len(parts) >= 3:
            r, c = get_last_attack_coords()
            ENEMY_BOARD[r][c] = 3 if parts[2] == "HIT" else 2
    elif message.startswith("UPDATE"):
        # Formato: UPDATE|MatchID|HIT/NOHIT|r,c|turn
        parts = message.split('|')
        if len(parts) >= 5:
            res = parts[2]
            coords = parts[3].split(',')
            try:
                r, c = int(coords[0]), int(coords[1])
                MY_BOARD[r][c] = 'X' if res == "HIT" else 'O'
                if res == "HIT":
                    print(f"\n¡Has recibido un IMPACTO en ({r},{c})!")
                else:
                    print(f"\nTu oponente disparó en ({r},{c}) y fue AGUA.")
            except ValueError:
                print(f"Error procesando coords de UPDATE: {parts[3]}")
    # Después de actualizar, mostramos ambos tableros
    print_boards_side_by_side(MY_BOARD, "MI TABLERO", ENEMY_BOARD, "TABLERO ENEMIGO")


def update_turn(message, sock, match_id):
    global my_turn

    if message.startswith("TIMEOUT"):
        my_turn = not my_turn
        print("\n⏰ TIMEOUT recibido. Se invierte el turno.")
        if my_turn:
            print("➡️  Ahora SÍ es tu turno (el oponente se quedó sin tiempo).")
        else:
            print("⛔ Ahora NO es tu turno (se te acabó el tiempo).")
        if my_turn:
            print_boards_side_by_side(MY_BOARD, "MI TABLERO", ENEMY_BOARD, "TABLERO ENEMIGO")
        return

    if message.startswith("LOGGED"):
        fill_board_from_logged_message(message)
        # Para LOGGED, el turno está en la posición 3
        parts = message.split('|')
        if len(parts) >= 4:
            try:
                my_turn = (int(parts[3]) == 1)
            except ValueError:
                my_turn = False
        print_boards_side_by_side(MY_BOARD, "MI TABLERO", ENEMY_BOARD, "TABLERO ENEMIGO")
        if my_turn:
            print("\n¡ES TU TURNO!")
        return

    # Para RESULT, UPDATE, END y ERROR, el turno está en la posición 4
    if any(k in message for k in ["RESULT", "UPDATE", "END", "ERROR"]):
        parts = message.split('|')
        if len(parts) >= 5:
            try:
                my_turn = (int(parts[4]) == 1)
            except ValueError:
                my_turn = False
            if my_turn:
                print("\n¡ES TU TURNO!")
                print_boards_side_by_side(MY_BOARD, "MI TABLERO", ENEMY_BOARD, "TABLERO ENEMIGO")


def read_msg(sock, match_id):
    global exit_flag
    while not exit_flag:
        try:
            buff = sock.recv(MAX).decode()
            if not buff:
                print("\nDesconexión del servidor.")
                exit_flag = True
                break

            if DEBUG:
                print(f"[DEBUG] Recibido: {buff}")

            interpret_message(buff)
            update_board(buff)
            update_turn(buff, sock, match_id)

            if buff.startswith("exit"):
                print("El otro usuario cerró sesión.")
                exit_flag = True
        except Exception as e:
            print(f"Error en recepción: {e}")
            exit_flag = True


def handle_input(sock, match_id):
    """Hilo dedicado a leer input SOLO cuando my_turn == True."""
    global exit_flag, my_turn
    while not exit_flag:
        if my_turn:
            coords = input("Ingresa coordenadas (fila,columna): ").strip()
            if not my_turn:
                print("🚫 Ya no es tu turno, no se envía ataque.")
                continue
            try:
                r, c = map(int, coords.split(','))
                if 0 <= r < 10 and 0 <= c < 10:
                    set_last_attack_coords(r, c)
                    sock.sendall(f"ATTACK|{match_id}|{r},{c}".encode())
                    my_turn = False
                else:
                    print("Error: coordenadas entre 0 y 9.")
            except ValueError:
                print("Formato incorrecto. Usa 'fila,columna' (ej: 6,2).")
        else:
            threading.Event().wait(0.1)


def main():
    global exit_flag

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_IP, PORT))
        print("Conectado al servidor.")
    except Exception as e:
        print(f"Error de conexión: {e}")
        return

    username = input("Username: ").strip()
    if not username:
        print("Username vacío.")
        return
    try:
        game_id = int(input("ID de partida: ").strip())
    except ValueError:
        print("ID inválido.")
        return

    # Mostrar tableros vacíos
    print_boards_side_by_side(MY_BOARD, "MI TABLERO", ENEMY_BOARD, "TABLERO ENEMIGO")

    # Enviar LOGIN y procesar ACK=LOGGED
    sock.sendall(f"LOGIN|{game_id}|{username}".encode())
    try:
        buff = sock.recv(MAX).decode()
        if not buff:
            print("No llegó respuesta del login.")
            sock.close()
            return
        if DEBUG:
            print(f"[DEBUG] Recibido (login): {buff}")
        update_turn(buff, sock, game_id)
    except Exception as e:
        print(f"Error al recibir login: {e}")
        sock.close()
        return

    # Arrancamos hilos de lectura y de input
    threading.Thread(target=read_msg, args=(sock, game_id), daemon=True).start()
    threading.Thread(target=handle_input, args=(sock, game_id), daemon=True).start()

    # Mantenemos vivo hasta exit
    try:
        while not exit_flag:
            threading.Event().wait(1)
    except KeyboardInterrupt:
        exit_flag = True
    finally:
        sock.close()
        print("Cliente cerrado.")


if __name__ == "__main__":
    main()
