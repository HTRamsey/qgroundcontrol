#include "TCPConnection.h"
#include "DeviceConnectionMgrService.h"
#include <QTimerEvent>
#include <QNetworkProxy>
#define MAX_REPEAT_STOP 2
#define REPEAT_STOP_TIMER_ELAPSE 50


CTCPConnection::CTCPConnection(IApplicationContext* pContext)
	: CDeviceConnection(pContext, CT_TCP)
	, m_iPort(0)
	, m_iReadBufferTimer(0)
	, m_iRepeatStopTimer(0)
	, m_pEncoderObject(NULL)
	, m_bKeepAliveSuspended(false)
	, isGetInfoCome(false)
	, isReceiveData(false)
{
	QObject::connect(&m_TcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	QObject::connect(&m_TcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	QObject::connect(&m_TcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
	
}


CTCPConnection::~CTCPConnection()
{
	if(m_pEncoderObject != NULL)
	{
		delete m_pEncoderObject;
		m_pEncoderObject = NULL;
	}
}

void CTCPConnection::SetConnectInfo(const QString& strIP, int iPort)
{
	m_strIP = strIP;
	m_iPort = iPort;

	// m_EncoderObject.SetConnectInfo(m_strIP, m_iPort);
}

void CTCPConnection::GetConnectInfo(QString& strIP, int& iPort)
{
	strIP = m_strIP;
	iPort = m_iPort;
}

CEncoderObject* CTCPConnection::GetEncoderObject()
{
	if(m_pEncoderObject == NULL)
	{
		m_pEncoderObject = new CEncoderObject(this, m_pContext);
	}
	return m_pEncoderObject;   
}

void CTCPConnection::SuspendKeepAlive()
{
	if(m_KeepAliveTimer != 0)
	{
		killTimer(m_KeepAliveTimer);
		m_KeepAliveTimer = 0;
	}

	// 清理掉所有数据    
	m_TcpSocket.readAll();
	m_lockBuffer.lock();
	m_Buffer.clear();
	m_lockBuffer.unlock();

	m_bKeepAliveSuspended = true;
}

void CTCPConnection::ResumeKeepAlive()
{
	if(m_bKeepAliveSuspended)
	{
		// 清理掉所有数据
		m_TcpSocket.readAll();
		m_lockBuffer.lock();
		m_Buffer.clear();
		m_lockBuffer.unlock();

		m_bKeepAliveSuspended = false;

		m_KeepAliveTimer = startTimer(KEEP_ALIVE_INTERVAL);
	}
}

bool CTCPConnection::Connect()
{
	Disconnect();
	m_TcpSocket.setProxy(QNetworkProxy::NoProxy);
	m_TcpSocket.connectToHost(m_strIP, m_iPort);
	// m_tcpSocket.waitForConnected(5000);
	return true;
}

bool CTCPConnection::IsConnected() const
{
	return m_bIsConnected;
}



void CTCPConnection::onConnected()
{
	auto strLocalIP = m_TcpSocket.localAddress().toString();
	auto strRemoteIP = m_TcpSocket.peerAddress().toString();

	qDebug() << __FUNCTION__;
	m_bIsConnected = true;

	CTCPConnectionStatusChangedEventArg* pArg = new CTCPConnectionStatusChangedEventArg();
	pArg->strDeviceIP = m_strIP;
	pArg->iPort = m_iPort;
	pArg->bIsOnline = m_bIsConnected;
	m_pContext->FireEvent(EVT_TCP_CONN_STATUS_CHANGED, pArg, false, false);
	pArg->Release();

	m_KeepAliveTimer = startTimer(KEEP_ALIVE_INTERVAL);
	//m_iReadBufferTimer = startTimer(READ_BUFFER_TIMER_ELAPSE); 
}
 

QByteArray flagData;
void CTCPConnection::onReadyRead()
{
	/*static bool b = [&]()->bool
	{
		flagData.append(char(0x3E));
		flagData.append(char(0x2A));
		flagData.append(char(0x80));
		return true;
	}();
	static int nRecvPacket = 0;
	qDebug() << "recv packet : " << ++nRecvPacket;*/
	// qDebug() << __FUNCTION__;
	QByteArray receive = m_TcpSocket.readAll();
	
	/*if (receive.indexOf(flagData) >= 0)
	{
		m_nQuerySuccCount++;
		qDebug() << "TCP RecvDevInfo Failed Count:" << (m_nQueryCount - m_nQuerySuccCount);
	}*/
	
#if 0
	// qDebug() << "************************begin receive**********************";
	PrintHexData(receive, "*********** receive");
	// qDebug() << "************************end receive**********************";
#endif

	//PrintHexData(receive, "*********** receive");//DreamSky 20210914 打印数据

	// 如果是编码板的相关消息，m_pEncoderObject直接处理掉，不用追加到
	// m_Buffer
	if(m_pEncoderObject != NULL && m_pEncoderObject->OnReceiveData(receive))
	{
		return;
	}

	m_lockBuffer.lock();
	m_Buffer.append(receive);
	m_lockBuffer.unlock();

	if (isReceiveData) {
	//CTCPGetInfoEventArg* pArg = new CTCPGetInfoEventArg();
		CTCPConnectionStatusChangedEventArg* pArg = new CTCPConnectionStatusChangedEventArg();
	if (pArg != NULL ) {
		pArg->strDeviceIP = PrintHexDataGetInfo(receive, "*********** receive");
		m_pContext->FireEvent(EVT_TCP_CONN_STATUS_CHANGED, pArg, false, false);
		pArg->Release();
	}
	}
	
	DoReadBuffer();
	
}


void CTCPConnection::onSocketError(QAbstractSocket::SocketError error)
{
	qCritical() << __FUNCTION__ << m_TcpSocket.errorString();
	m_bIsValidDevice = false;
	m_bIsConnected = false;

	m_lockBuffer.lock();
	m_Buffer.clear();
	m_lockBuffer.unlock();

	CTCPConnectionStatusChangedEventArg* pArg = new CTCPConnectionStatusChangedEventArg();
	pArg->strDeviceIP = m_strIP;
	pArg->iPort = m_iPort;
	pArg->bIsOnline = m_bIsConnected;
	pArg->strErrorMsg = m_TcpSocket.errorString();
	m_pContext->FireEvent(EVT_TCP_CONN_STATUS_CHANGED, pArg, false, false);
	pArg->Release();
}

void CTCPConnection::Disconnect()
{
	m_lockBuffer.lock();  //DreamSky 2021-12-4  断开需要清理下上次的脏数据
	m_Buffer.clear();
	m_lockBuffer.unlock();

	if(m_KeepAliveTimer != 0)
	{
		killTimer(m_KeepAliveTimer);
		m_KeepAliveTimer = 0;
	}

	if(m_iReadBufferTimer != 0)
	{
		killTimer(m_iReadBufferTimer);
		m_iReadBufferTimer = 0;
	}


	if(m_TcpSocket.isOpen())
	{
		m_TcpSocket.disconnectFromHost();
		m_TcpSocket.close();
	}
	else
	{
		m_TcpSocket.abort();
	}

	m_bIsConnected = false;
	m_bIsValidDevice = false;
}

void CTCPConnection::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == m_KeepAliveTimer)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			// 发送心跳包
			QByteArray frame = Package(GetKeepAliveData());
			qint64 r = m_TcpSocket.write(frame, frame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
		/*killTimer(m_KeepAliveTimer);
		m_KeepAliveTimer = 0;*/

		
		/*m_nQueryCount = 0;
		m_nQuerySuccCount = 0;
		m_nTestTimer = startTimer(200);*/
	}
	/*else if (event->timerId() == m_nTestTimer)
	{
		qDebug() << "QueryDevConfiguration count: " << ++m_nQueryCount;
		QueryDevConfiguration();
	}*/
	else if(event->timerId() == m_iReadBufferTimer)
	{
		DoReadBuffer();
	}
	else if(event->timerId() == m_iRepeatStopTimer)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			//因为对方丢包，重复停止指令以规避
			qint64 r = m_TcpSocket.write(m_StopCommand.data(), m_StopCommand.size());
			if(m_count >= MAX_REPEAT_STOP)
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

QByteArray CTCPConnection::Package(const QByteArray& cmd)
{
	QByteArray TCPFrame;
	TCPFrame.resize(4 + cmd.size());
	TCPFrame.fill(0x00);
	TCPFrame[0] = 0xEB; // head
	TCPFrame[1] = 0x90; // function code
	TCPFrame[2] = (uchar)cmd.size(); // data size

	int iCheckSum = 0;
	for(int i = 0; i < cmd.size(); ++i)
	{
		TCPFrame[3 + i] = cmd.at(i);
		iCheckSum += cmd.at(i);
	}
	TCPFrame[4 + cmd.size() - 1] = (uchar)(0xFF & iCheckSum);

	//qDebug() << "----------TCPFrame size: " << TCPFrame.size();
	//qDebug() << "----------TCPFrame data: " << TCPFrame;

#if 0
	PrintHexData(cmd, "send");
#endif
	return TCPFrame;
}

void CTCPConnection::Move(short uHorizontalSpeed, short uVeritcalSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetMoveData(uHorizontalSpeed, uVeritcalSpeed));
#if 0
			PrintHexData(TCPFrame, "Move");
#endif
			qint64 r = m_TcpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::MoveJoy(short uHorizontalSpeed, short uVeritcalSpeed)
{
	if (m_bIsValidDevice)
	{
		if (m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetMoveJoyData(uHorizontalSpeed, uVeritcalSpeed));
#if 0
			PrintHexData(TCPFrame, "Move");
#endif
			qint64 r = m_TcpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::Stop()
{
	if(m_TcpSocket.isOpen() && m_bIsConnected)
	{
		m_StopCommand = Package(GetStopData());
#if 0
		PrintHexData(m_StopCommand, "Stop");
#endif
		qint64 r = m_TcpSocket.write(m_StopCommand.data(), m_StopCommand.size());
		Q_ASSERT(r > 0);
		if(r <= 0)
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


void CTCPConnection::StopJoy()
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		m_StopCommand = Package(GetStopJoyData());
#if 0
		PrintHexData(m_StopCommand, "Stop");
#endif
		qint64 r = m_TcpSocket.write(m_StopCommand.data(), m_StopCommand.size());
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



void CTCPConnection::ZoomIn(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetZoomInData(uSpeed));
#if 0
			PrintHexData(TCPFrame, "ZoomIn");
#endif
			qint64 r = m_TcpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::ZoomOut(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetZoomOutData(uSpeed));
#if 0
			PrintHexData(TCPFrame, "ZoomOut");
#endif
			qint64 r = m_TcpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::StopZoom()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopZoomData());
			qint64 r = m_TcpSocket.write(TCPFrame.data(), TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::EnableTrackMode(const TrackModeParam& param)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetEnableTrackModeData(param));
			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::TrackTargetPosition(int iX, int iY)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetTrackTargetPositionData(iX, iY));
			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::TrackTargetPositionEx(const TrackModeParam& param, int iX, int iY)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{  //DreamSky  20210819  目标跟踪
			QByteArray data1 = GetEnableTrackModeData(param);
			QByteArray data2 = GetTrackTargetPositionData(iX, iY);

			QByteArray TCPFrame = Package(data1.append(data2));
			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::DisableTrackMode()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetDisableTrackModeData());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::FocusIn(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetFocusInData(uSpeed));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::FocusOut(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetFocusOutData(uSpeed));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::StopFocus()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopFocusData());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::Home(bool clear)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetHomeData(clear));
#if 0
			PrintHexData(TCPFrame, "Home");
#endif
			if(!clear)
			{
				qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
				Q_ASSERT(r > 0);
				if(r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		}
	}
}

void CTCPConnection::SwitchMotor(bool bOn)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSwitchMotorData(bOn));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::EnableFollowMode(bool bEnable)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetEnableFollowModeData(bEnable));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::TurnTo(double dYaw, double dPitch)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetTurnToData(dYaw, dPitch));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

