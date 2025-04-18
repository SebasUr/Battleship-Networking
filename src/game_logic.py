# game_logic.py
from .logging_utils import log_transaction
import socket


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

def set_last_attack_coords(r, c, state):
    state.last_attack_coords = (r, c)

def get_last_attack_coords(state):
    return state.last_attack_coords

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
        return

    if any(k in message for k in ["RESULT", "UPDATE", "END"]):
        parts = message.split('|')
        if len(parts) >= 5:
            try:
                state.my_turn = (int(parts[4]) == 1)
            except ValueError:
                state.my_turn = False
            if state.my_turn:
                print("\n¡ES TU TURNO! \n:", end='', flush=True)
                #print_boards_side_by_side(state, "MI TABLERO", "TABLERO ENEMIGO")
        return
