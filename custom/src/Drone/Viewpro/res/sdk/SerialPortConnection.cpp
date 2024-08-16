#include "SerialPortConnection.h"
#include "Utility/VLKLog.h"

CSerialPortConnection::CSerialPortConnection(const std::string& strPortName, int iBaudRate)
    : CDeviceConnection(CT_SerialPort)
    , m_SerialPort(strPortName, iBaudRate)
{
}

CSerialPortConnection::~CSerialPortConnection()
{
	Disconnect();
}

void CSerialPortConnection::SetConnectInfo(const std::string& strPortName, int iBaudRate)
{
    m_SerialPort.setPort(strPortName);
    m_SerialPort.setBaudrate(iBaudRate);
}

void CSerialPortConnection::GetConnectInfo(std::string& strPortName, int& iBaudRate)
{
    strPortName = m_SerialPort.getPort();
    iBaudRate = m_SerialPort.getBaudrate();
}


bool CSerialPortConnection::Connect()
{
	Disconnect();

    m_SerialPort.open();

    // ���������߳�
    StartKeepAliveThread();
	return true;
}

bool CSerialPortConnection::IsConnected()
{
	return /*m_SerialPort.isOpen()*/m_bIsConnected;
}

void CSerialPortConnection::Disconnect()
{
    StopKeepAliveThread();

	if (m_SerialPort.isOpen())
	{
		m_SerialPort.close();
	}
	
	m_bIsConnected = false;
}


void CSerialPortConnection::Move(short uHorizontalSpeed, short uVeritcalSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetMoveData(uHorizontalSpeed, uVeritcalSpeed);
#if 0
		PrintHexData(data, "send move");
#endif
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::Stop()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetStopData();
#if 0
        PrintHexData(data, "send stop");
#endif
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::ZoomIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetZoomInData(uSpeed);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::ZoomOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetZoomOutData(uSpeed);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::StopZoom()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetStopZoomData();

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::EnableTrackMode(const VLK_TRACK_MODE_PARAM& param)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetEnableTrackModeData(param);
#if 0
		PrintHexData(data, __FUNCTION__);
#endif
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::TrackTargetPosition(int iX, int iY)
{
                    DEBUG_LOG("size_cmd --- -TrackTargetPosition--20210813-- !!!");
	if (m_bIsValidDevice)
	{
		QByteArray data = GetTrackTargetPositionData(iX, iY);
#if 0
		PrintHexData(data, __FUNCTION__);
#endif
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::TrackTargetPositionEx(const VLK_TRACK_MODE_PARAM& param, int iX, int iY)
{
	EnableTrackMode(param);
	TrackTargetPosition(iX, iY);
}

void CSerialPortConnection::DisableTrackMode()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetDisableTrackModeData();

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::FocusIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		// if (m_iFocusMode != 0)
		{
			QByteArray data = GetFocusInData(uSpeed);

            m_SerialPort.write((const uint8_t*)&data[0], data.size());
		}
	}
}

void CSerialPortConnection::FocusOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		// if (m_iFocusMode != 0)
		{
			QByteArray data = GetFocusOutData(uSpeed);

            m_SerialPort.write((const uint8_t*)&data[0], data.size());
		}
	}
}

void CSerialPortConnection::StopFocus()
{
	if (m_bIsValidDevice)
	{
		// if (m_iFocusMode != 0)
		{
			QByteArray data = GetStopFocusData();

            m_SerialPort.write((const uint8_t*)&data[0], data.size());
		}
	}
}



void CSerialPortConnection::StartCarRecognition()
{
	if (m_bIsValidDevice)
	{
		// if (m_iFocusMode != 0)
		{
			QByteArray data = GetStartCarRecognition();

            m_SerialPort.write((const uint8_t*)&data[0], data.size());
		}
	}
}


