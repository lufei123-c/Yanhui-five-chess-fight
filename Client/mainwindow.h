#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gameengine.h"       // 用到了里面的常量 BLACK/WHITE

class BoardWidget;
class QTcpSocket;
class QLineEdit;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onConnect();              // 点击"连接"按钮
    void onConnected();            // 连接成功
    void onDisconnected();         // 连接断开
    void onReadyRead();            // 收到服务端消息
    void onBoardClicked(int row, int col);  // 玩家点击了棋盘

private:
    void setupUI();                // 构建界面布局
    void processMessage(const QString& msg);  // 解析并处理一条消息

    // UI 控件
    BoardWidget*  m_board;         // 棋盘绘制区
    QLineEdit*    m_ipEdit;        // IP 输入框
    QLineEdit*    m_portEdit;      // 端口输入框
    QPushButton*  m_connectBtn;    // 连接按钮
    QLabel*       m_statusLabel;   // 状态提示（"请连接"/"该你走了"/"黑棋胜"）

    // 网络
    QTcpSocket*   m_socket;        // 与服务端的连接

    // 本地游戏状态
    GameEngine    m_engine;         // 本地棋盘镜像（对象，不是指针）
    int           m_myColor;        // 我是黑还是白
    bool          m_connected;      // 是否已连上服务端
};

#endif // MAINWINDOW_H
