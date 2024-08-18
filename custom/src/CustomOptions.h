#pragma once

#include <QtCore/QObject>

#include "QGCOptions.h"

class CustomOptions;

class CustomFlyViewOptions : public QGCFlyViewOptions
{
public:
    explicit CustomFlyViewOptions(CustomOptions *options, QObject *parent = nullptr);

    bool showMultiVehicleList() const final { return false; }
    bool guidedBarShowEmergencyStop() const final { return false; }
};

class CustomOptions : public QGCOptions
{
public:
    explicit CustomOptions(QObject *parent = nullptr);

    // QUrl preFlightChecklistUrl() const final { return QUrl::fromUserInput("qrc:/qml/PreFlightCheckList.qml"); }
    bool showFirmwareUpgrade() const final { return false; }
    bool multiVehicleEnabled() const final { return false; }
    bool checkFirmwareVersion() const final { return false; }
    QGCFlyViewOptions *flyViewOptions() final;

private:
    CustomFlyViewOptions *_flyViewOptions = nullptr;
};