void CSerialPortConnection::StartCarTrack()
{
	if (m_bIsValidDevice)
	{
		// if (m_iFocusMode != 0)
		{
			QByteArray data = GetStartCarTrack();

            m_SerialPort.write((const uint8_t*)&data[0], data.size());
		}
	}
}


void CSerialPortConnection::StopCarRecognition()
{
	if (m_bIsValidDevice)
	{
		// if (m_iFocusMode != 0)
		{
			QByteArray data = GetStopCarRecognition();

            m_SerialPort.write((const uint8_t*)&data[0], data.size());
		}
	}
}


void CSerialPortConnection::Home()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetHomeData();

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}



void CSerialPortConnection::SwitchMotor(bool bOn)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSwitchMotorData(bOn);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());

		// m_bIsMotorOn = bOn;
	}
}

void CSerialPortConnection::EnableFollowMode(bool bEnable)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetEnableFollowModeData(bEnable);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());

		// m_bIsFollowMode = bEnable;
	}
}

void CSerialPortConnection::TurnTo(double dYaw, double dPitch)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetTurnToData(dYaw, dPitch);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetGimbalCalibration(double dLat, double dLng)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetGimbalCalibration(dLat, dLng);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetGimbalCalibrationT1(double dLat, double dLng,double dLat1, double dLng1)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetGimbalCalibrationT1(dLat, dLng,dLat1, dLng1);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::StartTrack()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetStartTrackData();

        m_SerialPort.write((const uint8_t*)&data[0], data.size());

		// m_bIsTracking = true;
	}
}

void CSerialPortConnection::StopTrack()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetStopTrackData();

        m_SerialPort.write((const uint8_t*)&data[0], data.size());

		// m_bIsTracking = false;
	}
}

// 0: auto; 1: 32; 2: 64: 3: 128;
void CSerialPortConnection::SetTrackTemplateSize(int iSize)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetTrackTemplateSizeData(iSize);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetImageColor(VLK_IMAGE_TYPE emImgType, bool bEnablePIP, VLK_IR_COLOR emIRColor)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetImageColorData(emImgType, bEnablePIP, emIRColor);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetImageColorV2(uchar cImgType)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetImageColorDataV2(cImgType);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetImageColorV3(uchar cIRColor)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetImageColorDataV3(cIRColor);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::Photograph()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetPhotographData();

        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SwitchRecord(bool bOn)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSwitchRecordData(bOn);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());

		// m_bIsRecording = bOn;
	}
}

void CSerialPortConnection::SwitchDefog(bool bOn)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSwitchDefogData(bOn);

        m_SerialPort.write((const uint8_t*)&data[0], data.size());

		// m_bIsDefogOn = bOn;
	}
}

void CSerialPortConnection::SetRecordMode(int iMode)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetRecordModeData(iMode);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetFocusMode(int iMode)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetFocusModeData(iMode);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
		m_iFocusMode = iMode;
	}
}

// �������ָ��
void CSerialPortConnection::SwitchLaser(bool bOn)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSwitchLaserData(bOn);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::LaserZoomIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetLaserZoomInData(uSpeed);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::LaserZoomOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetLaserZoomOutData(uSpeed);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::LaserStopZoom()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetLaserStopZoomData();
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// iMode: 0 automatically follow EO; 1 manually
void CSerialPortConnection::SetLaserZoomMode(int iMode)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetLaserZoomModeData(iMode);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// �ɼ���ŵ���ָ��������
void CSerialPortConnection::ZoomTo(float fMag)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetZoomToData(fMag);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// �����ǵ��ӷŴ���С
void CSerialPortConnection::IRDigitalZoomIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetIRDigitalZoomInData(uSpeed);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::IRDigitalZoomOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetIRDigitalZoomOutData(uSpeed);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// �򿪹ر�EO digital zoom
void CSerialPortConnection::SwitchEODigitalZoom(bool bOn)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSwitchEODigitalZoomData(bOn);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// ����SBUSģʽ
void CSerialPortConnection::EnterSBUSMode()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetEnterSBUSModeData();
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// �˳�SBUSģʽ
void CSerialPortConnection::ExitSBUSMode()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetExitSBUSModeData();
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// ��ѯ�豸��ǰ����
void CSerialPortConnection::QueryDevConfiguration()
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetQueryDevConfigurationData();
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetTimeZone(char tz)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetTimeZoneData(tz);
		m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// �����豸OSD
