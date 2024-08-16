#include "UDPConnection.h"
#include <QNetworkDatagram>
#include <QTimerEvent>
#include "DeviceConnectionMgrService.h"

#define MAX_REPEAT_STOP 2
#define REPEAT_STOP_TIMER_ELAPSE 50

CUDPConnection::CUDPConnection(IApplicationContext* pContext)
	: CDeviceConnection(pContext, CT_UDP)
	, m_iRemotePort(0)
	, m_iLocalPort(0)
	, m_iReadBufferTimer(0)
	, m_iRepeatStopTimer(0)
{
	connect(&m_UdpSocket, SIGNAL(hostFound()), this, SLOT(slot_hostfound()));
	connect(&m_UdpSocket, SIGNAL(connected()), this, SLOT(slot_connected()));
	connect(&m_UdpSocket, SIGNAL(readyRead()), this, SLOT(slot_read_data()));
	connect(&m_UdpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_udp_error(QAbstractSocket::SocketError)));
}

CUDPConnection::~CUDPConnection()
{
	disconnect(&m_UdpSocket, SIGNAL(hostFound()), this, SLOT(slot_hostfound()));
	disconnect(&m_UdpSocket, SIGNAL(connected()), this, SLOT(slot_connected()));
	disconnect(&m_UdpSocket, SIGNAL(readyRead()), this, SLOT(slot_read_data()));
	disconnect(&m_UdpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_udp_error(QAbstractSocket::SocketError)));
}

void CUDPConnection::slot_hostfound()
{
}

void CUDPConnection::slot_connected()
{
	qDebug() << __FUNCTION__;
	m_bIsConnected = true;
	CUDPConnStatusChangedEventArg* pArg = new CUDPConnStatusChangedEventArg();
	pArg->strRemoteIP = m_strRemoteIPAddr;
	pArg->iRemotePort = m_iRemotePort;
	pArg->strLocalIP = m_strLocalIPAddr;
	pArg->iLocalPort = m_iLocalPort;
	pArg->bIsOnline = m_bIsConnected;
	pArg->strErrorMsg = m_UdpSocket.errorString();
	m_pContext->FireEvent(EVT_UDP_CONN_STATUS_CHANGED, pArg, false, false);
	pArg->Release();

	m_KeepAliveTimer = startTimer(KEEP_ALIVE_INTERVAL);
	m_iReadBufferTimer = startTimer(READ_BUFFER_TIMER_ELAPSE);
}

void CUDPConnection::slot_udp_error(QAbstractSocket::SocketError error)
{
	qCritical() << __FUNCTION__ << error;
	m_bIsValidDevice = false;
	m_bIsConnected = false;

	CUDPConnStatusChangedEventArg* pArg = new CUDPConnStatusChangedEventArg();
	pArg->strRemoteIP = m_strRemoteIPAddr;
	pArg->iRemotePort = m_iRemotePort;
	pArg->strLocalIP = m_strLocalIPAddr;
	pArg->iLocalPort = m_iLocalPort;
	pArg->bIsOnline = m_bIsConnected;
	pArg->strErrorMsg = m_UdpSocket.errorString();
	m_pContext->FireEvent(EVT_UDP_CONN_STATUS_CHANGED, pArg, false, false);
	pArg->Release();
}

void CUDPConnection::slot_read_data()
{
	while (m_UdpSocket.hasPendingDatagrams())
	{
		QNetworkDatagram datagram = m_UdpSocket.receiveDatagram();
		auto data = datagram.data();
		if (!data.isEmpty())
		{
			m_lockBuffer.lock();
#if 0
			PrintHexData(data, "########## receive");
#endif
			m_Buffer.append(data);
			m_lockBuffer.unlock();
		}
	}
}

void CUDPConnection::SetConnectInfo(const QString& strRemoteIPAddr, int iRemotePort, const QString& strLocalIPAddr, int iLocalPort)
{
	m_strRemoteIPAddr = strRemoteIPAddr;
	m_iRemotePort = iRemotePort;
	m_strLocalIPAddr = strLocalIPAddr;
	m_iLocalPort = iLocalPort;
}

void CUDPConnection::GetConnectInfo(QString& strRemoteIPAddr, int& iRemotePort, QString& strLocalIPAddr, int& iLocalPort) const
{
	strRemoteIPAddr = m_strRemoteIPAddr;
	iRemotePort = m_iRemotePort;
	strLocalIPAddr = m_strLocalIPAddr;
	iLocalPort = m_iLocalPort;
}

