#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    whiteboard = new whiteboardCore(this);
    setCentralWidget(whiteboard);

    currentColor = Qt::black;
    currentWidth = 2;
    whiteboard->setSimplifyTolerance(1.0);

    bar = new penBar(this);

    currentTool = Mouse;

    connect(bar, &penBar::toolChanged, this, [this](ToolType newTool) {
        currentTool = newTool;
        switch (currentTool) {
            case Mouse:
                //whiteboard->setCursor(Qt::ArrowCursor);
                break;
            case Pen:
                //whiteboard->setCursor(Qt::PointingHandCursor);
                whiteboard->setDrawMode(whiteboardCore::PenMode);
                break;
            case Eraser:
                //whiteboard->setCursor(Qt::PointingHandCursor);
                whiteboard->setDrawMode(whiteboardCore::EraserMode);
                break;
        }
    });

    connect(bar, &penBar::onExitClicked, this, [this]() {
        close();
    });

    connect(bar, &penBar::onUndoClicked, this, [this]() {
        whiteboard->undo();
    });

}

MainWindow::~MainWindow()
{
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    bar->move(this->width()/2 - bar->width()/2, this->height() - 55);
}