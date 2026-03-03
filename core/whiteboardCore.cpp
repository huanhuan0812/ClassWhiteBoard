#include "whiteboardCore.h"
#include <QMouseEvent>
#include <QUndoCommand>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QApplication>
#include <QPainter>
#include <QVector2D>

// 绘制命令（修改为存储点集）
class DrawCommand : public QUndoCommand
{
public:
    DrawCommand(QGraphicsScene *scene, QGraphicsPathItem *pathItem, const QList<QPointF> &points)
        : m_scene(scene), m_pathItem(pathItem), m_points(points)
    {
        setText("Draw");
    }

    void undo() override {
        if (m_pathItem) m_scene->removeItem(m_pathItem);
    }

    void redo() override {
        if (m_pathItem) m_scene->addItem(m_pathItem);
    }

    QList<QPointF> getPoints() const { return m_points; }

private:
    QGraphicsScene *m_scene;
    QGraphicsPathItem *m_pathItem;
    QList<QPointF> m_points;  // 保存原始点用于可能的编辑
};

// 擦除命令
class EraseCommand : public QUndoCommand
{
public:
    EraseCommand(QGraphicsScene *scene, QGraphicsPathItem *pathItem)
        : m_scene(scene), m_pathItem(pathItem)
    {
        setText("Erase");
    }

    void undo() override {
        if (m_pathItem) m_scene->addItem(m_pathItem);
    }

    void redo() override {
        if (m_pathItem) m_scene->removeItem(m_pathItem);
    }

private:
    QGraphicsScene *m_scene;
    QGraphicsPathItem *m_pathItem;
};

whiteboardCore::whiteboardCore(QWidget *parent)
    : QGraphicsView(parent), drawing(false), currentMode(PenMode), 
      eraserWidth(60), eraserHeight(100),
      simplifyTolerance(5.0), useSimplification(true)  // 默认启用抽稀
{
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 800, 600);
    setScene(scene);
    setStyleSheet("background-color: #12520a;");
    setMouseTracking(true);

    pen = QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    // 创建板擦图片
    eraserPixmap = QPixmap(eraserWidth, eraserHeight);
    eraserPixmap.fill(Qt::transparent);
    
    // 绘制板擦外观
    QPainter painter(&eraserPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制板擦主体（灰色矩形）
    painter.fillRect(0, 0, eraserWidth, eraserHeight, QColor(200, 200, 200, 200));
    
    // 绘制边框
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(1, 1, eraserWidth-2, eraserHeight-2);
    
    // 绘制板擦纹理（斜线）
    painter.setPen(QPen(QColor(150, 150, 150, 150), 1, Qt::DashLine));
    for (int i = 0; i < eraserWidth + eraserHeight; i += 10) {
        painter.drawLine(i, 0, i - eraserHeight, eraserHeight);
    }
    
    // 添加文字
    painter.setPen(QPen(Qt::black, 1));
    painter.setFont(QFont("Arial", 8));
    painter.drawText(eraserPixmap.rect(), Qt::AlignCenter, "ERASER");
    
    painter.end();

    // 创建板擦光标
    eraserCursor = new QGraphicsPixmapItem(eraserPixmap);
    eraserCursor->setVisible(false);
    // 设置图片中心为光标热点
    eraserCursor->setOffset(-eraserWidth/2, -eraserHeight/2);
    scene->addItem(eraserCursor);

    undoStack = new QUndoStack(this);
    setCursor(Qt::CrossCursor);
}

void whiteboardCore::setSimplifyTolerance(qreal tolerance)
{
    simplifyTolerance = tolerance;
}

void whiteboardCore::setUseSimplification(bool enable)
{
    useSimplification = enable;
}

void whiteboardCore::setDrawMode(DrawMode mode)
{
    currentMode = mode;
    if (mode == EraserMode) {
        eraserCursor->setVisible(true);
        setCursor(Qt::BlankCursor);
    } else {
        eraserCursor->setVisible(false);
        setCursor(Qt::CrossCursor);
    }
}

void whiteboardCore::setPenColor(const QColor &color)
{
    pen.setColor(color);
}

void whiteboardCore::setPenWidth(int width)
{
    pen.setWidth(width);
}