bool CUDPConnection::Connect()
{
	if (m_UdpSocket.bind(QHostAddress(m_strLocalIPAddr), m_iLocalPort))
	{
		m_UdpSocket.connectToHost(m_strRemoteIPAddr, m_iRemotePort);
		return true;
	}
	else
	{
		return false;
	}
}

void CUDPConnection::Disconnect()
{
	m_UdpSocket.disconnectFromHost();
	m_UdpSocket.close();
	m_bIsConnected = false;
}

bool CUDPConnection::IsConnected() const
{
	return m_bIsConnected;
}


void CUDPConnection::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_KeepAliveTimer)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			// 发送心跳包
			QByteArray frame = Package(GetKeepAliveData());
			qint64 r = m_UdpSocket.write(frame, frame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
	else if (event->timerId() == m_iReadBufferTimer)
	{
		DoReadBuffer();
	}
	else if (event->timerId() == m_iRepeatStopTimer)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			//因为对方丢包，重复停止指令以规避
			qint64 r = m_UdpSocket.write(m_StopCommand.data(), m_StopCommand.size());
			if (m_count >= MAX_REPEAT_STOP)
			{
				m_count = 0;
				killTimer(m_iRepeatStopTimer);
			}
			else
			{
				m_count++;
			}
		}
	}
}

QByteArray CUDPConnection::Package(const QByteArray& cmd)
{
	QByteArray TCPFrame;
	TCPFrame.resize(4 + cmd.size());
	TCPFrame.fill(0x00);
	TCPFrame[0] = 0xEB; // head
	TCPFrame[1] = 0x90; // function code
	TCPFrame[2] = (uchar)cmd.size(); // data size

	int iCheckSum = 0;
	for (int i = 0; i < cmd.size(); ++i)
	{
		TCPFrame[3 + i] = cmd.at(i);
		iCheckSum += cmd.at(i);
	}
	TCPFrame[4 + cmd.size() - 1] = (uchar)(0xFF & iCheckSum);

	// qDebug() << "TCPFrame size: " << TCPFrame.size();
	// qDebug() << "TCPFrame data: " << TCPFrame;

#if 0
	PrintHexData(cmd, "send");
#endif
	return TCPFrame;
}

void CUDPConnection::Move(short uHorizontalSpeed, short uVeritcalSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetMoveData(uHorizontalSpeed, uVeritcalSpeed));
#if 0
			PrintHexData(TCPFrame, "Move");
#endif
			qint64 r = m_UdpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::MoveJoy(short uHorizontalSpeed, short uVeritcalSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetMoveJoyData(uHorizontalSpeed, uVeritcalSpeed));
#if 0
			PrintHexData(TCPFrame, "Move");
#endif
			qint64 r = m_UdpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::Stop()
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		m_StopCommand = Package(GetStopData());
#if 0
		PrintHexData(m_StopCommand, "Stop");
#endif
		qint64 r = m_UdpSocket.write(m_StopCommand.data(), m_StopCommand.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
		else
		{
			m_count = 0;
			m_iRepeatStopTimer = startTimer(REPEAT_STOP_TIMER_ELAPSE);
		}
	}
}

void CUDPConnection::StopJoy()
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		m_StopCommand = Package(GetStopJoyData());
#if 0
		PrintHexData(m_StopCommand, "Stop");
#endif
		qint64 r = m_UdpSocket.write(m_StopCommand.data(), m_StopCommand.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
		else
		{
			m_count = 0;
			m_iRepeatStopTimer = startTimer(REPEAT_STOP_TIMER_ELAPSE);
		}
	}
}



void CUDPConnection::ZoomIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetZoomInData(uSpeed));
#if 0
			PrintHexData(TCPFrame, "ZoomIn");
#endif
			qint64 r = m_UdpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::ZoomOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetZoomOutData(uSpeed));
#if 0
			PrintHexData(TCPFrame, "ZoomOut");
#endif
			qint64 r = m_UdpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::StopZoom()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopZoomData());
			qint64 r = m_UdpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::EnableTrackMode(const TrackModeParam& param)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetEnableTrackModeData(param));
			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::TrackTargetPosition(int iX, int iY)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetTrackTargetPositionData(iX, iY));
			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::TrackTargetPositionEx(const TrackModeParam& param, int iX, int iY)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data1 = GetEnableTrackModeData(param);
			QByteArray data2 = GetTrackTargetPositionData(iX, iY);

			QByteArray TCPFrame = Package(data1.append(data2));
			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::DisableTrackMode()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetDisableTrackModeData());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::FocusIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetFocusInData(uSpeed));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::FocusOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetFocusOutData(uSpeed));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::StopFocus()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopFocusData());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::Home(bool clear)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetHomeData(clear));
