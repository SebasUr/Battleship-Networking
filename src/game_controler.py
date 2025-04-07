import socket
import threading
from game_board import GameBoard, GameState
from message_parser import MessageParser
from network_cliente import NetworkClient
from message_handler import MessageHandler

class GameController:
    """Clase principal que controla el juego"""
    
    def __init__(self, port=8080):
        """Inicializa el controlador del juego"""
        self.game_state = GameState()
        self.my_board = GameBoard()
        self.enemy_board = GameBoard()
        self.network_client = NetworkClient(port)
        self.message_handler = MessageHandler(self.game_state, self.my_board, self.enemy_board, self.network_client)
        self.parser = MessageParser()
    
    def start(self):
        """Inicia el juego"""
        # Solicitar la IP del servidor
        server_ip = input("Ingrese la dirección IP del servidor: ").strip()
        if not server_ip:
            print("Error leyendo la IP del servidor.")
            return
        
        # Conectar al servidor
        if not self.network_client.connect(server_ip):
            return
        
        # Solicitar datos de usuario
        username = input("Ingrese su username: ").strip()
        if not username:
            print("Error leyendo username.")
            return
        
        # Solicitar ID de partida
        try:
            game_id = int(input("Ingrese el ID de partida: ").strip())
            self.game_state.set_match_id(game_id)
        except ValueError:
            print("Error leyendo el ID de partida.")
            return
        
        # Inicializar tableros
        print("\nInicializando juego...")
        self.my_board.print("MI TABLERO")
        self.enemy_board.print("TABLERO ENEMIGO")
        
        # Login
        login_msg = self.parser.create_login_message(game_id, username)
        if not self.network_client.send(login_msg):
            return
        
        # Esperar respuesta de login
        try:
            buff = self.network_client.socket.recv(1024).decode()
            if not buff:
                print("Error al recibir ACK de login")
                self.network_client.close()
                return
            print("Login exitoso")
            self.message_handler.process_message(buff)
        except socket.error as e:
            print(f"Error al recibir ACK de login: {e}")
            self.network_client.close()
            return
        
        # Iniciar escucha de mensajes
        self.network_client.start_listening(self.message_handler)
        
        # Bucle principal
        try:
            while not self.game_state.exit_flag:
                # Simplemente esperamos eventos
                threading.Event().wait(1)
        except KeyboardInterrupt:
            print("\nInterrupción manual. Cerrando cliente.")
            self.game_state.exit_flag = True
        finally:
            self.network_client.close()

def main():
    """Función principal"""
    try:
        controller = GameController()
        controller.start()
    except Exception as e:
        print(f"Error fatal: {e}")

if __name__ == "__main__":
    main()
