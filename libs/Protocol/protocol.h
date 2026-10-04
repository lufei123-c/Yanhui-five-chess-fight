#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QStringList>

class Protocol
{
public:
    // ============================================================
    //  编码函数：把游戏数据 → 字符串，准备通过网络发送
    //  用法：Socket->write( Protocol::xxxMsg(...).toUtf8() )
    // ============================================================

    // 发 "welcome:black" 或 "welcome:white" 给新连上的玩家
    // color: GameEngine::BLACK(1) 或 WHITE(2)  因为black和white都是静态成员变量，所以可以直接类名访问
    static QString welcomeMsg(int color);//静态函数只能访问自己的静态成员变量

    // 发 "turn:black" 或 "turn:white"，通知所有人当前该谁下
    static QString turnMsg(int color);

    // 发 "move:7,7,1"，广播落子结果
    // row=行号, col=列号, color=黑1还是白2
    static QString moveMsg(int row, int col, int color);

    // 发 "result:win,black"，宣布胜方
    static QString winMsg(int color);

    // 发 "result:leave"，通知对方：你家对手跑了
    static QString leaveMsg();

    // 发 "error:没轮到你" 之类的错误提示
    static QString errorMsg(const QString& reason);

    // 客户端落子时用：把 (行,列) 拼成 "move:3,5"
    // 和服务端用的 moveMsg 区别：这个不带颜色
    static QString encodeMove(int row, int col);

    // ============================================================
    //  解析函数：把收到的字符串 → 游戏数据
    //  参数用引用(&)往外传结果，返回值 bool 表示解析成功/失败
    // ============================================================

    // 提取消息类型。例如 "move:7,7" → 返回 "move"
    static QString parseType(const QString& msg);

    // 解析 "welcome:black" → color 被设为 BLACK，返回 true
    static bool parseWelcome(const QString& msg, int& color);

    // 解析 "turn:white" → color 被设为 WHITE，返回 true
    static bool parseTurn(const QString& msg, int& color);

    // 解析 "move:7,7,1" → row=7, col=7, color=1，返回 true
    // 也能解析 "move:7,7"（客户端发来的，没带颜色），此时 color=0(EMPTY)
    static bool parseMove(const QString& msg, int& row, int& col, int& color);

    // 解析 "result:win,black" → result="win",  color=BLACK
    // 解析 "result:leave"    → result="leave", color=EMPTY
    static bool parseResult(const QString& msg, QString& result, int& color);

    // ============================================================
    //  工具函数
    // ============================================================

    // BLACK(1)→"black",  WHITE(2)→"white"
    static QString colorName(int color);

    // "black"→BLACK(1),  "white"→WHITE(2),  其他→EMPTY(0)
    static int    parseColor(const QString& str);
};

#endif // PROTOCOL_H