#if 0
			PrintHexData(TCPFrame, "Home");
#endif
			if (!clear)
			{
				qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
				Q_ASSERT(r > 0);
				if (r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		}
	}
}

void CUDPConnection::SwitchMotor(bool bOn)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSwitchMotorData(bOn));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::EnableFollowMode(bool bEnable)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetEnableFollowModeData(bEnable));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::TurnTo(double dYaw, double dPitch)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetTurnToData(dYaw, dPitch));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}
//DreamSky 20210913 Calibration 
void CUDPConnection::SetGimbalCalibration(int valueType, double dLat, double dLng, int altValue)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibration(valueType,dLat, dLng,altValue));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}
//DreamSky 20210913 Calibration 
void CUDPConnection::SetGimbalCalibrationUp(double dLat)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationUp(dLat));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}
//DreamSky 20210913 Calibration 
void CUDPConnection::SetGimbalCalibrationDown(double dLat)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationDown(dLat));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}
//DreamSky 20210913 Calibration 
void CUDPConnection::SetGimbalCalibrationLeft(double dLat)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationLeft(dLat));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}
//DreamSky 20210913 Calibration 
void CUDPConnection::SetGimbalCalibrationRight(double dLat)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationRight(dLat));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

//DreamSky 20210913 Calibration 
void CUDPConnection::SetGimbalCalibrationT1(double dLat, double dLng, double dLat1, double dLng1)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationT1(dLat, dLng, dLat1, dLng1));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}




void CUDPConnection::StartTrack()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStartTrackData());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::StartCarRecognition()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStartCarRecognition());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::StartCarTrack()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStartCarTrack());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::StopCarRecognition()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopCarRecognition());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::StopTrack()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopTrackData());

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 0: auto; 1: 32; 2: 64: 3: 128;
void CUDPConnection::SetTrackTemplateSize(int iSize)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetTrackTemplateSizeData(iSize));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SetImageColor(ImageType emImgType, bool bEnablePIP, IRColor emIRColor)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetImageColorData(emImgType, bEnablePIP, emIRColor));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SetImageColorV2(uchar cImgType)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetImageColorDataV2(cImgType));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SetImageColorV3(uchar cIRColor)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetImageColorDataV3(cIRColor));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::Photograph(bool clear)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetPhotographData(clear));
			if (!clear)
			{
				qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
				Q_ASSERT(r > 0);
				if (r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		}
	}
}

void CUDPConnection::SwitchRecord(bool bOn, bool clear)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSwitchRecordData(bOn, clear));
			if (!clear)
			{
				qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
				Q_ASSERT(r > 0);
				if (r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		}
	}
}

void CUDPConnection::SwitchDefog(bool bOn)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSwitchDefogData(bOn));

			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}

			// m_bIsDefogOn = bOn;
		}
	}
}

// 设置录像模式
// iMode: 0 photo mode; 1 record mode
void CUDPConnection::SetRecordMode(int iMode)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSetRecordModeData(iMode));
			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}


// 设置聚焦模式
// iMode: 0 automatically; 1 manually
void CUDPConnection::SetFocusMode(int iMode)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSetFocusModeData(iMode));
			qint64 r = m_UdpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
			m_iFocusMode = iMode;
		}
	}
}

