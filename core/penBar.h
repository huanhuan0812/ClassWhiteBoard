#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

enum ToolType {
    Mouse,
    Pen,
    Eraser
};

class penBar: public QWidget
{
    Q_OBJECT

public:
    explicit penBar(QWidget *parent = nullptr);

    ToolType currentTool;

signals:
    void toolChanged(ToolType newTool);

    void onUndoClicked();

    void onExitClicked();


private:
    QHBoxLayout *layout;
    QPushButton *mouseButton ,*penButton, *eraserButton, *timerButton,*undoButton,* exitButton;

    void updateButtonStyles();

};
