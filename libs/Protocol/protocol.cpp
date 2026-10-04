#include "protocol.h"
#include "gameengine.h"

// ========== 编码 ==========

QString Protocol::welcomeMsg(int color)// 发 "welcome:black" 或 "welcome:white" 给新连上的玩家
{
    return QString("welcome:%1").arg(colorName(color));
}

QString Protocol::turnMsg(int color)// 发 "turn:black" 或 "turn:white"，通知所有人当前该谁下
{
    return QString("turn:%1").arg(colorName(color));
}

QString Protocol::moveMsg(int row, int col, int color)// 发 "move:7,7,1"，广播落子结果
                                                      // row=行号, col=列号, color=黑1还是白2
{
    return QString("move:%1,%2,%3").arg(row).arg(col).arg(color);
}

QString Protocol::winMsg(int color)// 发 "result:win,black"，宣布胜方
{
    return QString("result:win,%1").arg(colorName(color));
}

QString Protocol::leaveMsg()// 发 "result:leave"，通知对方：你家对手跑了
{
    return QString("result:leave");
}

QString Protocol::errorMsg(const QString& reason)// 发 "error:没轮到你" 之类的错误提示
{
    return QString("error:%1").arg(reason);
}

QString Protocol::encodeMove(int row, int col)// 客户端落子时用：把 (行,列) 拼成 "move:3,5"
                                              // 和服务端用的 moveMsg 区别：这个不带颜色
{
    return QString("move:%1,%2").arg(row).arg(col);
}

// ========== 解析 ==========

QString Protocol::parseType(const QString& msg)//获取动作类型
{
    int idx = msg.indexOf(':');
    if (idx < 0) return QString();
    return msg.left(idx);                // 冒号前的内容就是类型
}

bool Protocol::parseWelcome(const QString& msg, int& color)
{
    if (!msg.startsWith("welcome:")) return false;//这个startswith函数用来判断是否是指定前缀开头
    color = parseColor(msg.mid(8));      // 跳过 "welcome:"，截取第八位及以后的内容
    return color != GameEngine::EMPTY;
}

bool Protocol::parseTurn(const QString& msg, int& color)
{
    if (!msg.startsWith("turn:")) return false;
    color = parseColor(msg.mid(5));      // 跳过 "turn:"
    return color != GameEngine::EMPTY;
}

bool Protocol::parseMove(const QString& msg, int& row, int& col, int& color)
{
    if (!msg.startsWith("move:")) return false;

    QString body = msg.mid(5);            // "7,7" 或 "7,7,1"
    QStringList parts = body.split(',');//按逗号分割字符串形成一个字符串数组（Qstringlist）
    if (parts.size() < 2) return false;

    row   = parts[0].toInt();
    col   = parts[1].toInt();
    color = (parts.size() >= 3) ? parts[2].toInt() : GameEngine::EMPTY;

    return true;
}

bool Protocol::parseResult(const QString& msg, QString& result, int& color)
{
    if (!msg.startsWith("result:")) return false;

    QString body = msg.mid(7);           // "win,black" 或 "leave"

    if (body == "leave") {
        result = "leave";
        color  = GameEngine::EMPTY;
        return true;
    }
    if (body.startsWith("win,")) {
        result = "win";
        color  = parseColor(body.mid(4));
        return true;
    }
    return false;
}

// ========== 工具 ==========

QString Protocol::colorName(int color)//把数字转化为字符串
{
    return (color == GameEngine::BLACK) ? "black" : "white";
}

int Protocol::parseColor(const QString& str)//把字符串转化为数字
{
    if (str == "black") return GameEngine::BLACK;
    if (str == "white") return GameEngine::WHITE;
    return GameEngine::EMPTY;
}