void whiteboardCore::setEraserSize(int width, int height)
{
    eraserWidth = width;
    eraserHeight = height;
    
    // 重新创建板擦图片
    eraserPixmap = QPixmap(eraserWidth, eraserHeight);
    eraserPixmap.fill(Qt::transparent);
    
    QPainter painter(&eraserPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制板擦主体
    painter.fillRect(0, 0, eraserWidth, eraserHeight, QColor(200, 200, 200, 200));
    
    // 绘制边框
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(1, 1, eraserWidth-2, eraserHeight-2);
    
    // 绘制板擦纹理
    painter.setPen(QPen(QColor(150, 150, 150, 150), 1, Qt::DashLine));
    for (int i = 0; i < eraserWidth + eraserHeight; i += 10) {
        painter.drawLine(i, 0, i - eraserHeight, eraserHeight);
    }
    
    // 添加文字
    painter.setPen(QPen(Qt::black, 1));
    painter.setFont(QFont("Arial", 8));
    painter.drawText(eraserPixmap.rect(), Qt::AlignCenter, "ERASER");
    
    painter.end();
    
    // 更新光标图片
    eraserCursor->setPixmap(eraserPixmap);
    eraserCursor->setOffset(-eraserWidth/2, -eraserHeight/2);
}

void whiteboardCore::clearBoard()
{
    // 清除所有路径项
    QList<QGraphicsItem*> allItems = scene->items();
    for (auto item : allItems) {
        if (dynamic_cast<QGraphicsPathItem*>(item) && item != eraserCursor) {
            scene->removeItem(item);
            delete item;
        }
    }
    undoStack->clear();
}

void whiteboardCore::undo()
{
    if (undoStack->canUndo())
        undoStack->undo();
}

// 计算点到直线的距离
qreal whiteboardCore::pointLineDistance(const QPointF &p, const QPointF &a, const QPointF &b)
{
    QVector2D ab(b - a);
    QVector2D ap(p - a);
    
    if (ab.length() < 1e-6) return (p - a).manhattanLength();
    
    // 投影长度
    qreal t = QVector2D::dotProduct(ap, ab) / QVector2D::dotProduct(ab, ab);
    
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    
    QPointF projection = a + (b - a) * t;
    return (p - projection).manhattanLength();
}

// 道格拉斯-普克递归实现
void whiteboardCore::douglasPeucker(const QList<QPointF> &points, int start, int end, 
                                    qreal epsilon, QList<bool> &keep)
{
    if (end <= start + 1) return;
    
    // 找到距离最远的点
    int maxIndex = start;
    qreal maxDistance = 0;
    
    for (int i = start + 1; i < end; i++) {
        qreal distance = pointLineDistance(points[i], points[start], points[end]);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxIndex = i;
        }
    }
    
    // 如果最大距离大于容差，递归处理两侧
    if (maxDistance > epsilon) {
        keep[maxIndex] = true;
        douglasPeucker(points, start, maxIndex, epsilon, keep);
        douglasPeucker(points, maxIndex, end, epsilon, keep);
    }
}

// 道格拉斯-普克抽稀
QList<QPointF> whiteboardCore::simplifyPoints(const QList<QPointF> &points)
{
    if (points.size() < 3) return points;
    
    QList<bool> keep;
    keep.resize(points.size());
    keep.fill(false);
    
    // 保留首尾点
    keep.first() = true;
    keep.last() = true;
    
    // 递归抽稀
    douglasPeucker(points, 0, points.size() - 1, simplifyTolerance, keep);
    
    // 收集保留的点
    QList<QPointF> result;
    for (int i = 0; i < points.size(); i++) {
        if (keep[i]) {
            result.append(points[i]);
        }
    }
    
    return result;
}

// 使用Catmull-Rom样条创建平滑路径
QPainterPath whiteboardCore::createSmoothPath(const QList<QPointF> &points)
{
    QPainterPath smoothPath;
    if (points.isEmpty()) return smoothPath;
    if (points.size() == 1) {
        smoothPath.moveTo(points.first());
        return smoothPath;
    }
    
    // Catmull-Rom样条插值
    // 对于点P0,P1,P2,P3，Catmull-Rom曲线在P1-P2之间的公式：
    // P(t) = 0.5 * ((-t^3 + 2t^2 - t) * P0 + (3t^3 - 5t^2 + 2) * P1 + 
    //                (-3t^3 + 4t^2 + t) * P2 + (t^3 - t^2) * P3)
    
    smoothPath.moveTo(points.first());
    
    // 分段数
    const int segments = 20;
    
    for (int i = 0; i < points.size() - 1; i++) {
        // 获取四个控制点
        QPointF p0 = (i == 0) ? points[i] : points[i - 1];
        QPointF p1 = points[i];
        QPointF p2 = points[i + 1];
        QPointF p3 = (i + 2 < points.size()) ? points[i + 2] : points[i + 1];
        
        // 生成Catmull-Rom曲线段
        for (int s = 1; s <= segments; s++) {
            qreal t = (qreal)s / segments;
            
            qreal t2 = t * t;
            qreal t3 = t2 * t;
            
            qreal h1 = 0.5 * (-t3 + 2*t2 - t);
            qreal h2 = 0.5 * (3*t3 - 5*t2 + 2);
            qreal h3 = 0.5 * (-3*t3 + 4*t2 + t);
            qreal h4 = 0.5 * (t3 - t2);
            
            QPointF point = p0 * h1 + p1 * h2 + p2 * h3 + p3 * h4;
            smoothPath.lineTo(point);
        }
    }
    
    return smoothPath;
}