//DreamSky 20210913 Calibration
void CTCPConnection::SetGyroCalibration(double dLat)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGyroCalibration(dLat));

		PrintHexData(data, "SetGimbalCalibration");
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}


//DreamSky 20210913 Calibration
void CTCPConnection::SetGimbalCalibration(int valueType,double dLat, double dLng,int altValue)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibration(valueType,dLat, dLng, altValue));

		PrintHexData(data, "SetGimbalCalibration");
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

//DreamSky 20210913 Calibration
void CTCPConnection::SetGimbalCalibrationUp(double dLat)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationUp(dLat));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}


//DreamSky 20210913 Calibration
void CTCPConnection::SetGimbalCalibrationDown(double dLat)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationDown(dLat));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

//DreamSky 20210913 Calibration
void CTCPConnection::SetGimbalCalibrationLeft(double dLat)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationLeft(dLat));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}


//DreamSky 20210913 Calibration   
void CTCPConnection::SetGimbalCalibrationRight(double dLat)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationRight(dLat));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}


//DreamSky 20210913 Calibration
void CTCPConnection::SetTemAlarm(int tem)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetTemAlarm(tem));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

//DreamSky 2021-12-16 电子变倍增大
void CTCPConnection::SetDZoomMax(double dLat)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetDZoomMax(dLat));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}



