#include "penBar.h"
#include <QGraphicsOpacityEffect>
#include "timerWidget.h"

penBar::penBar(QWidget *parent)
    : QWidget{parent}
{
    layout = new QHBoxLayout(this);

    mouseButton = new QPushButton(this);
    penButton = new QPushButton( this);
    eraserButton = new QPushButton(this);
    timerButton = new QPushButton( this);
    undoButton = new QPushButton( this);
    exitButton = new QPushButton(this);

    mouseButton->setIcon(QIcon(":/icons/cursor.svg"));
    penButton->setIcon(QIcon(":/icons/edit-2.svg"));
    eraserButton->setIcon(QIcon(":/icons/eraser.svg"));
    timerButton->setIcon(QIcon(":/icons/timer.svg"));
    undoButton->setIcon(QIcon(":/icons/undo.svg"));
    exitButton->setIcon(QIcon(":/icons/exit.svg"));


    mouseButton->setFixedSize(48, 48);
    penButton->setFixedSize(48, 48);
    eraserButton->setFixedSize(48, 48);
    timerButton->setFixedSize(48, 48);
    undoButton->setFixedSize(48, 48);
    exitButton->setFixedSize(48, 48);

    mouseButton->setIconSize(QSize(44, 44));
    penButton->setIconSize(QSize(44, 44));
    eraserButton->setIconSize(QSize(44, 44));
    timerButton->setIconSize(QSize(44, 44));
    undoButton->setIconSize(QSize(44, 44));
    exitButton->setIconSize(QSize(44, 44));

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.7);
    this->setGraphicsEffect(effect);

    layout->addWidget(mouseButton);
    layout->addWidget(penButton);
    layout->addWidget(eraserButton);
    layout->addWidget(timerButton);
    layout->addWidget(undoButton);
    layout->addWidget(exitButton);

    this->setLayout(layout);

    this->adjustSize();

    setStyleSheet(R"(
    QPushButton {
        background-color: #ffffff;
        border: none;
        border-radius: 5px;
    }
    QPushButton:hover {
        background-color: #ececec;
    }
    )");

    connect(mouseButton, &QPushButton::clicked, this, [this]() {
        currentTool = Mouse;
        updateButtonStyles();
        emit toolChanged(Mouse);
    });

    connect(penButton, &QPushButton::clicked, this, [this]() {

        currentTool = Pen;
        updateButtonStyles();
        emit toolChanged(Pen);
    });

    connect(eraserButton, &QPushButton::clicked, this, [this]() {
        currentTool = Eraser;
        updateButtonStyles();
        emit toolChanged(Eraser);
    });

    connect(timerButton, &QPushButton::clicked, this, [this]() {
        // 创建倒计时组件
        timerWidget *timer = new timerWidget(this);
        // 设置为模态窗口
        timer->setWindowModality(Qt::ApplicationModal);
        // 显示倒计时
        timer->show();
    });

    connect(undoButton, &QPushButton::clicked, this, [this]() {
        emit onUndoClicked();
    });

    connect(exitButton, &QPushButton::clicked, this, [this]() {
        emit onExitClicked();
    });
}

void penBar::updateButtonStyles()
{
    mouseButton->setStyleSheet(currentTool == Mouse ? "background-color: #92D050;" : "background-color: #ffffff;");
    penButton->setStyleSheet(currentTool == Pen ? "background-color: #92D050;" : "background-color: #ffffff;");
    eraserButton->setStyleSheet(currentTool == Eraser ? "background-color: #92D050;" : "background-color: #ffffff;");
}