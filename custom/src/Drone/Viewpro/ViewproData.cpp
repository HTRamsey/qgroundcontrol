#include "ViewproData.h"
#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>

static Q_LOGGING_CATEGORY(Log, "zat.comms.viewprodata")

ViewproData::ViewproData(QObject* parent): MavCameraData(QHostAddress(GIMBAL_IP), GIMBAL_PORT, sysID(), gimbalCompID(), parent),
    m_stream(QString("rtsp://%1:554/stream0").arg(getIPAddress().toString()))
{
    // TODO: SERIAL1_PROTOCOL param?

    connect(this, &ViewproData::ipChanged, this, [this]()
    {
        m_stream = QUrl(m_streamUrl.arg(getIPAddress().toString()));
        emit streamChanged();
    });

    (void) connect(MavLib::mavlib(), &MavLib::msgReceived, this, &ViewproData::msgReceived, Qt::AutoConnection);

    qCDebug(Log) << Q_FUNC_INFO << this;
}

ViewproData::~ViewproData()
{
    qCDebug(Log) << Q_FUNC_INFO << this;
}

void ViewproData::_handleParamExtValue(char param_id[16], uint16_t param_index, const QVariant value)
{
    Q_UNUSED(param_id);
    bool ok;
    switch(static_cast<ViewproParams>(param_index))
    {
        case GIM_MODE:
        {
            const uint8_t result = value.toUInt(&ok);
            if(ok)
            {
                if(result != m_gimMode)
                {
                    m_gimMode = result;
                    emit gimModeChanged(m_gimMode);
                }
            }
            break;
        }

        case GIM_SPEED:
        {
            const uint8_t result = value.toUInt(&ok);
            if(ok)
            {
                if(result != m_gimSpeed)
                {
                    m_gimSpeed = result;
                    emit gimSpeedChanged(m_gimSpeed);
                }
            }
            break;
        }

        case PITCH_ANGLE:
        {
            const float result = value.toFloat(&ok);
            if(ok)
            {
                if(result != m_pitchAngle)
                {
                    m_pitchAngle = result;
                    emit pitchAngleChanged(m_pitchAngle);
                }
            }
            break;
        }

        case YAW_ANGLE:
        {
            const float result = value.toFloat(&ok);
            if(ok)
            {
                if(result != m_yawAngle)
                {
                    m_yawAngle = result;
                    emit yawAngleChanged(m_yawAngle);
                }
            }
            break;
        }

        case CAM_MODE:
        {
            const uint8_t result = value.toUInt(&ok);
            if(ok)
            {
                if(result != m_camMode2)
                {
                    m_camMode2 = result;
                    emit camModeChanged(m_camMode2);
                }
            }
            break;
        }

        case TRACK_SW:
        {
            const bool result = value.toBool();
            if(result != m_trackSW)
            {
                m_trackSW = result;
                emit trackSWChanged(m_trackSW);
            }
            break;
        }

        case OSD_SET:
        {
            const uint8_t result = value.toUInt(&ok);
            if(ok)
            {
                if(result != m_osdSet)
                {
                    m_osdSet = result;
                    emit osdSetChanged(m_osdSet);
                }
            }
            break;
        }

        case IR_DZOOM:
        {
            const uint8_t result = value.toUInt(&ok);
            if(ok)
            {
                if(result != m_irDZoom)
                {
                    m_irDZoom = result;
                    emit irDZoomChanged(m_irDZoom);
                }
            }
            break;
        }

        case PIP_MODE:
        {
            const uint8_t result = value.toUInt(&ok);
            if(ok)
            {
                if(result != m_pipMode)
                {
                    m_pipMode = result;
                    emit pipModeChanged(m_pipMode);
                }
            }
            break;
        }

        default:
            break;
    }
}