//DreamSky 20210913 Calibration
void CTCPConnection::SetGimbalCalibrationT1(double dLat, double dLng, double dLat1, double dLng1)
{
	if (m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalCalibrationT1(dLat, dLng, dLat1, dLng1));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if (r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

void CTCPConnection::StartTrack()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStartTrackData());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::StartCarRecognition()
{
	if (m_bIsValidDevice)
	{
		if (m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStartCarRecognition());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::StartCarTrack()
{
	if (m_bIsValidDevice)
	{
		if (m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStartCarTrack());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::StopCarRecognition()
{
	if (m_bIsValidDevice)
	{
		if (m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopCarRecognition());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}


void CTCPConnection::StopTrack()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetStopTrackData());

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 0: auto; 1: 32; 2: 64: 3: 128;
void CTCPConnection::SetTrackTemplateSize(int iSize)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetTrackTemplateSizeData(iSize));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SetImageColor(ImageType emImgType, bool bEnablePIP, IRColor emIRColor)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetImageColorData(emImgType, bEnablePIP, emIRColor));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SetImageColorV2(uchar cImgType)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetImageColorDataV2(cImgType));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SetImageColorV3(uchar cIRColor)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetImageColorDataV3(cIRColor));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::Photograph(bool clear)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetPhotographData(clear));
			if(!clear)
			{
				qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
				Q_ASSERT(r > 0);
				if(r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		}
	}
}

