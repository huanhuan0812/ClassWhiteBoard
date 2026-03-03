#include "timerWidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QDebug>

timerWidget::timerWidget(QWidget *parent)
    : QWidget{parent}
    , totalSeconds(0)
    , remainingSeconds(0)
    , isRunning(false)
    , isCompactMode(false)
{
    // 设置窗口属性
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // 设置初始大小
    setMaximumSize(650,350);
    
    // 设置最小大小
    setMinimumSize(650, 350);
    
    setupUI();
    
    // 设置焦点跟踪
    setFocusPolicy(Qt::StrongFocus);
    installEventFilter(this);
    
    // 初始化计时器
    countdownTimer = new QTimer(this);
    countdownTimer->setInterval(1000);
    connect(countdownTimer, &QTimer::timeout, this, &timerWidget::onTimeout);
    
    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(5000);
    inactivityTimer->setSingleShot(true);
    connect(inactivityTimer, &QTimer::timeout, this, &timerWidget::switchToCompactMode);
    
    // 初始显示
    setTotalSeconds(60);
    
    // 移动到屏幕中心
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.center() - rect().center());
    
    // 确保窗口显示在最前面
    raise();
    activateWindow();
}

timerWidget::TimeUnitWidgets timerWidget::createTimeUnit(const QString &initialDigit)
{
    TimeUnitWidgets unit;
    
    // 创建容器
    unit.container = new QWidget(this);
    unit.layout = new QVBoxLayout(unit.container);
    unit.layout->setSpacing(5);
    unit.layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建上箭头按钮
    unit.upButton = new QPushButton("▲", unit.container);
    unit.upButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 20px;"
        "   min-width: 40px;"
        "   min-height: 40px;"
        "   max-width: 40px;"
        "   max-height: 40px;"
        "}"
        "QPushButton:pressed { background-color: #45a049; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    
    // 创建数字标签
    unit.digitLabel = new QLabel(initialDigit, unit.container);
    unit.digitLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 64px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "   background-color: white;"
        "   border: 2px solid #cccccc;"
        "   border-radius: 10px;"
        "   min-width: 70px;"
        "   min-height: 90px;"
        "   max-width: 70px;"
        "   max-height: 90px;"
        "   qproperty-alignment: AlignCenter;"
        "}"
    );
    
    // 创建下箭头按钮
    unit.downButton = new QPushButton("▼", unit.container);
    unit.downButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 20px;"
        "   min-width: 40px;"
        "   min-height: 40px;"
        "   max-width: 40px;"
        "   max-height: 40px;"
        "}"
        "QPushButton:pressed { background-color: #da190b; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    
    // 添加到布局
    unit.layout->addWidget(unit.upButton, 0, Qt::AlignCenter);
    unit.layout->addWidget(unit.digitLabel, 0, Qt::AlignCenter);
    unit.layout->addWidget(unit.downButton, 0, Qt::AlignCenter);
    
    return unit;
}

