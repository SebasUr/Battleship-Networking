import message_parser as MessageParser
import socket

class MessageHandler:
    """Clase para manejar los mensajes recibidos"""
    
    def __init__(self, game_state, my_board, enemy_board, network_client):
        """Inicializa el manejador de mensajes"""
        self.game_state = game_state
        self.my_board = my_board
        self.enemy_board = enemy_board
        self.network_client = network_client
        self.parser = MessageParser()
    
    def process_message(self, message):
        """Procesa un mensaje recibido"""
        # Interpretar el mensaje
        self._interpret_message(message)
        
        # Actualizar el tablero
        self._update_board(message)
        
        # Actualizar el turno
        self._update_turn(message)
    
    def _interpret_message(self, message):
        """Interpreta el mensaje y muestra información relevante"""
        if message.startswith("RESULT"):
            result_data = self.parser.parse_result(message)
            if result_data:
                if result_data['result'] == "HIT":
                    print("\n¡IMPACTO! Has golpeado un barco enemigo.")
                elif result_data['result'] == "NOHIT":
                    print("\nAGUA. No has golpeado ningún barco.")
        
        elif message.startswith("ERROR"):
            error_data = self.parser.parse_error(message)
            if error_data:
                if error_data['error_type'] == "OOB":
                    print("\nError: Ataque fuera de los límites del tablero.")
                elif error_data['error_type'] == "DA":
                    print("\nError: Ya has atacado esa posición anteriormente.")
    
    def _update_board(self, message):
        """Actualiza el tablero según el mensaje recibido"""
        if message.startswith("RESULT"):
            result_data = self.parser.parse_result(message)
            if result_data:
                coords = self.game_state.get_last_attack()
                if coords:
                    x, y = coords
                    if result_data['result'] == "HIT":
                        self.enemy_board.update_cell(x, y, 3)  # Hit
                    elif result_data['result'] == "NOHIT":
                        self.enemy_board.update_cell(x, y, 2)  # Miss
                
                # Mostrar tableros
                self.my_board.print("MI TABLERO")
                self.enemy_board.print("TABLERO ENEMIGO")
    
    def _update_turn(self, message):
        """Actualiza el turno según el mensaje recibido"""
        try:
            if any(keyword in message for keyword in ["LOGGED", "RESULT", "UPDATE", "END", "ERROR"]):
                parts = message.split('|')
                if len(parts) >= 4:
                    turn_value = int(parts[3])
                    self.game_state.set_turn(turn_value)
                    
                    if self.game_state.my_turn:
                        self._handle_my_turn()
        except Exception as e:
            print(f"Error al actualizar el turno: {e}")
    
    def _handle_my_turn(self):
        """Maneja el turno del jugador"""
        print("\n¡ES TU TURNO!")
        self.my_board.print("MI TABLERO")
        self.enemy_board.print("TABLERO ENEMIGO")
        
        # Solicitar coordenadas
        while True:
            attack_input = input("Ingresa coordenadas (x,y): ").strip()
            try:
                # Verificar formato
                x, y = map(int, attack_input.split(','))
                if 0 <= x < self.enemy_board.size and 0 <= y < self.enemy_board.size:
                    self.game_state.set_last_attack(x, y)
                    
                    # Enviar ataque
                    attack_msg = self.parser.create_attack_message(
                        self.game_state.match_id, x, y)
                    self.network_client.send(attack_msg)
                    
                    # Ceder turno
                    self.game_state.my_turn = False
                    break
                else:
                    print(f"Error: Las coordenadas deben estar entre 0 y {self.enemy_board.size-1}")
            except ValueError:
                print("Error: Formato incorrecto. Utiliza 'x,y' (ejemplo: 3,5)")