// 激光控制指令
void CUDPConnection::SwitchLaser(bool bOn)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSwitchLaserData(bOn));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::LaserZoomIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetLaserZoomInData(uSpeed));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::LaserZoomOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetLaserZoomOutData(uSpeed));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::LaserStopZoom()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetLaserStopZoomData());
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// iMode: 0 automatically follow EO; 1 manually
void CUDPConnection::SetLaserZoomMode(int iMode)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetLaserZoomModeData(iMode));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 可见光放到到指定倍数字
void CUDPConnection::ZoomTo(float fMag)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetZoomToData(fMag));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 热像仪电子放大缩小
void CUDPConnection::IRDigitalZoomIn(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetIRDigitalZoomInData(uSpeed));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::IRDigitalZoomOut(short uSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetIRDigitalZoomOutData(uSpeed));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 打开关闭EO digital zoom
void CUDPConnection::SwitchEODigitalZoom(bool bOn)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSwitchEODigitalZoomData(bOn));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 进入SBUS模式
void CUDPConnection::EnterSBUSMode()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetEnterSBUSModeData());
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 退出SBUS模式
void CUDPConnection::ExitSBUSMode()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetExitSBUSModeData());
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 查询设备当前配置
void CUDPConnection::QueryDevConfiguration()
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetQueryDevConfigurationData());
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SetTimeZone(bool newOSDVersion,char tz)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{

			if (false)
			{
				QByteArray data = Package(GetTimeZoneDataV2(tz));
#if 0
				PrintHexData(data, "set osd");
#endif
				qint64 r = m_UdpSocket.write(data, data.size());
				Q_ASSERT(r > 0);
				if (r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
		}
			else
			{
				QByteArray data = Package(GetTimeZoneData(tz));
#if 0
				PrintHexData(data, "set osd");
#endif
				qint64 r = m_UdpSocket.write(data, data.size());
				Q_ASSERT(r > 0);
				if (r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		
		}
	}
}

// 设置设备OSD
void CUDPConnection::SetOSD(bool newOSDVersion,bool newOSDVersionLeft, uchar uParam1, uchar uParam2)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			if (newOSDVersion)
			{
				if (newOSDVersionLeft)
				{
					QByteArray data = Package(GetSetOSDDataLeft(uParam1, uParam2));
#if 0
					PrintHexData(data, "set osd");
#endif
					qint64 r = m_UdpSocket.write(data, data.size());
					Q_ASSERT(r > 0);
					if (r <= 0)
					{
						qCritical("tcp socket write failed.");
					}
				}
				else
				{
					QByteArray data = Package(GetSetOSDDataRight(uParam1, uParam2));
#if 0
					PrintHexData(data, "set osd");
#endif
					qint64 r = m_UdpSocket.write(data, data.size());
					Q_ASSERT(r > 0);
					if (r <= 0)
					{
						qCritical("tcp socket write failed.");
					}
				}

			}
			else
			{
				QByteArray data = Package(GetSetOSDData(uParam1, uParam2));
#if 0
				PrintHexData(data, "set osd");
#endif
				qint64 r = m_UdpSocket.write(data, data.size());
				Q_ASSERT(r > 0);
				if (r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}

		}
	}
}

void CUDPConnection::SetTem(bool check,uchar uParam1, uchar uParam2)
{//DreamSky 20210806
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetOSDData(uParam1, uParam2));
#if 0
			PrintHexData(data, "set osd");
#endif
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SetWirelessCtrlChnlMap(const VLK_ChannelsMap& ChnlsMap)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetWirelessCtrlChnlMapData(ChnlsMap));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SetWirelessCtrlChnlMapV2(const VLK_ChannelsMap& ChnlsMap, const VLK_ChannelsInvertFlag& ChnlInvertFlag)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetWirelessCtrlChnlMapData(ChnlsMap));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}

			data = Package(GetSetWirelessCtrlChnlInvertFlagDataPart1(ChnlInvertFlag));
			r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}

			data = Package(GetSetWirelessCtrlChnlInvertFlagDataPart2(ChnlInvertFlag));
			r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}


//DreamSky 20210913 Calibration
void CUDPConnection::SetGyroCalibration(double dLat)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetGyroCalibration(dLat));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

//DreamSky 20210913 Calibration
void CUDPConnection::SetTemAlarm(int tem)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetTemAlarm(tem));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
	
}

//DreamSky 2021-12-16 电子变倍增大
void CUDPConnection::SetDZoomMax(double dLat)
{
	if (m_bIsValidDevice)
	{
		if (m_UdpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetDZoomMax(dLat));
			qint64 r = m_UdpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CUDPConnection::SendExtentCmd(const QByteArray& cmd)
{
	// if (m_bIsValidDevice)
	{
		QByteArray data = Package(cmd);
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

void CUDPConnection::SendExtentCmdReceiveData(bool cmd)
{

}

void CUDPConnection::SetDevConfiguration(const VLK_DevConfig& DeviceCfg)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetDevConfigurationData(DeviceCfg));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

void CUDPConnection::SetGimbalAimPoint(double dLat, double dLng, double dAlt)
{
	if (m_UdpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalAimPointData(dLat, dLng, dAlt));
		qint64 r = m_UdpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}


