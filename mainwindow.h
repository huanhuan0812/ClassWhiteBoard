#pragma once

#include <QMainWindow>
#include "core/whiteboardCore.h"
#include "core/penBar.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    whiteboardCore *whiteboard;
    penBar *bar;
    
    QColor currentColor;
    int currentWidth;
    ToolType currentTool;
protected:
    void showEvent(QShowEvent *event) override {
        QMainWindow::showEvent(event);
        bar->move(this->width()/2 - bar->width()/2, this->height() - 55);
    }
    void resizeEvent(QResizeEvent *event) override;
};
