# ============================================================================
# State Machine Unit Tests
# Tests for state machine states and transitions
# ============================================================================

# ----------------------------------------------------------------------------
# Root-level tests (QGCStateMachine helpers)
# ----------------------------------------------------------------------------
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCStateMachineTest.cc
        QGCStateMachineTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ----------------------------------------------------------------------------
# State Tests
# ----------------------------------------------------------------------------
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        states/StateTestCommon.h
        states/FunctionStateTest.cc
        states/FunctionStateTest.h
        states/DelayStateTest.cc
        states/DelayStateTest.h
        states/AsyncFunctionStateTest.cc
        states/AsyncFunctionStateTest.h
        states/WaitForSignalStateTest.cc
        states/WaitForSignalStateTest.h
        states/ConditionalStateTest.cc
        states/ConditionalStateTest.h
        states/SkippableAsyncStateTest.cc
        states/SkippableAsyncStateTest.h
        states/ParallelStateTest.cc
        states/ParallelStateTest.h
        states/SubMachineStateTest.cc
        states/SubMachineStateTest.h
        states/QGCStateTest.cc
        states/QGCStateTest.h
        states/QGCFinalStateTest.cc
        states/QGCFinalStateTest.h
        states/QGCHistoryStateTest.cc
        states/QGCHistoryStateTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/states)

# ----------------------------------------------------------------------------
# Transition Tests
# ----------------------------------------------------------------------------
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        transitions/TransitionTestCommon.cc
        transitions/TransitionTestCommon.h
        transitions/GuardedTransitionTest.cc
        transitions/GuardedTransitionTest.h
        transitions/InternalTransitionTest.cc
        transitions/InternalTransitionTest.h
        transitions/MachineEventTransitionTest.cc
        transitions/MachineEventTransitionTest.h
        transitions/NamedEventTransitionTest.cc
        transitions/NamedEventTransitionTest.h
        transitions/SignalDataTransitionTest.cc
        transitions/SignalDataTransitionTest.h
        transitions/QGCSignalTransitionTest.cc
        transitions/QGCSignalTransitionTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/transitions)
