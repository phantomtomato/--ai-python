// Copyright (c) 2025 phantomtomato
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

import pygame
import threading
import ctypes
import os

# 获取当前脚本所在目录
script_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(script_dir, "gobang_ai.dll")
lib = ctypes.CDLL(dll_path)

SIZE = 15
COLS = "ABCDEFGHIJKLMNO"
EMPTY, BLACK, WHITE = "·●○"

# 用于存储 AI 计算结果的全局变量
ai_move = None
ai_move_ready = threading.Event()

GRID_WIDTH = 40
MARGIN = 50

# 定义颜色
BLACK_COLOR = (0, 0, 0)
WHITE_COLOR = (255, 255, 255)

# 定义 C 函数参数和返回值类型
GRID_ARRAY = ctypes.c_int * SIZE * SIZE
INT_PTR = ctypes.POINTER(ctypes.c_int)
CALLBACK_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_char_p)

lib.get_ai_move.argtypes = [
    GRID_ARRAY,
    INT_PTR,
    INT_PTR,
    CALLBACK_TYPE
]

def print_callback(message):
    try:
        message = message.decode('gbk')
    except UnicodeDecodeError:
        print("解码失败，使用默认编码处理")
        message = str(message)
    print(message.strip())

callback = CALLBACK_TYPE(print_callback)

def ai_hell(b):
    board = GRID_ARRAY()
    for r in range(SIZE):
        for c in range(SIZE):
            if b[r][c] == BLACK:
                board[r][c] = 1
            elif b[r][c] == WHITE:
                board[r][c] = 2
    row = ctypes.c_int()
    col = ctypes.c_int()
    try:
        lib.get_ai_move(board, ctypes.byref(row), ctypes.byref(col), callback)
        print(f"AI 计算得到的落子位置: ({row.value}, {col.value})")
        return row.value, col.value
    except Exception as e:
        print(f"调用 DLL 函数出错: {e}")
        return None, None

def new_board():
    return [[EMPTY] * SIZE for _ in range(SIZE)]

def print_board(b):
    print("   " + " ".join(COLS))
    for r in range(SIZE - 1, -1, -1):
        print(f"{r + 1:2d} " + " ".join(b[r]))
    print()

def parse(move):
    m = __import__('re').fullmatch(r"([A-Oa-o])(\d{1,2})", move.strip())
    if not m:
        return None
    col = ord(m.group(1).upper()) - ord('A')
    row = int(m.group(2)) - 1
    return (row, col) if 0 <= row < SIZE and 0 <= col < SIZE else None

def in_range(x, y):
    return 0 <= x < SIZE and 0 <= y < SIZE

def check_win(b, r, c, color):
    directions = ((1, 0), (0, 1), (1, 1), (1, -1))
    positions = [(r, c)] if r is not None and c is not None else [(i, j) for i in range(SIZE) for j in range(SIZE) if b[i][j] == color]
    
    for row, col in positions:
        for dx, dy in directions:
            cnt = 1
            for k in (1, -1):
                dr, dc = dx * k, dy * k
                nr, nc = row + dr, col + dc
                while in_range(nr, nc) and b[nr][nc] == color:
                    cnt += 1
                    nr += dr
                    nc += dc
            if cnt >= 5:
                return True
    return False

def legal_moves(b):
    return [(r, c) for r in range(SIZE) for c in range(SIZE) if b[r][c] == EMPTY]

def ai_thread(board, ai_func):
    global ai_move
    ai_move = ai_func(board)
    ai_move_ready.set()

def draw_board(screen, board, font):
    screen.fill(BLACK_COLOR)
    for r in range(SIZE):
        for c in range(SIZE):
            text = font.render(board[r][c], True, WHITE_COLOR)
            screen.blit(text, text.get_rect(center=(MARGIN + c * GRID_WIDTH, MARGIN + r * GRID_WIDTH)))
    for c in range(SIZE):
        col_text = font.render(COLS[c], True, WHITE_COLOR)
        screen.blit(col_text, col_text.get_rect(center=(MARGIN + c * GRID_WIDTH, MARGIN - 20)))
    for r in range(SIZE):
        row_text = font.render(str(SIZE - r), True, WHITE_COLOR)
        screen.blit(row_text, row_text.get_rect(center=(MARGIN - 20, MARGIN + r * GRID_WIDTH)))

def get_board_pos(x, y):
    best = min(
        ((r, c) for r in range(SIZE) for c in range(SIZE)),
        key=lambda rc: (x - MARGIN - rc[1] * GRID_WIDTH) ** 2 + (y - MARGIN - rc[0] * GRID_WIDTH) ** 2
    )
    distance = ((x - MARGIN - best[1] * GRID_WIDTH) ** 2 + (y - MARGIN - best[0] * GRID_WIDTH) ** 2) ** 0.5
    return best if distance <= GRID_WIDTH / 2 else None

def main():
    pygame.init()
    size = MARGIN * 2 + (SIZE - 1) * GRID_WIDTH
    screen = pygame.display.set_mode((size, size))
    pygame.display.set_caption("五子棋")
    font = pygame.font.Font(os.path.join(script_dir, "SimHei.ttf"), 36)

    board = new_board()
    move_history = []
    ai_func = ai_hell
    turn = WHITE
    ai_thread_active = False

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return
            elif event.type == pygame.MOUSEBUTTONDOWN and turn == WHITE:
                pos = get_board_pos(*event.pos)
                if pos and board[pos[0]][pos[1]] == EMPTY:
                    r, c = pos
                    board[r][c] = WHITE
                    move_history.append((r, c, WHITE))
                    if check_win(board, r, c, WHITE):
                        print_board(board)
                        print("White wins!")
                        return
                    turn = BLACK
                    ai_move_ready.clear()
                    ai_thread_active = True
                    threading.Thread(target=ai_thread, args=(board, ai_func)).start()
            elif event.type == pygame.KEYDOWN and event.key == pygame.K_z:
                if move_history and (turn != BLACK or not ai_thread_active):
                    r, c, color = move_history.pop()
                    board[r][c] = EMPTY
                    turn = WHITE if color == BLACK else BLACK

        if turn == BLACK and ai_thread_active and ai_move_ready.is_set():
            if ai_move and board[ai_move[0]][ai_move[1]] == EMPTY:
                r, c = ai_move
                board[r][c] = BLACK
                move_history.append((r, c, BLACK))
                print_board(board)
                if check_win(board, r, c, BLACK):
                    print("Black wins!")
                    return
                turn = WHITE
            else:
                print("AI 落子失败，重新计算" if ai_move else "AI 计算结果为空，重新计算")
                ai_move_ready.clear()
                ai_thread_active = True
                threading.Thread(target=ai_thread, args=(board, ai_func)).start()
            ai_thread_active = False
            ai_move_ready.clear()

        draw_board(screen, board, font)
        pygame.display.flip()

if __name__ == "__main__":
    main()