void timerWidget::setupUI()
{
    // 主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 20, 15, 5);
    
    // 设置背景样式
    setStyleSheet(
        "timerWidget {"
        "   background-color: white;"
        "   border-radius: 20px;"
        "   border: 2px solid #e0e0e0;"
        "}"
    );
    
    // ==================== 时间控制区域 ====================
    timeControlWidget = new QWidget(this);
    timeControlLayout = new QHBoxLayout(timeControlWidget);
    timeControlLayout->setSpacing(5);
    timeControlLayout->setContentsMargins(5, 0, 5, 5);
    
    // 创建六个时间单元
    hourTen = createTimeUnit("0");
    hourOne = createTimeUnit("0");
    minuteTen = createTimeUnit("0");
    minuteOne = createTimeUnit("0");
    secondTen = createTimeUnit("0");
    secondOne = createTimeUnit("0");
    
    // 创建冒号标签
    QString colonStyle = 
        "QLabel {"
        "   font-size: 64px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "   background-color: transparent;"
        "   min-width: 20px;"
        "   margin-bottom: 40px;"  // 调整垂直位置与数字对齐
        "}";
    
    colon1Label = new QLabel(":", this);
    colon1Label->setStyleSheet(colonStyle);
    
    colon2Label = new QLabel(":", this);
    colon2Label->setStyleSheet(colonStyle);
    
    // 添加到水平布局
    timeControlLayout->addWidget(hourTen.container);
    timeControlLayout->addWidget(hourOne.container);
    timeControlLayout->addWidget(colon1Label);
    timeControlLayout->addWidget(minuteTen.container);
    timeControlLayout->addWidget(minuteOne.container);
    timeControlLayout->addWidget(colon2Label);
    timeControlLayout->addWidget(secondTen.container);
    timeControlLayout->addWidget(secondOne.container);
    
    mainLayout->addWidget(timeControlWidget);
    
    // ==================== 底部控制面板 ====================
    bottomPanel = new QWidget(this);
    bottomLayout = new QHBoxLayout(bottomPanel);
    bottomLayout->setSpacing(20);
    bottomLayout->setContentsMargins(20, 10, 20, 5);
    
    QString controlButtonStyle = 
        "QPushButton {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 35px;"
        "   min-width: 100px;"
        "   min-height: 70px;"
        "}"
        "QPushButton:pressed { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #cccccc; }";
    
    QString resetButtonStyle = 
        "QPushButton {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: #FF9800;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 35px;"
        "   min-width: 100px;"
        "   min-height: 70px;"
        "}"
        "QPushButton:pressed { background-color: #F57C00; }"
        "QPushButton:disabled { background-color: #cccccc; }";
    
    QString closeButtonStyle = 
        "QPushButton {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 35px;"
        "   min-width: 100px;"
        "   min-height: 70px;"
        "}"
        "QPushButton:pressed { background-color: #da190b; }";
    
    playButton = new QPushButton("▶", bottomPanel);
    playButton->setStyleSheet(controlButtonStyle);
    
    resetButton = new QPushButton("↺", bottomPanel);
    resetButton->setStyleSheet(resetButtonStyle);
    
    closeButton = new QPushButton("✕", bottomPanel);
    closeButton->setStyleSheet(closeButtonStyle);
    
    bottomLayout->addStretch();
    bottomLayout->addWidget(playButton);
    bottomLayout->addWidget(resetButton);
    bottomLayout->addWidget(closeButton);
    bottomLayout->addStretch();
    
    mainLayout->addWidget(bottomPanel);
    
    // ==================== 连接信号槽 ====================
    
    // 连接小时十位按钮
    connect(hourTen.upButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int hours = getHours();
        int newHours = hours + 10;
        setTimeFromComponents(newHours, getMinutes(), getSeconds());
    });
    
    connect(hourTen.downButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int hours = getHours();
        int newHours = hours - 10;
        if (newHours >= 0) {
            setTimeFromComponents(newHours, getMinutes(), getSeconds());
        }
    });
    
    // 连接小时个位按钮
    connect(hourOne.upButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int hours = getHours();
        int newHours = hours + 1;
        setTimeFromComponents(newHours, getMinutes(), getSeconds());
    });
    
    connect(hourOne.downButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int hours = getHours();
        int newHours = hours - 1;
        if (newHours >= 0) {
            setTimeFromComponents(newHours, getMinutes(), getSeconds());
        }
    });
    
    // 连接分钟十位按钮
    connect(minuteTen.upButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int minutes = getMinutes();
        int newMinutes = minutes + 10;
        if (newMinutes <= 60) {
            setTimeFromComponents(getHours(), newMinutes, getSeconds());
        }
    });
    
    connect(minuteTen.downButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int minutes = getMinutes();
        int newMinutes = minutes - 10;
        if (newMinutes >= 0) {
            setTimeFromComponents(getHours(), newMinutes, getSeconds());
        }
    });
    
    // 连接分钟个位按钮
    connect(minuteOne.upButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int minutes = getMinutes();
        int newMinutes = minutes + 1;
        if (newMinutes <= 60) {
            setTimeFromComponents(getHours(), newMinutes, getSeconds());
        }
    });
    
    connect(minuteOne.downButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int minutes = getMinutes();
        int newMinutes = minutes - 1;
        if (newMinutes >= 0) {
            setTimeFromComponents(getHours(), newMinutes, getSeconds());
        }
    });
    
    // 连接秒十位按钮
    connect(secondTen.upButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int seconds = getSeconds();
        int newSeconds = seconds + 10;
        if (newSeconds <= 60) {
            setTimeFromComponents(getHours(), getMinutes(), newSeconds);
        }
    });
    
    connect(secondTen.downButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int seconds = getSeconds();
        int newSeconds = seconds - 10;
        if (newSeconds >= 0) {
            setTimeFromComponents(getHours(), getMinutes(), newSeconds);
        }
    });
    
    // 连接秒个位按钮
    connect(secondOne.upButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int seconds = getSeconds();
        int newSeconds = seconds + 1;
        if (newSeconds <= 60) {
            setTimeFromComponents(getHours(), getMinutes(), newSeconds);
        }
    });
    
    connect(secondOne.downButton, &QPushButton::clicked, [this]() { 
        if (isRunning) return;
        int seconds = getSeconds();
        int newSeconds = seconds - 1;
        if (newSeconds >= 0) {
            setTimeFromComponents(getHours(), getMinutes(), newSeconds);
        }
    });
    
    // 连接底部按钮
    connect(playButton, &QPushButton::clicked, this, &timerWidget::startCountdown);
    connect(resetButton, &QPushButton::clicked, this, &timerWidget::resetCountdown);
    connect(closeButton, &QPushButton::clicked, this, &timerWidget::close);
    
    // 初始状态
    updateTimeDisplay();
    updateDigitButtons();
}

