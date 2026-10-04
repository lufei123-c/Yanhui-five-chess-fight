#include "gameengine.h"

GameEngine::GameEngine()
{
    reset();
}

void GameEngine::reset()
{
    // 清空棋盘
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            m_board[r][c] = EMPTY;

    m_currentPlayer = BLACK;//重置棋盘，黑棋先手
    m_gameOver = false;
}

bool GameEngine::inBoard(int row, int col) const
{
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool GameEngine::isValidMove(int row, int col) const
{
    if (m_gameOver)        return false;   // 如果这个m_gameOver为true，则游戏已结束，就不能再落子
    if (!inBoard(row, col)) return false;   // 越界
    return m_board[row][col] == EMPTY;      // 如果不是空，说明这个地方已经被落了子了，就不能再落子了，位置必须为空，才能保证这一步合法
}

bool GameEngine::placePiece(int row, int col)
{
    if (!isValidMove(row, col)) return false;
    //这里是落了子，当前是谁下，那么这个棋盘位置的值就是谁，黑是1，白是2
    m_board[row][col] = m_currentPlayer;//用Black和white

    // 切换回合
    m_currentPlayer = (m_currentPlayer == BLACK) ? WHITE : BLACK;

    return true;
}

bool GameEngine::checkWin(int row, int col) const
{
    //此时落了子之后就要检查这个子的横，纵，主队角，副队角有没有成线
    int color = m_board[row][col];
    if (color == EMPTY) return false;

    // 四个方向：横、纵、主对角、副对角
    const int dr[4] = { 1, 0, 1, -1 };//这两个数组是精髓，nb
    const int dc[4] = { 0, 1, 1,  1 };//

    for (int dir = 0; dir < 4; ++dir) {
        int count = 1;  // 算上自身

        // 正方向扫
        for (int i = 1; i < 5; ++i) {
            int r = row + dr[dir] * i;
            int c = col + dc[dir] * i;
            if (inBoard(r, c) && m_board[r][c] == color)
                ++count;
            else
                break;
        }

        // 反方向扫
        for (int i = 1; i < 5; ++i) {
            int r = row - dr[dir] * i;
            int c = col - dc[dir] * i;
            if (inBoard(r, c) && m_board[r][c] == color)
                ++count;
            else
                break;
        }

        if (count >= 5) return true;
    }

    return false;
}

int GameEngine::getCell(int row, int col) const
{
    if (!inBoard(row, col)) return EMPTY;//注意这个inboar是没有超出返回true
    return m_board[row][col];
}

int GameEngine::currentPlayer() const
{
    return m_currentPlayer;
}

bool GameEngine::isGameOver() const
{
    return m_gameOver;
}

const int* GameEngine::boardData() const
{
    return &m_board[0][0];
}
