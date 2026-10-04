#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>

class GameEngine;

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget* parent = nullptr);

    // 外部通过这两个函数获取 / 设置棋盘数据
    void setEngine(const GameEngine* engine);   // 绑定棋盘数据源
    void setMyColor(int color);                 // 设置自己的棋色（黑/白）
    void setMyTurn(bool isMyTurn);              // 是否轮到自己

signals:
    void clicked(int row, int col);             // 用户点击了(row, col)位置

protected:
    //下面的三个函数后面的override是重写父函数的虚函数的声明，是个关键字
    void paintEvent(QPaintEvent* event) override;       // 画棋盘
    void mouseReleaseEvent(QMouseEvent* event) override; // 处理点击
    void mouseMoveEvent(QMouseEvent* event) override;    // 处理鼠标悬停

private:
    // 坐标计算
    int  cellSize() const;       // 每个格子的像素大小
    int  startX() const;         // 棋盘左上角 X
    int  startY() const;         // 棋盘左上角 Y
    QPointF cellToPixel(int row, int col) const;  // 棋盘坐标→像素坐标
    void  pixelToCell(const QPoint& pt, int& row, int& col) const; // 像素→棋盘坐标

    const GameEngine* m_engine;   // 棋盘数据（只读，不拥有所有权）
    int  m_myColor;               // 我是黑棋还是白棋
    bool m_myTurn;                // 现在轮到我了吗
    int  m_hoverRow;              // 鼠标悬停的行（-1 表示无）
    int  m_hoverCol;              // 鼠标悬停的列
};

#endif // BOARDWIDGET_H