void timerWidget::setTotalSeconds(int seconds)
{
    if (seconds >= 0) {
        totalSeconds = seconds;
        if (!isRunning) {
            remainingSeconds = totalSeconds;
            updateTimeDisplay();
            updateDigitButtons();
        }
    }
}

int timerWidget::getRemainingSeconds() const
{
    return remainingSeconds;
}

bool timerWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        // 只有在运行时才启动无活动计时器
        if (isRunning) {
            inactivityTimer->start();
        }
    }
    else if (event->type() == QEvent::FocusIn) {
        inactivityTimer->stop();
        restoreFullDisplay();
    }
    
    return QWidget::eventFilter(obj, event);
}

void timerWidget::mousePressEvent(QMouseEvent *event)
{
    restoreFullDisplay();
    if (isRunning) {
        inactivityTimer->start();
    }
    QWidget::mousePressEvent(event);
}

void timerWidget::updateDisplay()
{
    updateTimeDisplay();
}

void timerWidget::startCountdown()
{
    if (remainingSeconds > 0 && !isRunning) {
        isRunning = true;
        countdownTimer->start();
        playButton->setText("⏸");
        
        // 开始倒计时后立即隐藏所有箭头按钮
        hideAllArrowButtons();
        
        updateDigitButtons();
        
        // 启动无活动计时器（用于后续可能切换到更简洁的模式）
        inactivityTimer->start();
    } else if (isRunning) {
        // 如果正在运行，点击暂停
        isRunning = false;
        countdownTimer->stop();
        playButton->setText("▶");
        
        // 暂停时显示所有箭头按钮
        showAllArrowButtons();
        
        updateDigitButtons();
        restoreFullDisplay();
        inactivityTimer->stop();
    }
}

void timerWidget::pauseCountdown()
{
    if (isRunning) {
        isRunning = false;
        countdownTimer->stop();
        playButton->setText("▶");
        
        // 暂停时显示所有箭头按钮
        showAllArrowButtons();
        
        updateDigitButtons();
        restoreFullDisplay();
        inactivityTimer->stop();
    }
}

void timerWidget::resetCountdown()
{
    isRunning = false;
    countdownTimer->stop();
    remainingSeconds = totalSeconds;
    playButton->setText("▶");
    
    // 重置时显示所有箭头按钮
    showAllArrowButtons();
    
    updateTimeDisplay();
    updateDigitButtons();
    restoreFullDisplay();
    inactivityTimer->stop();
}

void timerWidget::onTimeout()
{
    if (remainingSeconds > 0) {
        remainingSeconds--;
        updateTimeDisplay();
        
        if (remainingSeconds == 0) {
            countdownTimer->stop();
            isRunning = false;
            playButton->setText("▶");
            
            // 倒计时结束时显示所有箭头按钮
            showAllArrowButtons();
            
            updateDigitButtons();
            restoreFullDisplay();
            inactivityTimer->stop();
            
            // 倒计时结束闪烁提示
            QPropertyAnimation *animation = new QPropertyAnimation(timeControlWidget, "styleSheet");
            animation->setDuration(500);
            animation->setKeyValueAt(0, "");
            animation->setKeyValueAt(0.5, "background-color: #ffebee; border-radius: 10px;");
            animation->setKeyValueAt(1, "");
            animation->start();
        }
    }
}

