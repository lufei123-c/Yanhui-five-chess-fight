#include "mainwindow.h"
#include "boardwidget.h"
#include "protocol.h"

#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_board(nullptr)
    , m_ipEdit(nullptr)
    , m_portEdit(nullptr)
    , m_connectBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_socket(new QTcpSocket(this))      // socket 的父对象设为主窗口
    , m_myColor(GameEngine::EMPTY)
    , m_connected(false)
{
    setupUI();

    // ===== 网络信号绑定 =====
    connect(m_socket, &QTcpSocket::connected,
            this, &MainWindow::onConnected);//连接
    connect(m_socket, &QTcpSocket::disconnected,
            this, &MainWindow::onDisconnected);//断开连接
    connect(m_socket, &QTcpSocket::readyRead,
            this, &MainWindow::onReadyRead);//有数据可读socket

    // ===== 棋盘点击信号 =====
    connect(m_board, &BoardWidget::clicked,
            this, &MainWindow::onBoardClicked);
}

MainWindow::~MainWindow()
{
}

// ========== 界面搭建 ==========

void MainWindow::setupUI()
{
    auto* central = new QWidget(this);//原理是说在mainwindow中心区域用qwidget填充，不能单独只用控件填充

    setCentralWidget(central);//将central这个填充空白面板设置在mainwindow的中心区域

    auto* mainLayout = new QVBoxLayout(central);//垂直布局管理器，它会自适应改动布局

    // ---- 第一行：连接面板 ----
    auto* topLayout = new QHBoxLayout();//水平布局管理器，注意看名字
    topLayout->addWidget(new QLabel("IP:"));//创建一个标签
    m_ipEdit = new QLineEdit("127.0.0.1");//创建一个输入框，默认127.0.0.1
    topLayout->addWidget(m_ipEdit);//把输入框加入水平布局

    topLayout->addWidget(new QLabel("端口:"));//创建窗口标签
    m_portEdit = new QLineEdit("8888");//创建一个输入框，显示端口号，默认8888
    topLayout->addWidget(m_portEdit);//加入水平布局

    m_connectBtn = new QPushButton("连接");//创建按钮，内容显示连接
    topLayout->addWidget(m_connectBtn);//加入水平布局
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnect);//绑定这个按钮

    mainLayout->addLayout(topLayout);//把刚刚的水平布局加入整个垂直布局当中

    // ---- 第二行：棋盘 ----
    m_board = new BoardWidget(this);//创建本地棋盘
    m_board->setMinimumSize(450, 450);//设置最小尺寸，防止拉伸窗口时棋盘太小
    m_board->setEngine(&m_engine);          // 绑定棋盘数据源
    mainLayout->addWidget(m_board, 1);      // stretch=1 占满剩余空间,能够让棋盘自由拉伸

    // ---- 第三行：状态栏 ----
    m_statusLabel = new QLabel("未连接");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 16px; padding: 8px;");
    mainLayout->addWidget(m_statusLabel);

    setWindowTitle("五子棋");
    resize(550, 650);//设置窗口初始大小
}

// ========== 连接 ==========

void MainWindow::onConnect()
{
    if (m_connected) {//
        // 已连接 → 断开
        m_socket->disconnectFromHost();
        return;
    }

    QString ip   = m_ipEdit->text().trimmed();
    quint16 port = m_portEdit->text().toUShort();

    m_statusLabel->setText("正在连接...");
    m_socket->connectToHost(ip, port);
}

void MainWindow::onConnected()
{
    m_connected = true;//设置m_connected为true
    m_connectBtn->setText("断开");
    m_statusLabel->setText("已连接，等待对手...");
    m_ipEdit->setEnabled(false);
    m_portEdit->setEnabled(false);
}

void MainWindow::onDisconnected()
{
    m_connected = false;
    m_connectBtn->setText("连接");
    m_statusLabel->setText("连接已断开");
    m_ipEdit->setEnabled(true);
    m_portEdit->setEnabled(true);

    m_engine.reset();
    m_myColor = GameEngine::EMPTY;
    m_board->setMyTurn(false);
    m_board->update();
}

// ========== 收到数据 ==========

void MainWindow::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString msg = QString::fromUtf8(m_socket->readLine().trimmed());
        processMessage(msg);
    }
}

void MainWindow::processMessage(const QString& msg)
{
    QString type = Protocol::parseType(msg);

    // ------ welcome: 告知棋色 ------
    if (type == "welcome") {
        int color;
        if (Protocol::parseWelcome(msg, color)) {
            m_myColor = color;
            m_board->setMyColor(color);
            m_statusLabel->setText(
                QString("你是%1棋，等待对手...")
                    .arg(Protocol::colorName(color)));
        }
    }
    // ------ turn: 通知该谁走 ------
    else if (type == "turn") {
        int color;
        if (Protocol::parseTurn(msg, color)) {
            bool isMe = (color == m_myColor);
            m_board->setMyTurn(isMe);
            m_statusLabel->setText(
                isMe ? "该你走了" : "等待对手落子...");
        }
    }
    // ------ move: 广播落子 ------
    else if (type == "move") {
        int row, col, color;
        if (Protocol::parseMove(msg, row, col, color)) {
            m_engine.placePiece(row, col);
            m_board->update();           // 触发重绘
        }
    }
    // ------ result: 对局结果 ------
    else if (type == "result") {
        QString result;
        int color;
        if (Protocol::parseResult(msg, result, color)) {
            if (result == "win") {
                m_statusLabel->setText(
                    QString("%1棋获胜！")
                        .arg(Protocol::colorName(color)));
            } else if (result == "leave") {
                m_statusLabel->setText("对手已离开");
            }
            m_board->setMyTurn(false);
        }
    }
}

// ========== 玩家点击棋盘 ==========

void MainWindow::onBoardClicked(int row, int col)
{
    if (!m_connected) return;

    QString moveCmd = Protocol::encodeMove(row, col);
    m_socket->write((moveCmd + "\n").toUtf8());
}