void CTCPConnection::SwitchRecord(bool bOn, bool clear)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSwitchRecordData(bOn, clear));
			if(!clear)
			{
				qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
				Q_ASSERT(r > 0);
				if(r <= 0)
				{
					qCritical("tcp socket write failed.");
				}
			}
		}
	}
}

void CTCPConnection::SwitchDefog(bool bOn)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSwitchDefogData(bOn));

			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}

			// m_bIsDefogOn = bOn;
		}
	}
}

// 设置录像模式
// iMode: 0 photo mode; 1 record mode
void CTCPConnection::SetRecordMode(int iMode)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSetRecordModeData(iMode));
			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}


// 设置聚焦模式
// iMode: 0 automatically; 1 manually
void CTCPConnection::SetFocusMode(int iMode)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray TCPFrame = Package(GetSetFocusModeData(iMode));
			qint64 r = m_TcpSocket.write(TCPFrame, TCPFrame.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
			m_iFocusMode = iMode;
		}
	}
}

// 激光控制指令
void CTCPConnection::SwitchLaser(bool bOn)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSwitchLaserData(bOn));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::LaserZoomIn(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetLaserZoomInData(uSpeed));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::LaserZoomOut(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetLaserZoomOutData(uSpeed));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::LaserStopZoom()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetLaserStopZoomData());
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// iMode: 0 automatically follow EO; 1 manually
void CTCPConnection::SetLaserZoomMode(int iMode)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetLaserZoomModeData(iMode));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 可见光放到到指定倍数字
void CTCPConnection::ZoomTo(float fMag)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetZoomToData(fMag));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 热像仪电子放大缩小
void CTCPConnection::IRDigitalZoomIn(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetIRDigitalZoomInData(uSpeed));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::IRDigitalZoomOut(short uSpeed)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetIRDigitalZoomOutData(uSpeed));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 打开关闭EO digital zoom
void CTCPConnection::SwitchEODigitalZoom(bool bOn)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSwitchEODigitalZoomData(bOn));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 进入SBUS模式
void CTCPConnection::EnterSBUSMode()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetEnterSBUSModeData());
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 退出SBUS模式
void CTCPConnection::ExitSBUSMode()
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetExitSBUSModeData());
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

