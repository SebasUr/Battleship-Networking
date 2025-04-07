class MessageParser:
    """Clase para interpretar mensajes del protocolo"""
    
    @staticmethod
    def parse_result(message):
        """Interpreta un mensaje de resultado"""
        parts = message.split('|')
        if len(parts) >= 4:
            match_id = parts[1]
            result = parts[2]  # HIT/NOHIT
            turn = int(parts[3])
            return {
                'match_id': match_id,
                'result': result,
                'turn': turn
            }
        return None
    
    @staticmethod
    def parse_error(message):
        """Interpreta un mensaje de error"""
        parts = message.split('|')
        if len(parts) >= 4:
            match_id = parts[1]
            error_type = parts[2]  # OOB/DA
            turn = int(parts[3])
            return {
                'match_id': match_id,
                'error_type': error_type,
                'turn': turn
            }
        return None
    
    @staticmethod
    def parse_login_response(message):
        """Interpreta la respuesta al login"""
        parts = message.split('|')
        if len(parts) >= 4 and parts[0] == "LOGGED":
            match_id = parts[1]
            status = parts[2]
            turn = int(parts[3])
            return {
                'match_id': match_id,
                'status': status,
                'turn': turn
            }
        return None
    
    @staticmethod
    def create_login_message(game_id, username):
        """Crea un mensaje de login"""
        return f"MSG_LOGIN|{game_id}|{username}|"
    
    @staticmethod
    def create_attack_message(match_id, x, y):
        """Crea un mensaje de ataque"""
        return f"ATTACK|{match_id}|{x},{y}"


# game_state.py
class GameState:
    """Clase para manejar el estado del juego"""
    
    def __init__(self):
        """Inicializa el estado del juego"""
        self.exit_flag = False
        self.my_turn = False
        self.match_id = None
        self.last_attack_coords = None
    
    def set_match_id(self, match_id):
        """Establece el ID de la partida"""
        self.match_id = match_id
    
    def set_turn(self, turn_value):
        """Establece si es el turno del jugador"""
        self.my_turn = turn_value == 1
    
    def set_last_attack(self, x, y):
        """Guarda las coordenadas del último ataque"""
        self.last_attack_coords = (x, y)
    
    def get_last_attack(self):
        """Retorna las coordenadas del último ataque"""
        return self.last_attack_coords

