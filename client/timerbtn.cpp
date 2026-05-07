#include "timerbtn.h"
#include <QString>

TimerBtn::TimerBtn(QWidget *parent)
    : QPushButton(parent),
    _timer(new QTimer(this)),
    _counter(10),
    _default_counter(10)
{
    connect(_timer, &QTimer::timeout, this, [this]() {
        _counter--;

        if (_counter <= 0) {
            StopTimer();
            return;
        }

        this->setText(QString::number(_counter));
    });
}

TimerBtn::~TimerBtn()
{
    StopTimer();
}

void TimerBtn::StartTimer()
{
    // 如果已经在倒计时，就不重复启动
    if (_timer->isActive()) {
        return;
    }

    this->setEnabled(false);
    this->setText(QString::number(_counter));
    _timer->start(1000);
}

void TimerBtn::StopTimer()
{
    if (_timer->isActive()) {
        _timer->stop();
    }

    _counter = _default_counter;
    this->setText(tr("获取"));
    this->setEnabled(true);
}