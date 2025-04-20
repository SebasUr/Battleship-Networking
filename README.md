# Battleship Networking Client

Este repositorio contiene el cliente de consola para el juego Battleship sobre red.

## Estructura del proyecto

```
client/
├── src/
│   ├── config.py          # Configuración de IP, puerto, debug y logfile por defecto
│   ├── main.py            # Módulo principal para ejecutar el cliente
│   ├── communication.py   # Lógica de comunicación con el servidor
│   ├── gamestate.py       # Clase que encapsula el estado del juego
│   ├── game_logic.py      # Funciones de manejo de tableros y mensajes
│   ├── logging_utils.py   # Funciones de registro de transacciones
│   └── __init__.py        # Marca `src/` como paquete Python
└── README.md
```

## Requisitos

- Python 3.7 o superior
- Sistema Unix/Linux con Python instalado

## Instalación y dependencia

1. Clona este repositorio:
   ```bash
   git clone <URL_DEL_REPOSITORIO>
   git fetch --all
   git checkout client
   cd Battleship-Networking
   ```

## Configuración

Todas las constantes de red y debug se encuentran en **`src/config.py`**:

```python
# src/config.py
MAX = 1024              # Tamaño máximo de buffer para recv()
PORT = 8080             # Puerto del servidor de Battleship
SERVER_IP = "127.0.0.1"  # IP del servidor
DEBUG = True            # Mostrar mensajes DEBUG en consola
LOGFILE = "transaction_log.txt"  # Archivo de log por defecto
```

Puedes editar estos valores antes de ejecutar el cliente.

## Ejecución

Desde el directorio principal (`client/`), ejecuta:

```bash
python -m src.main [ruta_log]
```

- `ruta_log` (opcional): path donde deseas que se guarde el archivo de transacciones.
  - Si no se proporciona, se usará el valor por defecto definido en `config.LOGFILE`.
  - Ejemplo:
    ```bash
    python -m src.main ./mis_logs/log1.txt
    ```

## Flujo del cliente

1. Se pide **Username**.
2. Se pide **ID de partida**. Si la sala está ocupada o no existe, se vuelve a pedir.
3. Se inicia la partida:
   - Se imprimen los tableros (propio y enemigo).
   - Se alternan mensajes `LOGIN`, `RESULT`, `UPDATE`, `TIMEOUT`, `ACK`, etc.
   - Al usuario se le pide introducir coordenadas para el ataque.
   - Se van cambiando los turnos hasta desistir utilizando "FF" o que uno de los jugadores hunda todos los barcos.
4. Al terminar la partida, pregunta si deseas **volver a jugar** (`s`/`n`). Se inicia un registro desde 0.

## Logs de transacciones

Todas las peticiones y respuestas al servidor se registran en el archivo de log:

```
Fecha Hora Cliente Query ResponseID ServerResponse
```

- Se registra tanto en cliente como en servidor.