// 查询设备当前配置
void CTCPConnection::QueryDevConfiguration()
{
	qDebug() << "QueryDevConfiguration:" << m_bIsValidDevice;
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetQueryDevConfigurationData());
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SetTimeZone(bool newOSDVersion,char tz)
{
	if (m_bIsValidDevice)
	{
		if (m_TcpSocket.isOpen() && m_bIsConnected)
		{

			if (false)
			{
				QByteArray data = Package(GetTimeZoneDataV2(tz));
#if 0
				PrintHexData(data, "set osd");
#endif
				qint64 r = m_TcpSocket.write(data, data.size());
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
				qint64 r = m_TcpSocket.write(data, data.size());
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
void CTCPConnection::SetOSD(bool newOSDVersion,bool newOSDVersionLeft,uchar uParam1, uchar uParam2)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			if (newOSDVersion)
			{

				if (newOSDVersionLeft)
				{
					QByteArray data = Package(GetSetOSDDataLeft(uParam1, uParam2));
#if 0
					PrintHexData(data, "set osd");
#endif
					qint64 r = m_TcpSocket.write(data, data.size());
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
					qint64 r = m_TcpSocket.write(data, data.size());
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
				qint64 r = m_TcpSocket.write(data, data.size());
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
void CTCPConnection::SetTem(bool check,uchar uParam1, uchar uParam2)
{
	if (m_bIsValidDevice)
	{
		if (m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetOSDData(uParam1, uParam2));
#if 0
			PrintHexData(data, "set osd");
#endif
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if (r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SetWirelessCtrlChnlMap(const VLK_ChannelsMap& ChnlsMap)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)
		{
			QByteArray data = Package(GetSetWirelessCtrlChnlMapData(ChnlsMap));
			qint64 r = m_TcpSocket.write(data, data.size());
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SetWirelessCtrlChnlMapV2(const VLK_ChannelsMap& ChnlsMap, const VLK_ChannelsInvertFlag& ChnlInvertFlag)
{
	if(m_bIsValidDevice)
	{
		if(m_TcpSocket.isOpen() && m_bIsConnected)  
		{
			QByteArray data = Package(GetSetWirelessCtrlChnlMapData(ChnlsMap));
			PrintHexData(data, "rocker ChnlMapData");
			qint64 r = m_TcpSocket.write(data, data.size());
			m_TcpSocket.flush();
			_sleep(20);
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}

			data = Package(GetSetWirelessCtrlChnlInvertFlagDataPart1(ChnlInvertFlag));
			PrintHexData(data, "rocker FlagDataPart1");
			r = m_TcpSocket.write(data, data.size());
			m_TcpSocket.flush();
			_sleep(20);

			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}

			data = Package(GetSetWirelessCtrlChnlInvertFlagDataPart2(ChnlInvertFlag));
			PrintHexData(data, "rocker FlagDataPart2");
			r = m_TcpSocket.write(data, data.size());
			m_TcpSocket.flush();
			_sleep(20);
			Q_ASSERT(r > 0);
			if(r <= 0)
			{
				qCritical("tcp socket write failed.");
			}
		}
	}
}

void CTCPConnection::SendExtentCmd(const QByteArray& cmd)
{
	isGetInfoCome = true;
	// if (m_bIsValidDevice)  DreamSky 20210812 15.49
	{

		QByteArray data = Package(cmd);
		PrintHexData(data, "set cmd");
		
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if(r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

void CTCPConnection::SendExtentCmdReceiveData(bool cmd)
{
	isReceiveData = cmd;
}

void CTCPConnection::SetDevConfiguration(const VLK_DevConfig& DeviceCfg)
{
	if(m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetDevConfigurationData(DeviceCfg));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if(r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

void CTCPConnection::SetGimbalAimPoint(double dLat, double dLng, double dAlt)
{
	if(m_TcpSocket.isOpen() && m_bIsConnected)
	{
		QByteArray data = Package(GetSetGimbalAimPointData(dLat, dLng, dAlt));
		qint64 r = m_TcpSocket.write(data, data.size());
		Q_ASSERT(r > 0);
		if(r <= 0)
		{
			qCritical("tcp socket write failed.");
		}
	}
}

void CTCPConnection::sendMessage(QString msg)
{
}


