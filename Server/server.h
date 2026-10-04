#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QList>

class QTcpServer;
class QTcpSocket;
class GameEngine;

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);//这个explicit是防止隐式转化，让server只能显示转化
                                              //就是说这个server只能显示的进行实例化对象
    ~Server();

    bool start(quint16 port);        // 开始监听指定端口
    void stop();                     // 停止服务

private slots:
    void onNewConnection();          // 有新客户端连进来
    void onReadyRead();              // 收到客户端发来的数据
    void onDisconnected();           // 有客户端断开

private:
    void broadcast(const QString& msg);          // 群发给所有在线玩家
    void sendTo(QTcpSocket* sock, const QString& msg); // 发给指定玩家
    void resetGame();                            // 重置对局
    void cleanupSocket(QTcpSocket* sock);        // 清理断开的连接

    QTcpServer*       m_tcpServer;     // 监听器
    QList<QTcpSocket*> m_players;      // 在线玩家列表，[0]=黑棋, [1]=白棋
    GameEngine*       m_engine;        // 唯一的棋盘权威
    bool              m_gameStarted;   // 是否已满两人，对局进行中
};

#endif // SERVER_H
