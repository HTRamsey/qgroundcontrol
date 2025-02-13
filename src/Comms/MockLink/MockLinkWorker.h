#ifndef MOCKLINKWORKER_H
#define MOCKLINKWORKER_H

#include <QObject>
#include <QTimer>

class MockLink; // Forward declaration

/**
 * @brief The MockLinkWorker class
 *
 * This QObject contains the timer-based tasks that were originally in MockLink::run().
 * It is moved to a separate QThread.
 */
class MockLinkWorker : public QObject {
    Q_OBJECT
public:
    explicit MockLinkWorker(MockLink* link, QObject* parent = nullptr);
    ~MockLinkWorker();

public slots:
    /// Called when the thread starts; starts the timers.
    void startWork();

    /// Optionally stop the timers (for clean shutdown).
    void stopWork();

private slots:
    /// Called at 1Hz intervals.
    void run1HzTasks();

    /// Called at 10Hz intervals.
    void run10HzTasks();

    /// Called at 500Hz intervals.
    void run500HzTasks();

    /// Called to send status text messages (delayed start).
    void sendStatusTextMessages();

private:
    MockLink* _mockLink;  ///< Back-reference to the owning MockLink
    QTimer    _timer1Hz;
    QTimer    _timer10Hz;
    QTimer    _timer500Hz;
    QTimer    _timerStatusText;
};

#endif // MOCKLINKWORKER_H
