#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>

class timerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit timerWidget(QWidget *parent = nullptr);
    
    // 设置倒计时时间（秒）
    void setTotalSeconds(int seconds);
    // 获取当前剩余时间（秒）
    int getRemainingSeconds() const;

protected:
    // 事件过滤器，用于检测失去焦点
    bool eventFilter(QObject *obj, QEvent *event) override;
    // 鼠标点击事件
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateDisplay();
    void startCountdown();
    void pauseCountdown();
    void resetCountdown();
    void onTimeout();
    void handleDigitAdjust(int unit, int increment);
    void restoreFullDisplay();

private:
    // UI组件
    QWidget *mainWidget;
    QVBoxLayout *mainLayout;
    
    // 时间显示区域（包含数字和按钮）
    QWidget *timeControlWidget;
    QHBoxLayout *timeControlLayout;
    
    // 每个时间单元的组合部件
    struct TimeUnitWidgets {
        QWidget *container;
        QVBoxLayout *layout;
        QPushButton *upButton;
        QLabel *digitLabel;
        QPushButton *downButton;
    };
    
    TimeUnitWidgets hourTen;    // 小时十位
    TimeUnitWidgets hourOne;    // 小时个位
    TimeUnitWidgets minuteTen;  // 分钟十位
    TimeUnitWidgets minuteOne;  // 分钟个位
    TimeUnitWidgets secondTen;  // 秒十位
    TimeUnitWidgets secondOne;  // 秒个位
    
    // 冒号标签
    QLabel *colon1Label;
    QLabel *colon2Label;
    
    // 底部控制面板
    QWidget *bottomPanel;
    QHBoxLayout *bottomLayout;
    QPushButton *playButton;      // 播放/暂停按钮
    QPushButton *resetButton;     // 重置按钮
    QPushButton *closeButton;     // 关闭按钮
    
    // 计时器
    QTimer *countdownTimer;
    QTimer *inactivityTimer;
    
    // 状态变量
    int totalSeconds;
    int remainingSeconds;
    bool isRunning;
    bool isCompactMode;
    
    // 辅助函数
    void setupUI();
    TimeUnitWidgets createTimeUnit(const QString &initialDigit);
    void updateDigitButtons();
    void updateTimeDisplay();
    void switchToCompactMode();
    void switchToFullMode();
    void hideAllArrowButtons();
    void showAllArrowButtons();
    QString formatTime(int seconds) const;
    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;
    void setTimeFromComponents(int hours, int minutes, int seconds);
    void updateTimeFromDigits();
};