void timerWidget::handleDigitAdjust(int unit, int increment)
{
    if (isRunning) return;
    
    int hours = getHours();
    int minutes = getMinutes();
    int seconds = getSeconds();
    
    if (unit == 3600) {
        hours += increment / 3600;
        if (hours < 0) hours = 0;
    }
    else if (unit == 60) {
        int newMinutes = minutes + (increment / 60);
        if (newMinutes >= 0 && newMinutes <= 60) {
            minutes = newMinutes;
        }
    }
    else if (unit == 1) {
        int newSeconds = seconds + increment;
        if (newSeconds >= 0 && newSeconds <= 60) {
            seconds = newSeconds;
        }
    }
    
    setTimeFromComponents(hours, minutes, seconds);
}

void timerWidget::restoreFullDisplay()
{
    if (isCompactMode) {
        switchToFullMode();
    }
}

void timerWidget::switchToCompactMode()
{
    if (!isCompactMode && isRunning) {  // 只有在运行时才切换到简洁模式
        isCompactMode = true;
        
        // 注意：箭头按钮已经在 startCountdown 时隐藏了，这里只隐藏底部面板
        bottomPanel->hide();
        
        // 调整数字标签样式（简洁模式）- 可以更大一些
        QString compactDigitStyle = 
            "QLabel {"
            "   font-size: 80px;"
            "   font-weight: bold;"
            "   color: #333333;"
            "   background-color: transparent;"
            "   border: none;"
            "   min-width: 70px;"
            "   min-height: 90px;"
            "   qproperty-alignment: AlignCenter;"
            "}";
        
        hourTen.digitLabel->setStyleSheet(compactDigitStyle);
        hourOne.digitLabel->setStyleSheet(compactDigitStyle);
        minuteTen.digitLabel->setStyleSheet(compactDigitStyle);
        minuteOne.digitLabel->setStyleSheet(compactDigitStyle);
        secondTen.digitLabel->setStyleSheet(compactDigitStyle);
        secondOne.digitLabel->setStyleSheet(compactDigitStyle);
        
        // 调整冒号样式
        QString compactColonStyle = 
            "QLabel {"
            "   font-size: 80px;"
            "   font-weight: bold;"
            "   color: #333333;"
            "   background-color: transparent;"
            "   min-width: 20px;"
            "}";
        
        colon1Label->setStyleSheet(compactColonStyle);
        colon2Label->setStyleSheet(compactColonStyle);
        
        // 调整窗口大小（可以更小一些）
        setFixedSize(500, 120);
    }
}

void timerWidget::switchToFullMode()
{
    if (isCompactMode) {
        isCompactMode = false;
        
        // 显示底部面板（但箭头按钮的显示状态由 isRunning 决定）
        bottomPanel->show();
        
        // 如果不在运行状态，才显示箭头按钮
        if (!isRunning) {
            showAllArrowButtons();
        }
        
        // 恢复数字标签样式（完整模式）
        QString fullDigitStyle = 
            "QLabel {"
            "   font-size: 64px;"
            "   font-weight: bold;"
            "   color: #333333;"
            "   background-color: white;"
            "   border: 2px solid #cccccc;"
            "   border-radius: 10px;"
            "   min-width: 70px;"
            "   min-height: 90px;"
            "   max-width: 70px;"
            "   max-height: 90px;"
            "   qproperty-alignment: AlignCenter;"
            "}";
        
        hourTen.digitLabel->setStyleSheet(fullDigitStyle);
        hourOne.digitLabel->setStyleSheet(fullDigitStyle);
        minuteTen.digitLabel->setStyleSheet(fullDigitStyle);
        minuteOne.digitLabel->setStyleSheet(fullDigitStyle);
        secondTen.digitLabel->setStyleSheet(fullDigitStyle);
        secondOne.digitLabel->setStyleSheet(fullDigitStyle);
        
        // 恢复冒号样式
        QString fullColonStyle = 
            "QLabel {"
            "   font-size: 64px;"
            "   font-weight: bold;"
            "   color: #333333;"
            "   background-color: transparent;"
            "   min-width: 20px;"
            "   margin-bottom: 40px;"
            "}";
        
        colon1Label->setStyleSheet(fullColonStyle);
        colon2Label->setStyleSheet(fullColonStyle);
        
        // 恢复窗口大小
        setFixedSize(650, 350);
    }
}

