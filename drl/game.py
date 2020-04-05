import numpy as np


class Gobang:
    def __init__(self):
        self.board = np.zeros((15+2, 15+2))
        for i in range(16):
            # 3 means boarder
            self.board[0, i] = self.board[i, 0] = self.board[16, i] = self.board[i, 16] = 3

        self.curr_player = 1
        self.steps = 0
        self.latest_move = np.array([-1, -1])

    def new_game(self):
        self.board = np.zeros((15+2, 15+2))
        for i in range(16):
            self.board[0, i] = self.board[i, 0] = self.board[16, i] = self.board[i, 16] = 3

        self.curr_player = 1
        self.steps = 0
        self.latest_move = np.array([-1, -1])

    def set(self, boarder_board, curr_player, steps):
        self.board = boarder_board.copy()
        self.curr_player = curr_player
        self.steps = steps

    def curr_board(self, boarder=False):
        return self.board[1:16, 1:16] if not boarder else self.board

    def actions(self):
        return np.argwhere(self.curr_board() == 0)

    def place_chess(self, coord):
        self.board[coord[0]+1, coord[1]+1] = self.curr_player
        self.curr_player *= -1
        self.steps += 1
        self.latest_move[0], self.latest_move[1] = coord

        return self._terminal_test(), self.curr_board()

    def _terminal_test(self):
        if self._terminal_test_dir(np.array([1, 0])):
            return True
        if self._terminal_test_dir(np.array([0, 1])):
            return True
        if self._terminal_test_dir(np.array([1, 1])):
            return True
        if self._terminal_test_dir(np.array([-1, 1])):
            return True

        return False

    def _terminal_test_dir(self, dir):
        i, coord = 1, self.latest_move.copy()
        coord += dir
        while self.board[coord[0], coord[1]] == -self.curr_player:
            i += 1
            coord += dir

        coord = self.latest_move.copy()
        coord -= dir
        while self.board[coord[0], coord[1]] == -self.curr_player:
            i += 1
            coord -= dir

        if i >= 5:
            return True
        else:
            return False
