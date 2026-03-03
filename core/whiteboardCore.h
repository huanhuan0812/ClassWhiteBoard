#pragma once

#include <QWidget>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPainterPath>
#include <QGraphicsView>
#include <QUndoStack>
#include <QList>
#include <QGraphicsItemGroup>
#include <QEnterEvent>

class whiteboardCore: public QGraphicsView
{
    Q_OBJECT

public:
    explicit whiteboardCore(QWidget *parent = nullptr);

    enum DrawMode { PenMode, EraserMode };
    
    // 设置抽稀参数
    void setSimplifyTolerance(qreal tolerance);  // 道格拉斯-普克算法的容差
    void setUseSimplification(bool enable);      // 是否启用抽稀优化

    void setDrawMode(DrawMode mode);
    void setPenColor(const QColor &color);
    void setPenWidth(int width);
    void setEraserSize(int width, int height);  // 设置橡皮擦大小
    void clearBoard();
    void undo();

private:
    QGraphicsScene *scene;
    QGraphicsPathItem *currentPath;
    QPainterPath painterPath;
    QList<QPointF> currentPoints;  // 存储当前笔画的原始点
    QPen pen;
    bool drawing;
    DrawMode currentMode;
    
    // 橡皮擦相关
    QGraphicsPixmapItem *eraserCursor;  // 板擦图片光标
    int eraserWidth;
    int eraserHeight;
    QPixmap eraserPixmap;  // 板擦图片
    
    // 曲线优化相关
    qreal simplifyTolerance;        // 抽稀容差
    bool useSimplification;          // 是否启用抽稀
    
    QUndoStack *undoStack;
    
    // 曲线优化方法
    QList<QPointF> simplifyPoints(const QList<QPointF> &points);  // 道格拉斯-普克抽稀
    QPainterPath createSmoothPath(const QList<QPointF> &points);  // Catmull-Rom插值
    qreal pointLineDistance(const QPointF &p, const QPointF &a, const QPointF &b);  // 点到直线距离
    void douglasPeucker(const QList<QPointF> &points, int start, int end, 
                        qreal epsilon, QList<bool> &keep);  // 道格拉斯-普克递归实现
    
    void eraseAt(const QPointF &pos);
    QList<QGraphicsPathItem*> findItemsUnderEraser(const QPointF &center);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        scene->setSceneRect(0, 0, this->width(), this->height());
    }
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

/*
// 使用示例：
// 创建白板
whiteboardCore *whiteboard = new whiteboardCore(this);

// 设置抽稀参数
whiteboard->setSimplifyTolerance(3.0);  // 设置抽稀容差
whiteboard->setUseSimplification(true);  // 启用抽稀优化

// 设置画笔
whiteboard->setPenColor(Qt::blue);
whiteboard->setPenWidth(3);

// 切换到橡皮擦模式
whiteboard->setDrawMode(whiteboardCore::EraserMode);

// 设置橡皮擦大小
whiteboard->setEraserSize(80, 120);

// 切换回画笔模式
whiteboard->setDrawMode(whiteboardCore::PenMode);

// 清屏
whiteboard->clearBoard();

// 撤销上一步
whiteboard->undo();
*/