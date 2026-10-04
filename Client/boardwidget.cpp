#include "boardwidget.h"
#include "gameengine.h"

#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent)
    , m_engine(nullptr)
    , m_myColor(GameEngine::EMPTY)
    , m_myTurn(false)
    , m_hoverRow(-1)
    , m_hoverCol(-1)
{
    setMouseTracking(true);   // 打开鼠标追踪：不点击也能收到 mouseMoveEvent
}

// ========== 外部接口 ==========

void BoardWidget::setEngine(const GameEngine* engine)
{
    m_engine = engine;
    update();                 // 触发重绘
}

void BoardWidget::setMyColor(int color)
{
    m_myColor = color;
}

void BoardWidget::setMyTurn(bool isMyTurn)
{
    m_myTurn = isMyTurn;
}

// ========== 坐标计算 ==========

int BoardWidget::cellSize() const
{
    // 格子边长 = 控件短边 ÷ 15（14 个区间长度 + 两边各 0.5 格留白）
    int w = width();
    int h = height();
    return qMin(w, h) / 15;
}

int BoardWidget::startX() const
{
    int sz = cellSize();
    // 棋盘居中：左边留白 = (控件宽 - 14 格区间宽度) / 2
    return (width() - 14 * sz) / 2;
}

int BoardWidget::startY() const
{
    int sz = cellSize();
    return (height() - 14 * sz) / 2;
}

QPointF BoardWidget::cellToPixel(int row, int col) const//这个QPointF是一种坐标结构体
{
    int sz = cellSize();
    return QPointF(startX() + col * sz, startY() + row * sz);
}

void BoardWidget::pixelToCell(const QPoint& pt, int& row, int& col) const
{
    int sz = cellSize();
    // 四舍五入到最近的交叉点
    col = qRound(static_cast<double>(pt.x() - startX()) / sz);//static是c++标准强制转化关键字
    row = qRound(static_cast<double>(pt.y() - startY()) / sz);//qRound是一个四舍五入的函数
                                                              //其作用是浮点数四舍五入到整数
}

// ========== 绘制 ==========

void BoardWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);//消除未使用参数的编译器警告

    QPainter painter(this);//绘画类
    painter.setRenderHint(QPainter::Antialiasing, true);  // 抗锯齿

    int sz = cellSize();
    int sx = startX();
    int sy = startY();

    // ---- 1) 绘制背景 ----
    painter.fillRect(rect(), QColor(220, 180, 120));  // 木色背景

    // ---- 2) 绘制网格线 ----
    painter.setPen(QPen(Qt::black, 1));//设置画线的笔参数
    for (int i = 0; i < GameEngine::BOARD_SIZE; ++i) {
        // 横线
        painter.drawLine(sx, sy + i * sz,
                         sx + 14 * sz, sy + i * sz);//从起点画到终点,参数(x1,y1)->(x2,y2)
        // 竖线
        painter.drawLine(sx + i * sz, sy,
                         sx + i * sz, sy + 14 * sz);
    }

    // ---- 3) 绘制星位点（天元 + 四角星位） ----
    painter.setBrush(Qt::black);
    // 星位坐标：(3,3) (3,7) (3,11) (7,3) (7,7) (7,11) (11,3) (11,7) (11,11)
    int stars[9][2] = {
        {3,3}, {3,7}, {3,11},
        {7,3}, {7,7}, {7,11},
        {11,3}, {11,7}, {11,11}
    };
    for (auto& s : stars) {//依次从stars中取出一组数组赋给s
        QPointF p = cellToPixel(s[0], s[1]);//把行列坐标转化为像素坐标
        painter.drawEllipse(p, 3, 3);//画一个圆形在p点，横向与纵向半径都为3，颜色为黑色
    }

    // ---- 4) 绘制棋子 ----
    if (!m_engine) return;

    for (int r = 0; r < GameEngine::BOARD_SIZE; ++r) {
        for (int c = 0; c < GameEngine::BOARD_SIZE; ++c) {
            int piece = m_engine->getCell(r, c);
            if (piece == GameEngine::EMPTY) continue;

            QPointF center = cellToPixel(r, c);//行列转像素坐标
            int radius = sz / 2 - 2;  // 棋子略小于半个格子

            // 渐变让棋子有立体感
            QRadialGradient gradient(center, radius);//这是一个类，径向渐变（从圆心向外扩散的渐变，做立体棋子核心）
            if (piece == GameEngine::BLACK) {
                gradient.setColorAt(0,   QColor(80, 80, 80));//给渐变设置颜色
                gradient.setColorAt(0.7, QColor(30, 30, 30));//第一个参数也相当于
                gradient.setColorAt(1,   Qt::black);
            } else {
                gradient.setColorAt(0,   Qt::white);
                gradient.setColorAt(0.7, QColor(220, 220, 220));
                gradient.setColorAt(1,   QColor(140, 140, 140));
            }

            painter.setBrush(gradient);//填充
            painter.setPen(Qt::NoPen);//不绘制外圈轮廓，nopen
            painter.drawEllipse(center, radius, radius);//画一个圆形，横向半径纵向半径radius
        }
    }

    // ---- 5) 悬停预览（半透明棋子） ----
    if (m_myTurn && m_hoverRow >= 0 && m_hoverCol >= 0
        && m_engine->getCell(m_hoverRow, m_hoverCol) == GameEngine::EMPTY) {

        QPointF center = cellToPixel(m_hoverRow, m_hoverCol);
        int radius = sz / 2 - 2;

        QColor previewColor = (m_myColor == GameEngine::BLACK)
                                  ? QColor(0, 0, 0, 80)   // 半透明黑
                                  : QColor(255, 255, 255, 120);  // 半透明白
        painter.setBrush(previewColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, radius, radius);
    }
}

// ========== 鼠标事件 ==========

void BoardWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_engine) return;

    int row, col;
    pixelToCell(event->pos(), row, col);//把鼠标像素位置转化为最匹配的棋盘位置

    // 越界检查
    if (row < 0 || row >= GameEngine::BOARD_SIZE ||
        col < 0 || col >= GameEngine::BOARD_SIZE) {
        m_hoverRow = -1;
        m_hoverCol = -1;
    } else {
        // 离交叉点太远也不算（容差 = 半个格子内）
        QPointF ideal = cellToPixel(row, col);//算出棋盘位置的像素坐标
        double dist = QLineF(ideal, event->pos()).length();//得出此时鼠标位置和棋盘位置像素的距离
        int sz = cellSize();
        if (dist > sz * 0.4) {//如果超过格子的40%就是太远了，，不用显示
            m_hoverRow = -1;
            m_hoverCol = -1;
        } else {
            // 只有位置变了才触发重绘，避免不必要的刷新，悬停效果
            if (m_hoverRow != row || m_hoverCol != col) {
                m_hoverRow = row;
                m_hoverCol = col;
                update();
            }
        }
    }
}

void BoardWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_engine || !m_myTurn) return;

    int row, col;
    pixelToCell(event->pos(), row, col);

    // 越界检查
    if (row < 0 || row >= GameEngine::BOARD_SIZE ||
        col < 0 || col >= GameEngine::BOARD_SIZE)
        return;

    // 离交叉点太远也忽略
    QPointF ideal = cellToPixel(row, col);
    double dist = QLineF(ideal, event->pos()).length();
    if (dist > cellSize() * 0.4) return;

    // 位置已有棋子
    if (m_engine->getCell(row, col) != GameEngine::EMPTY) return;

    emit clicked(row, col);
}
