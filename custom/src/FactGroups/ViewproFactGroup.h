#pragma once

#include <QtCore/QLoggingCategory>

#include "FactGroup.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(ViewproFactGroupLog)

class ViewproFactGroup : public FactGroup
{
    Q_OBJECT

public:
    ViewproFactGroup(QObject *parent = nullptr);
    ~ViewproFactGroup();

    void handleMessage(Vehicle *vehicle, mavlink_message_t &message) final;

private:
    void _handleMsg(mavlink_message_t &message);
};
