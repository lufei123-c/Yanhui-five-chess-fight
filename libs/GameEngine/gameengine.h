#ifndef GAMEENGINE_H
#define GAMEENGINE_H

class GameEngine
{
public:
    //用static const修饰保证这些棋盘的关键值不能被改动，并且是在本类里面共享同一份数据
    static const int BOARD_SIZE = 15;   // 15×15 棋盘
    static const int EMPTY  = 0;        // 空位
    static const int BLACK  = 1;        // 黑棋
    static const int WHITE  = 2;        // 白棋

    GameEngine();
    //有些成员函数后面加了const就是说这个函数不能修改成员变量，它只能做读取操作，不能修改
    //而且如果用const修饰的对象它只能够调用后面用const修饰的函数，其他的不加const的成员函数不能调用
    void reset();                               // 重置棋局
    bool isValidMove(int row, int col) const;   // 落子是否合法
    bool placePiece(int row, int col);          // 落子（自动切换回合）
    bool checkWin(int row, int col) const;     // 胜负判定

    int  getCell(int row, int col) const;       // 读取某格
    int  currentPlayer() const;                 // 当前该谁走
    bool isGameOver() const;                    // 是否已结束
    const int* boardData() const;               // 棋盘数据首地址

private:
    int  m_board[15][15];//代表棋盘
    int  m_currentPlayer;//记录现在轮到谁下
    bool m_gameOver;//记录这局棋有没有结束

    bool inBoard(int row, int col) const;//这个是用来判断传进来的坐标有没有越界的函数 const修饰，只读不运行修改
};

#endif // GAMEENGINE_H
