#include "QGCStateMachineTest.h"
#include "Async/QGCStateMachine.h"

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

void QGCStateMachineTest::_testBasicTransitions()
{
    QGCStateMachine sm;

    sm.addState("idle");
    sm.addState("running");
    sm.addState("stopped");

    sm.addTransition("idle", "start", "running");
    sm.addTransition("running", "stop", "stopped");
    sm.addTransition("stopped", "reset", "idle");

    sm.setInitialState("idle");
    sm.start();

    QCOMPARE(sm.currentState(), QString("idle"));
    QVERIFY(sm.isRunning());

    QVERIFY(sm.trigger("start"));
    QCOMPARE(sm.currentState(), QString("running"));
    QCOMPARE(sm.previousState(), QString("idle"));

    QVERIFY(sm.trigger("stop"));
    QCOMPARE(sm.currentState(), QString("stopped"));

    QVERIFY(sm.trigger("reset"));
    QCOMPARE(sm.currentState(), QString("idle"));
}

void QGCStateMachineTest::_testGuards()
{
    QGCStateMachine sm;

    sm.addState("locked");
    sm.addState("unlocked");

    bool hasKey = false;

    sm.addTransition("locked", "unlock", "unlocked", [&hasKey]() { return hasKey; });
    sm.addTransition("unlocked", "lock", "locked");

    sm.setInitialState("locked");
    sm.start();

    // Should fail - no key
    QVERIFY(!sm.trigger("unlock"));
    QCOMPARE(sm.currentState(), QString("locked"));

    // Give key and try again
    hasKey = true;
    QVERIFY(sm.trigger("unlock"));
    QCOMPARE(sm.currentState(), QString("unlocked"));
}

void QGCStateMachineTest::_testActions()
{
    QGCStateMachine sm;
    int enterCount = 0;
    int exitCount = 0;

    sm.addState("state1", [&enterCount]() { enterCount++; }, [&exitCount]() { exitCount++; });
    sm.addState("state2");

    sm.addTransition("state1", "next", "state2");
    sm.addTransition("state2", "back", "state1");

    sm.setInitialState("state1");
    sm.start();

    QCOMPARE(enterCount, 1);  // Entered state1
    QCOMPARE(exitCount, 0);

    sm.trigger("next");
    QCOMPARE(enterCount, 1);
    QCOMPARE(exitCount, 1);  // Exited state1

    sm.trigger("back");
    QCOMPARE(enterCount, 2);  // Re-entered state1
    QCOMPARE(exitCount, 1);
}

void QGCStateMachineTest::_testHistoricalStates()
{
    QGCStateMachine sm;

    sm.addState("a");
    sm.addState("b");
    sm.addState("c");

    sm.addTransition("a", "next", "b");
    sm.addTransition("b", "next", "c");

    sm.setInitialState("a");
    sm.start();

    sm.trigger("next");
    sm.trigger("next");

    const QStringList history = sm.history();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history[0], QString("a"));
    QCOMPARE(history[1], QString("b"));
    QCOMPARE(history[2], QString("c"));
}

void QGCStateMachineTest::_testPersistence()
{
    QGCStateMachine sm1;
    sm1.addState("start");
    sm1.addState("middle");
    sm1.addState("end");

    sm1.addTransition("start", "go", "middle");
    sm1.addTransition("middle", "go", "end");

    sm1.setInitialState("start");
    sm1.start();
    sm1.trigger("go");

    // Save state
    const QVariantMap saved = sm1.saveState();

    // Create new machine and restore
    QGCStateMachine sm2;
    sm2.addState("start");
    sm2.addState("middle");
    sm2.addState("end");

    sm2.addTransition("start", "go", "middle");
    sm2.addTransition("middle", "go", "end");

    QVERIFY(sm2.restoreState(saved));
    QCOMPARE(sm2.currentState(), QString("middle"));
}