void timerWidget::updateDigitButtons()
{
    bool enabled = !isRunning;
    int hours = getHours();
    int minutes = getMinutes();
    int seconds = getSeconds();
    
    // 小时十位按钮
    hourTen.upButton->setEnabled(enabled);
    hourTen.downButton->setEnabled(enabled && hours >= 10);
    
    // 小时个位按钮
    hourOne.upButton->setEnabled(enabled);
    hourOne.downButton->setEnabled(enabled && (hours % 10) > 0);
    
    // 分钟十位按钮
    minuteTen.upButton->setEnabled(enabled && minutes <= 50);
    minuteTen.downButton->setEnabled(enabled && minutes >= 10);
    
    // 分钟个位按钮
    minuteOne.upButton->setEnabled(enabled && (minutes % 10) < 9 && minutes < 60);
    minuteOne.downButton->setEnabled(enabled && (minutes % 10) > 0);
    
    // 秒十位按钮
    secondTen.upButton->setEnabled(enabled && seconds <= 50);
    secondTen.downButton->setEnabled(enabled && seconds >= 10);
    
    // 秒个位按钮
    secondOne.upButton->setEnabled(enabled && (seconds % 10) < 9 && seconds < 60);
    secondOne.downButton->setEnabled(enabled && (seconds % 10) > 0);
    
    // 根据运行状态显示/隐藏箭头按钮
    if (isRunning) {
        hideAllArrowButtons();
    } else {
        showAllArrowButtons();
    }
}

void timerWidget::updateTimeDisplay()
{
    int hours = getHours();
    int minutes = getMinutes();
    int seconds = getSeconds();
    
    // 更新数字显示
    hourTen.digitLabel->setText(QString::number(hours / 10));
    hourOne.digitLabel->setText(QString::number(hours % 10));
    minuteTen.digitLabel->setText(QString::number(minutes / 10));
    minuteOne.digitLabel->setText(QString::number(minutes % 10));
    secondTen.digitLabel->setText(QString::number(seconds / 10));
    secondOne.digitLabel->setText(QString::number(seconds % 10));
}

void timerWidget::hideAllArrowButtons()
{
    hourTen.upButton->hide();
    hourTen.downButton->hide();
    hourOne.upButton->hide();
    hourOne.downButton->hide();
    minuteTen.upButton->hide();
    minuteTen.downButton->hide();
    minuteOne.upButton->hide();
    minuteOne.downButton->hide();
    secondTen.upButton->hide();
    secondTen.downButton->hide();
    secondOne.upButton->hide();
    secondOne.downButton->hide();
}

void timerWidget::showAllArrowButtons()
{
    hourTen.upButton->show();
    hourTen.downButton->show();
    hourOne.upButton->show();
    hourOne.downButton->show();
    minuteTen.upButton->show();
    minuteTen.downButton->show();
    minuteOne.upButton->show();
    minuteOne.downButton->show();
    secondTen.upButton->show();
    secondTen.downButton->show();
    secondOne.upButton->show();
    secondOne.downButton->show();
}

QString timerWidget::formatTime(int seconds) const
{
    int hrs = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    return QString("%1:%2:%3")
        .arg(hrs, 2, 10, QChar('0'))
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}

int timerWidget::getHours() const
{
    return remainingSeconds / 3600;
}

int timerWidget::getMinutes() const
{
    return (remainingSeconds % 3600) / 60;
}

int timerWidget::getSeconds() const
{
    return remainingSeconds % 60;
}

void timerWidget::setTimeFromComponents(int hours, int minutes, int seconds)
{
    // 确保分钟和秒在有效范围内
    minutes = qBound(0, minutes, 60);
    seconds = qBound(0, seconds, 60);
    
    remainingSeconds = hours * 3600 + minutes * 60 + seconds;
    if (!isRunning) {
        totalSeconds = remainingSeconds;
    }
    updateTimeDisplay();
    updateDigitButtons();
}