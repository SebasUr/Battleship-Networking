# gamestate.py

class GameState:
    def __init__(self):
        self.exit_flag = False
        self.my_turn = False
        self.last_attack_coords = None
        self.my_board = [[0 for _ in range(10)] for _ in range(10)]
        self.enemy_board = [[0 for _ in range(10)] for _ in range(10)]
