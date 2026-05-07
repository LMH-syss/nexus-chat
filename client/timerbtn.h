#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

    void StartTimer();
    void StopTimer();

private:
    QTimer *_timer;
    int _counter;
    int _default_counter;
};

#endif // TIMERBTN_H
