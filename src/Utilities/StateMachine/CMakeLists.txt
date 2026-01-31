# ============================================================================
# State Machine Subsystem
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        # Core
        QGCStateMachine.cc
        QGCStateMachine.h
        QGCStateMachineEvent.cc
        QGCStateMachineEvent.h

        # States
        states/QGCAbstractState.cc
        states/QGCAbstractState.h
        states/QGCFinalState.cc
        states/QGCFinalState.h
        states/QGCHistoryState.cc
        states/QGCHistoryState.h
        states/ParallelState.cc
        states/ParallelState.h
        states/QGCState.cc
        states/QGCState.h
        states/AsyncFunctionState.cc
        states/AsyncFunctionState.h
        states/ConditionalState.cc
        states/ConditionalState.h
        states/DelayState.cc
        states/DelayState.h
        states/FunctionState.cc
        states/FunctionState.h
        states/RequestMessageState.cc
        states/RequestMessageState.h
        states/SendMavlinkCommandState.cc
        states/SendMavlinkCommandState.h
        states/SendMavlinkMessageState.cc
        states/SendMavlinkMessageState.h
        states/ShowAppMessageState.cc
        states/ShowAppMessageState.h
        states/SkippableAsyncState.cc
        states/SkippableAsyncState.h
        states/SubMachineState.cc
        states/SubMachineState.h
        states/WaitForMavlinkMessageState.cc
        states/WaitForMavlinkMessageState.h
        states/WaitForSignalState.cc
        states/WaitForSignalState.h
        states/WaitStateBase.cc
        states/WaitStateBase.h

        # Transitions
        transitions/QGCAbstractTransition.cc
        transitions/QGCAbstractTransition.h
        transitions/QGCSignalTransition.cc
        transitions/QGCSignalTransition.h
        transitions/GuardedTransition.cc
        transitions/GuardedTransition.h
        transitions/InternalTransition.cc
        transitions/InternalTransition.h
        transitions/MachineEventTransition.cc
        transitions/MachineEventTransition.h
        transitions/NamedEventTransition.cc
        transitions/NamedEventTransition.h
        transitions/QGCEventTransition.cc
        transitions/QGCEventTransition.h
        transitions/SignalDataTransition.cc
        transitions/SignalDataTransition.h
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/states
        ${CMAKE_CURRENT_SOURCE_DIR}/transitions
)