QList<QGraphicsPathItem*> whiteboardCore::findItemsUnderEraser(const QPointF &center)
{
    QList<QGraphicsPathItem*> items;
    
    // 创建矩形擦除区域（使用固定的宽高）
    QRectF rect(center.x() - eraserWidth/2, 
                center.y() - eraserHeight/2,
                eraserWidth, eraserHeight);
    
    // 获取该区域内的所有项
    QList<QGraphicsItem*> all = scene->items(rect);
    for (auto item : all) {
        if (item == eraserCursor) continue;
        QGraphicsPathItem *pathItem = dynamic_cast<QGraphicsPathItem*>(item);
        if (pathItem) items.append(pathItem);
    }
    return items;
}

void whiteboardCore::eraseAt(const QPointF &pos)
{
    QList<QGraphicsPathItem*> toErase = findItemsUnderEraser(pos);
    for (auto pathItem : toErase) {
        // 创建擦除命令并执行（从场景移除）
        EraseCommand *cmd = new EraseCommand(scene, pathItem);
        undoStack->push(cmd);  // push 会自动调用 redo()
    }
}

void whiteboardCore::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        drawing = true;
        QPointF scenePos = mapToScene(event->pos());
        
        if (currentMode == PenMode) {
            currentPoints.clear();
            currentPoints.append(scenePos);
            
            // 初始创建一条简单的路径，后续会在mouseMove中更新
            painterPath = QPainterPath();
            painterPath.moveTo(scenePos);
            painterPath.lineTo(scenePos);  // 初始线段
            currentPath = scene->addPath(painterPath, pen);
        } else { // EraserMode
            eraseAt(scenePos);
        }
    }
}

void whiteboardCore::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    
    if (currentMode == EraserMode) {
        // 更新板擦位置，使其中心对准鼠标位置
        eraserCursor->setPos(scenePos);
        
        if (drawing) {
            eraseAt(scenePos);
        }
    }

    if (drawing && currentMode == PenMode) {
        currentPoints.append(scenePos);
        
        // 根据是否启用抽稀来创建平滑路径
        QPainterPath smoothPath;
        
        if (useSimplification && currentPoints.size() > 5) {
            // 抽稀后再插值
            QList<QPointF> simplified = simplifyPoints(currentPoints);
            smoothPath = createSmoothPath(simplified);
        } else {
            // 直接使用Catmull-Rom插值
            smoothPath = createSmoothPath(currentPoints);
        }
        
        // 更新显示的路径
        scene->removeItem(currentPath);
        currentPath = scene->addPath(smoothPath, pen);
    }
}

void whiteboardCore::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && drawing) {
        drawing = false;
        
        if (currentMode == PenMode && currentPath) {
            // 最终优化并创建命令
            QPainterPath finalPath;
            
            if (useSimplification && currentPoints.size() > 5) {
                QList<QPointF> simplified = simplifyPoints(currentPoints);
                finalPath = createSmoothPath(simplified);
            } else {
                finalPath = createSmoothPath(currentPoints);
            }
            
            // 更新最终的路径
            scene->removeItem(currentPath);
            currentPath = scene->addPath(finalPath, pen);
            
            // 提交到撤销栈（保存原始点集以便可能的重新编辑）
            DrawCommand *cmd = new DrawCommand(scene, currentPath, currentPoints);
            undoStack->push(cmd);
            
            // 重置
            currentPoints.clear();
            currentPath = nullptr;
            painterPath = QPainterPath();
        }
    }
}

void whiteboardCore::enterEvent(QEnterEvent *event)
{
    if (currentMode == EraserMode) {
        eraserCursor->setVisible(true);
        setCursor(Qt::BlankCursor);
    }
    QGraphicsView::enterEvent(event);
}

void whiteboardCore::leaveEvent(QEvent *event)
{
    eraserCursor->setVisible(false);
    setCursor(Qt::CrossCursor);
    QGraphicsView::leaveEvent(event);
}