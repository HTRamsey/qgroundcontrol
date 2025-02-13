#include "MockLinkWorker.h"
#include "MockLink.h"
#include <QDebug>

MockLinkWorker::MockLinkWorker(MockLink* link, QObject* parent)
    : QObject(parent)
    , _mockLink(link)
{
    // Connect timers to our slots:
    connect(&_timer1Hz, &QTimer::timeout, this, &MockLinkWorker::run1HzTasks);
    connect(&_timer10Hz, &QTimer::timeout, this, &MockLinkWorker::run10HzTasks);
    connect(&_timer500Hz, &QTimer::timeout, this, &MockLinkWorker::run500HzTasks);
    connect(&_timerStatusText, &QTimer::timeout, this, &MockLinkWorker::sendStatusTextMessages);
}

MockLinkWorker::~MockLinkWorker() {
    _timer1Hz.stop();
    _timer10Hz.stop();
    _timer500Hz.stop();
    _timerStatusText.stop();
}

void MockLinkWorker::startWork() {
    // Start the periodic timers:
    _timer1Hz.start(1000);  // 1Hz tasks every 1000 ms.
    _timer10Hz.start(100);  // 10Hz tasks every 100 ms.
    _timer500Hz.start(2);   // 500Hz tasks every 2 ms.

    // If the configuration requires sending status text messages, start that timer.
    if (_mockLink->shouldSendStatusText()) {
        _timerStatusText.setSingleShot(true);
        _timerStatusText.start(10000); // Delay before first sending.
    }

    // Optionally, run tasks immediately:
    run1HzTasks();
    run10HzTasks();
    run500HzTasks();
}

void MockLinkWorker::stopWork() {
    _timer1Hz.stop();
    _timer10Hz.stop();
    _timer500Hz.stop();
    _timerStatusText.stop();
}

void MockLinkWorker::run1HzTasks() {
    _mockLink->run1HzTasks();
}

void MockLinkWorker::run10HzTasks() {
    _mockLink->run10HzTasks();
}

void MockLinkWorker::run500HzTasks() {
    _mockLink->run500HzTasks();
}

void MockLinkWorker::sendStatusTextMessages() {
    _mockLink->sendStatusTextMessages();
}
