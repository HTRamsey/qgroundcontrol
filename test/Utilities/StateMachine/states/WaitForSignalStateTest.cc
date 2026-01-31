#include "WaitForSignalStateTest.h"
#include "StateTestCommon.h"


void WaitForSignalStateTest::_testWaitForSignalState()
{
    QStateMachine machine;
    QObject signalSource;

    auto* waitState = new WaitForSignalState(
        QStringLiteral("TestWait"),
        &machine,
        &signalSource,
        &QObject::objectNameChanged,
        0
    );

    auto* finalState = new QFinalState(&machine);

    waitState->addTransition(waitState, &QGCState::advance, finalState);
    machine.setInitialState(waitState);

    QSignalSpy finishedSpy(&machine, &QStateMachine::finished);
    QSignalSpy enteredSpy(waitState, &QState::entered);

    machine.start();

    QVERIFY(enteredSpy.wait(500));

    QTimer::singleShot(50, &signalSource, [&signalSource]() {
        signalSource.setObjectName(QStringLiteral("triggered"));
    });

    QVERIFY(finishedSpy.wait(500));
}

void WaitForSignalStateTest::_testWaitForSignalStateTimeout()
{
    QStateMachine machine;
    QObject signalSource;
    bool timeoutReached = false;
    const int timeoutMs = 100;

    auto* waitState = new WaitForSignalState(
        QStringLiteral("TestWaitTimeout"),
        &machine,
        &signalSource,
        &QObject::objectNameChanged,
        timeoutMs
    );
    auto* timeoutState = new FunctionState(QStringLiteral("TimeoutHandler"), &machine, [&timeoutReached]() {
        timeoutReached = true;
    });
    auto* finalState = new QFinalState(&machine);

    waitState->addTransition(waitState, &QGCState::advance, finalState);
    waitState->addTransition(waitState, &WaitForSignalState::timeout, timeoutState);
    timeoutState->addTransition(timeoutState, &QGCState::advance, finalState);
    machine.setInitialState(waitState);

    // Use MultiSignalSpyV2 to verify timeout() fired but advance() did not
    MultiSignalSpyV2 stateSpy;
    QVERIFY(stateSpy.init(waitState));

    QSignalSpy finishedSpy(&machine, &QStateMachine::finished);
    machine.start();

    QVERIFY(finishedSpy.wait(500));
    QVERIFY(timeoutReached);
    // Verify timeout path taken, not success path
    QVERIFY(stateSpy.checkSignalByMask(stateSpy.signalNameToMask("timeout")));
    QVERIFY(stateSpy.checkNoSignalByMask(stateSpy.signalNameToMask("advance")));
}
