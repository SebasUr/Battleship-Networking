class GameBoard:
    """Clase para representar y manipular el tablero de juego"""
    
    def __init__(self, size=10):
        """Inicializa un tablero de tamaño size x size"""
        self.size = size
        # 0 = agua, 1 = barco, 2 = agua atacada (miss), 3 = barco atacado (hit)
        self.board = [[0 for _ in range(size)] for _ in range(size)]
    
    def print(self, title):
        """Imprime el tablero de forma visual"""
        print(f"\n{title}")
        print("   0 1 2 3 4 5 6 7 8 9")
        print("  ---------------------")
        for i in range(self.size):
            row = f"{i}| "
            for j in range(self.size):
                if self.board[i][j] == 0:  # Agua
                    row += "~ "
                elif self.board[i][j] == 1:  # Barco
                    row += "B "
                elif self.board[i][j] == 2:  # Agua atacada (miss)
                    row += "O "
                elif self.board[i][j] == 3:  # Barco atacado (hit)
                    row += "X "
            print(row)
        print()
    
    def update_cell(self, x, y, value):
        """Actualiza una celda del tablero"""
        if 0 <= x < self.size and 0 <= y < self.size:
            self.board[y][x] = value
            return True
        return False
