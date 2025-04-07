import socket
import threading

class NetworkClient:
    """Clase para manejar la comunicación con el servidor"""
    
    def __init__(self, port=8080):
        """Inicializa el cliente de red sin predefinir la IP"""
        self.port = port
        self.socket = None
        self.read_thread = None
        self.message_handler = None
    
    def connect(self, server_ip):
        """Conecta al servidor utilizando la IP proporcionada"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((server_ip, self.port))
            print("Conectado al servidor.")
            return True
        except socket.error as e:
            print(f"Error al conectar: {e}")
            return False
    
    def send(self, message):
        """Envía un mensaje al servidor"""
        if self.socket:
            try:
                self.socket.sendall(message.encode())
                return True
            except socket.error as e:
                print(f"Error al enviar mensaje: {e}")
        return False
    
    def start_listening(self, message_handler):
        """Inicia el hilo de escucha de mensajes"""
        self.message_handler = message_handler
        self.read_thread = threading.Thread(target=self._read_messages)
        self.read_thread.daemon = True  # Permite que el hilo se cierre automáticamente cuando el programa principal termine
        self.read_thread.start()
    
    def _read_messages(self):
        """Función para leer mensajes del servidor"""
        while not self.message_handler.game_state.exit_flag:
            try:
                buff = self.socket.recv(1024).decode()
                if not buff:
                    print("\nDesconexión o error en la recepción.")
                    self.message_handler.game_state.exit_flag = True
                    break
                
                # Procesar el mensaje recibido
                self.message_handler.process_message(buff)
                
                if buff.startswith("exit"):
                    print("El otro usuario ha cerrado la sesión.")
                    self.message_handler.game_state.exit_flag = True
                    break
            except Exception as e:
                print(f"Error en la recepción: {e}")
                self.message_handler.game_state.exit_flag = True
                break
    
    def close(self):
        """Cierra la conexión"""
        if self.socket:
            self.socket.close()
            print("Conexión cerrada.")
