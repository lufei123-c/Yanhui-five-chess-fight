#include "server.h"
#include "gameengine.h"
#include "protocol.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

Server::Server(QObject* parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))      // 创建监听器，父对象=this（Server销毁时自动回收）
    , m_engine(new GameEngine)               // 创建棋盘引擎
    , m_gameStarted(false)
{
    // 新连接信号 → 槽函数
    connect(m_tcpServer, &QTcpServer::newConnection,
            this, &Server::onNewConnection);
}

Server::~Server()
{
    stop();
    delete m_engine;
}

// ========== 启动 / 停止 ==========

bool Server::start(quint16 port)
{
    // 监听本机所有网卡、指定端口
    if (m_tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "服务端已启动，端口:" << port;
        return true;
    }
    qDebug() << "监听失败:" << m_tcpServer->errorString();
    return false;
}

void Server::stop()
{
    // 断开所有客户端
    for (auto* sock : m_players)
        cleanupSocket(sock);
    m_players.clear();//清空m_players这个列表内的所有指针
    m_tcpServer->close();
}

// ========== 槽函数 ==========

void Server::onNewConnection()
{
    while (m_tcpServer->hasPendingConnections()) {//是否等待连接队列为空
        QTcpSocket* sock = m_tcpServer->nextPendingConnection();//从等待队列取出一条连接，返回socket指针

        // 人满了 → 拒绝
        if (m_players.size() >= 2) {
            sendTo(sock, Protocol::errorMsg("房间已满"));
            sock->disconnectFromHost();
            sock->deleteLater();
            return;
        }

        // 绑定信号：收到数据 / 断线
        connect(sock, &QTcpSocket::readyRead,
                this, &Server::onReadyRead);
        connect(sock, &QTcpSocket::disconnected,
                this, &Server::onDisconnected);

        // 加入列表，分配棋色
        m_players.append(sock);
        int color = (m_players.size() == 1) ? GameEngine::BLACK : GameEngine::WHITE;
        sendTo(sock, Protocol::welcomeMsg(color));

        qDebug() << "新玩家接入，棋色:" << Protocol::colorName(color)
                 << "，当前人数:" << m_players.size();

        // 第二个玩家接入 → 对局开始
        if (m_players.size() == 2) {
            m_gameStarted = true;
            m_engine->reset();
            broadcast(Protocol::turnMsg(GameEngine::BLACK));   // 通知黑棋先手
            qDebug() << "对局开始，黑棋先手";
        }
    }
}

void Server::onReadyRead()
{
    // sender() 返回发出信号的 QObject*，转成 QTcpSocket*
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());//这个sender函数：返回触发当前这个槽函数的信号发送者对象指针。
                                                        //qobject_cast<QTcpSocket*>()这是一个强制转化，目标类型是QTcpSocket*,
                                                        //括号里的类型是Qobject*类型或者可以隐式转化为Qobject*的类型
    if (!sock) return;

    // 逐行读取（协议里每条指令以换行结束）
    while (sock->canReadLine()) {
        QByteArray data = sock->readLine().trimmed();
        QString msg = QString::fromUtf8(data);//将字节流(QByteArray)转化为字符串
        qDebug() << "收到:" << msg;

        int row, col, dummyColor;
        if (!Protocol::parseMove(msg, row, col, dummyColor))//判断是不是移动指令
            continue;   // 不是 move 指令，跳过

        // ---- 下面是落子处理 ----

        // 1) 对局开始了没？
        if (!m_gameStarted) {
            sendTo(sock, Protocol::errorMsg("对局尚未开始"));
            continue;
        }

        // 2) 找到发送者在列表中的位置：[0]→黑棋, [1]→白棋
        int playerIdx = m_players.indexOf(sock);//注意这个indexof和qstring里的indexof是不一样的
                                                //这里的qlist的indexof是找到目标在列表里面的位置，也就是下标
        if (playerIdx < 0) continue;
        int playerColor = (playerIdx == 0) ? GameEngine::BLACK : GameEngine::WHITE;

        // 3) 轮到你了吗？
        if (m_engine->currentPlayer() != playerColor) {
            sendTo(sock, Protocol::errorMsg("还没轮到你"));
            continue;
        }

        // 4) 落子
        if (!m_engine->placePiece(row, col)) {
            sendTo(sock, Protocol::errorMsg("无效落子"));
            continue;
        }

        // 5) 广播落子结果给双方
        broadcast(Protocol::moveMsg(row, col, playerColor));

        // 6) 胜负判定
        if (m_engine->checkWin(row, col)) {
            m_gameStarted = false;
            broadcast(Protocol::winMsg(playerColor));
            qDebug() << Protocol::colorName(playerColor) << "获胜！";
        } else {
            // 没赢 → 通知下一个玩家该走了
            broadcast(Protocol::turnMsg(m_engine->currentPlayer()));
        }
    }
}

void Server::onDisconnected()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;

    qDebug() << "玩家断开连接";

    int idx = m_players.indexOf(sock);
    cleanupSocket(sock);
    m_players.removeAll(sock);

    // 对局中有人跑路 → 通知剩下的人
    if (m_gameStarted && !m_players.isEmpty()) {
        broadcast(Protocol::leaveMsg());
    }

    resetGame();
}

// ========== 工具函数 ==========

void Server::broadcast(const QString& msg)
{
    // 向列表里所有人发同一条消息
    for (auto* sock : m_players)
        sendTo(sock, msg);
}

void Server::sendTo(QTcpSocket* sock, const QString& msg)
{
    if (sock && sock->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = (msg + "\n").toUtf8();   // 每条指令末尾加换行
        sock->write(data);
    }
}

void Server::resetGame()
{
    m_engine->reset();
    m_gameStarted = false;
}

void Server::cleanupSocket(QTcpSocket* sock)
{
    disconnect(sock, nullptr, this, nullptr);   // 断开所有与本 Server 相关的信号槽
    sock->disconnectFromHost();//主动向对端客户端发起TCP 挥手断开连接。
                               //仅仅发起断开请求，不会立刻断开，是异步操作
    if (sock->state() != QAbstractSocket::UnconnectedState)
        sock->waitForDisconnected(1000);         // 等最多 1 秒
    sock->deleteLater();                         // 延迟删除，让 Qt 事件循环安全回收
}
