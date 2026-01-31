#include "QGCStateMachineTest.h"

#include "MultiSignalSpyV2.h"
#include "QGCStateMachine.h"

#include <QtCore/QTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>


void QGCStateMachineTest::_testQGCStateMachineFactories()
{
    QGCStateMachine machine(QStringLiteral("FactoryTest"), nullptr);
    bool functionCalled = false;

    auto* funcState = machine.addFunctionState(QStringLiteral("Func"), [&functionCalled]() {
        functionCalled = true;
    });
    auto* delayState = machine.addDelayState(50);
    auto* finalState = machine.addFinalState(QStringLiteral("Final"));

    funcState->addTransition(funcState, &QGCState::advance, delayState);
    delayState->addTransition(delayState, &DelayState::delayComplete, finalState);

    machine.setInitialState(funcState);

    QSignalSpy finishedSpy(&machine, &QStateMachine::finished);
    machine.start();

    QVERIFY(finishedSpy.wait(500));
    QVERIFY(functionCalled);
}

void QGCStateMachineTest::_testGlobalErrorState()
{
    QGCStateMachine machine(QStringLiteral("GlobalErrorTest"), nullptr);
    bool errorHandled = false;

    auto* errorState = new FunctionState(QStringLiteral("GlobalError"), &machine, [&errorHandled]() {
        errorHandled = true;
    });
    machine.setGlobalErrorState(errorState);

    auto* asyncState = machine.addAsyncFunctionState(QStringLiteral("Async"), [](AsyncFunctionState* state) {
        QTimer::singleShot(50, state, [state]() { state->fail(); });
    });
    auto* finalState = machine.addFinalState();

    asyncState->addTransition(asyncState, &QGCState::advance, finalState);
    errorState->addTransition(errorState, &QGCState::advance, finalState);

    machine.setInitialState(asyncState);

    QSignalSpy finishedSpy(&machine, &QStateMachine::finished);
    machine.start();

    QVERIFY(finishedSpy.wait(500));
    QVERIFY(errorHandled);
}

void QGCStateMachineTest::_testLocalErrorState()
{
    QGCStateMachine machine(QStringLiteral("LocalErrorTest"), nullptr);
    bool globalErrorHandled = false;
    bool localErrorHandled = false;

    auto* globalErrorState = new FunctionState(QStringLiteral("GlobalError"), &machine, [&globalErrorHandled]() {
        globalErrorHandled = true;
    });
    machine.setGlobalErrorState(globalErrorState);

    auto* localErrorState = new FunctionState(QStringLiteral("LocalError"), &machine, [&localErrorHandled]() {
        localErrorHandled = true;
    });

    auto* asyncState = new AsyncFunctionState(QStringLiteral("Async"), &machine, [](AsyncFunctionState* state) {
        QTimer::singleShot(50, state, [state]() { state->fail(); });
    });
    asyncState->setLocalErrorState(localErrorState);  // Override global error for this state

    auto* finalState = machine.addFinalState();

    asyncState->addTransition(asyncState, &QGCState::advance, finalState);
    globalErrorState->addTransition(globalErrorState, &QGCState::advance, finalState);
    localErrorState->addTransition(localErrorState, &QGCState::advance, finalState);

    machine.setInitialState(asyncState);

    QSignalSpy finishedSpy(&machine, &QStateMachine::finished);
    machine.start();

    QVERIFY(finishedSpy.wait(500));
    QVERIFY(!globalErrorHandled);  // Global error should NOT be called
    QVERIFY(localErrorHandled);    // Local error should be called
}

void QGCStateMachineTest::_testPropertyAssignment()
{
    QGCStateMachine machine(QStringLiteral("PropTest"), nullptr);
    machine.enablePropertyRestore();

    QObject target;
    target.setProperty("testProp", 0);

    // Use a state that doesn't transition immediately so we can verify property
    auto* state1 = new QGCState(QStringLiteral("State1"), &machine);
    state1->setProperty(&target, "testProp", 42);

    auto* state2 = new QGCState(QStringLiteral("State2"), &machine);
    auto* finalState = new QFinalState(&machine);

    // Use event-based transitions instead of entered signal
    machine.addEventTransition(state1, QStringLiteral("next"), state2);
    state2->addTransition(state2, &QState::entered, finalState);

    machine.setInitialState(state1);

    QSignalSpy state1EnteredSpy(state1, &QState::entered);
    QSignalSpy finishedSpy(&machine, &QStateMachine::finished);

    machine.start();

    // Wait for state1 to be entered
    QVERIFY(state1EnteredSpy.wait(500));
    // Property should be set to 42 while in state1
    QCOMPARE(target.property("testProp").toInt(), 42);

    // Trigger transition to state2
    machine.postEvent(QStringLiteral("next"));

    QVERIFY(finishedSpy.wait(500));
    // Property should be restored after leaving state1
    QCOMPARE(target.property("testProp").toInt(), 0);
}
