# Ruta de los directorios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Nombre de los ejecutables
SERVER = server
CLIENT = client

# Archivos fuente para el servidor
SERVER_SRCS = $(SRC_DIR)/server_main.c \
              $(SRC_DIR)/protocol.c \
              $(SRC_DIR)/session_handler.c \
              $(SRC_DIR)/game_manager.c \
              $(SRC_DIR)/utils.c   

# Archivos fuente para el cliente (solo si el cliente también usa funciones de utils.c)
CLIENT_SRCS = $(SRC_DIR)/client.c \
              $(SRC_DIR)/protocol.c
# Compilador y flags
CC = gcc
CFLAGS = -Wall -I$(INC_DIR)
LDFLAGS = -lpthread

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRCS) $(LDFLAGS)

$(CLIENT): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRCS)

clean:
	rm -f $(SERVER) $(CLIENT)

.PHONY: all clean