void CSerialPortConnection::SetOSD(uchar uParam1, uchar uParam2)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetOSDData(uParam1, uParam2);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

// ��������ң����ͨ������
void CSerialPortConnection::SetWirelessCtrlChnlMap(const VLK_CHANNELS_MAP& ChnlsMap)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetWirelessCtrlChnlMapData(ChnlsMap);
        m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SetWirelessCtrlChnlMapV2(const VLK_CHANNELS_MAP& ChnlsMap, const VLK_CHANNELS_INVERT_FLAG& ChnlInvertFlag)
{
	if (m_bIsValidDevice)
	{
		QByteArray data = GetSetWirelessCtrlChnlMapData(ChnlsMap);
		m_SerialPort.write((const uint8_t*)&data[0], data.size());

		data = GetSetWirelessCtrlChnlInvertFlagDataPart1(ChnlInvertFlag);
		m_SerialPort.write((const uint8_t*)&data[0], data.size());

		data = GetSetWirelessCtrlChnlInvertFlagDataPart2(ChnlInvertFlag);
		m_SerialPort.write((const uint8_t*)&data[0], data.size());
	}
}

void CSerialPortConnection::SendExtentCmd(const QByteArray& cmd)
{
	// if (m_bIsValidDevice)
	{
        DEBUG_LOG("size_cmd --- ---20210813-- !!!");
        m_SerialPort.write((const uint8_t*)&cmd[0], cmd.size());
	}
}

void CSerialPortConnection::SetDevConfiguration(const VLK_DEV_CONFIG& DeviceCfg)
{
	QByteArray data = GetSetDevConfigurationData(DeviceCfg);
    m_SerialPort.write((const uint8_t*)&data[0], data.size());
}

void CSerialPortConnection::SendKeepAlive()
{
    if (m_bIsConnected && !m_SerialPort.isOpen())
    {
        m_bIsConnected = false;
        if (m_pConnStatusCB)
        {
            m_pConnStatusCB(VLK_CONN_STATUS_SERIAL_PORT_DISCONNECTED, NULL, 0, m_pConnStatusUserParam);
        }
        DEBUG_LOG("serial port disconnected !!!");
        return;
    }
    else if (!m_bIsConnected && m_SerialPort.isOpen())
    {
        m_bIsConnected = true;
        if (m_pConnStatusCB)
        {
            m_pConnStatusCB(VLK_CONN_STATUS_SERIAL_PORT_CONNECTED, NULL, 0, m_pConnStatusUserParam);
        }

        DEBUG_LOG("serial port connected !!!");
    }

    QByteArray data = GetKeepAliveData();
#if 0
        PrintHexData(data, "send keep alive");
#endif
    m_SerialPort.write((const uint8_t*)&data[0], data.size());
}

void CSerialPortConnection::DoReadBuffer()
{
    size_t ReableSize = m_SerialPort.available();
    if (ReableSize > 0)
    {
        // DEBUG_LOG("m_SerialPort.available() = %d", ReableSize);
        QByteArray frame(ReableSize, 0x00);
        size_t readLen = m_SerialPort.read((uint8_t*)&frame[0], frame.size());
        if (readLen > 0)
        {
            m_lockBuffer.lock();
            m_Buffer.insert(m_Buffer.end(), frame.begin(), frame.begin() + readLen);
            m_lockBuffer.unlock();
        }
    }

    CDeviceConnection::DoReadBuffer();
}
