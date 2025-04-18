# main.py
import sys
from .communication import run_client
from . import config

def main():
    if len(sys.argv) > 1:
        log_path = sys.argv[1]

        # Actualizamos el valor en el módulo config.
        config.LOGFILE = log_path

    while True:
        try:
            run_client()
        except Exception as e:
            print(f"Error durante la partida: {e}")
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
