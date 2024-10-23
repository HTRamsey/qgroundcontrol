#include "DeviceConnection.h"
#include <QDebug>
#include "DeviceConnectionMgrService.h"
#include "ILocalOptionsService.h"
#include <windows.h>
#include "../UI/DevModel.h"

// #define __DUMP_RECEIVE_DATA__
#define __REVERSE_PITCH__

QByteArray CDeviceConnection::s_VLKHead;
QByteArray CDeviceConnection::s_VLKHeadV2;

void PrintHexData(const QByteArray& data, const QString& strTag)
{
	QString str;
	for (int i = 0; i < data.size(); ++i)
	{
		char temp[8] = { 0 };
		sprintf(temp, "%02x ", (uchar)data[i]);
		str += temp;
	}
	
	qDebug() << QString("%1 size = %2, data: %3").arg(strTag).arg(data.size()).arg(str);
	

}


QString PrintHexDataGetInfo(const QByteArray& data, const QString& strTag)
{

	
	QString str;
	for (int i = 0; i < data.size(); ++i)
	{
		char temp[8] = { 0 };
		sprintf(temp, "%02x ", (uchar)data[i]);
		str += temp;
	}
	qDebug() << QString("%1 size = %2, data: %3").arg(strTag).arg(data.size()).arg(str);
	return str;
	
}


CDeviceConnection::CDeviceConnection(IApplicationContext* pContext, ConnectionType emType)
	: m_pContext(pContext), m_emConnectionType(emType)
	, m_bIsValidDevice(false)
	, m_KeepAliveTimer(0)
	, m_ucDeviceType(0)
	, m_iFocusMode(0)
	, m_bIsConnected(false)
{
	GetDataByFrameID(0x30, m_CommonCmd, m_iCommonBeginPos, m_iCommonEndPos);
	GetDataByFrameID(0x31, m_UncommonCmd, m_iUncommonBeginPos, m_iUncommonEndPos);

	for (int i = 0; i < LOOP_FRAME_NO_COUNT; ++i)
	{
		m_sCrcTabElementIndex[i] = LoopFrameNO[i];
		m_sCrcTabElementIndex[i] = m_sCrcTabElementIndex[i] << 8;
	}

	memset(&m_DevConfig, 0, sizeof(m_DevConfig));

	s_VLKHead.resize(3);
	s_VLKHead[0] = 0x55;
	s_VLKHead[1] = 0xAA;
	s_VLKHead[2] = 0xDC;

	s_VLKHeadV2.resize(4);
	s_VLKHeadV2[0] = 0x3E;
	s_VLKHeadV2[1] = 0x2A;
	s_VLKHeadV2[2] = 0x80;
	s_VLKHeadV2[3] = 0xAA;
}

CDeviceConnection::~CDeviceConnection()
{
}

void CDeviceConnection::SetApplicationContext(IApplicationContext* pContext)
{
	m_pContext = pContext;
}

ConnectionType CDeviceConnection::GetConnectionType()
{
	return m_emConnectionType;
}

QByteArray CDeviceConnection::GetKeepAliveData()
{
	// ����Ѿ����ֳɹ�, ���Ǿ����ϴ��յ������ظ��Ѿ�����5����
	// ����Ϊ������Ч��
	if (m_bIsValidDevice && m_tmPreActiveTime.secsTo(QTime::currentTime()) > 10)
	{
		qDebug() << "keep alive package out of time, this software offline. keep sending hand shake package until receive hand shake response";
		m_bIsValidDevice = false;
	}

	// ����Э���������е�4���ֽ�Ӧ��Я��Ӧ�ô���ѭ��֡��

	static ulong uFrameIndex = 0;
	uchar ucFrameNo = LoopFrameNO[uFrameIndex % (sizeof(LoopFrameNO))];
	++uFrameIndex;

	if (!m_bIsValidDevice) // �����û�����ֳɹ�
	{
		QByteArray SayHello;
		int begin = 0, end = 0;
		GetDataByFrameID(0x00, SayHello, begin, end);

		SayHello[(int)sizeof(VLK_HEAD)] = SayHello[(int)sizeof(VLK_HEAD)] | ucFrameNo;

		CalculateCheckSum(SayHello);
		return SayHello;
	}
	else // �Ѿ����ֳɹ�
	{
		QByteArray KeepAlive;
		int begin = 0, end = 0;
		GetDataByFrameID(0x00, KeepAlive, begin, end);

		KeepAlive[(int)sizeof(VLK_HEAD)] = KeepAlive[(int)sizeof(VLK_HEAD)] | ucFrameNo;

		// Я���豸����
		KeepAlive[(int)sizeof(VLK_HEAD) + 1] = 0x10 | m_ucDeviceType;

		// ����Э��
		qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()));
		uchar ucCrcTabElementIndex = qrand() % sizeof(s_CrcTab);
		KeepAlive[(int)sizeof(VLK_HEAD) + 2] = ucCrcTabElementIndex;

		// ����У������index����Ա����
		for (int i = 0; i < LOOP_FRAME_NO_COUNT; ++i)
		{
			uchar highbyte = (m_sCrcTabElementIndex[i] >> 8);
			if (highbyte == ucFrameNo)
			{
				m_sCrcTabElementIndex[i] &= 0xFF00;
				m_sCrcTabElementIndex[i] |= ucCrcTabElementIndex;
				break;
			}
			// qDebug("===== %02x:%02x", highbyte, m_sCrcTabElementIndex[i] & 0xFF);
		}
		// qDebug("===== send ucFrameNo = %02x, ucCrcTabElementIndex = %02x, s_CrcTab[%02x] = %02x",
		//	ucFrameNo, ucCrcTabElementIndex, ucCrcTabElementIndex, s_CrcTab[ucCrcTabElementIndex]);

		CalculateCheckSum(KeepAlive);
		return KeepAlive;
	}
}

QByteArray CDeviceConnection::GetMoveData(short uHorizontalSpeed, short uVeritcalSpeed)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x01;  
	m_CommonCmd[m_iCommonBeginPos + 1] = 0xFF & (uHorizontalSpeed >> 8);
	m_CommonCmd[m_iCommonBeginPos + 2] = 0xFF & uHorizontalSpeed;

	m_CommonCmd[m_iCommonBeginPos + 3] = 0xFF & (uVeritcalSpeed >> 8);
	m_CommonCmd[m_iCommonBeginPos + 4] = 0xFF & uVeritcalSpeed;

	//// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);

	return m_CommonCmd;
}



QByteArray CDeviceConnection::GetMoveJoyData(short uHorizontalSpeed, short uVeritcalSpeed)
{
	
	uchar* puHorizontalSpeed = (uchar*)&uHorizontalSpeed;
	uchar* puVeritcalSpeed = (uchar*)&uVeritcalSpeed;

	QByteArray moveJoyData;
	moveJoyData.resize(20);
	moveJoyData[0] = 0x55;
	moveJoyData[1] = 0xAA;
	moveJoyData[2] = 0xDC;

	moveJoyData[3] = 0x11;
	moveJoyData[4] = 0x30;

	moveJoyData[5] = 0x0D;

	moveJoyData[6] = 0x00;
	moveJoyData[7] = 0x00;

	moveJoyData[8] = puHorizontalSpeed[1];
	moveJoyData[9] = puHorizontalSpeed[0];

	moveJoyData[10] = 0x00;
	moveJoyData[11] = 0x00;

	moveJoyData[12] = puVeritcalSpeed[1];
	moveJoyData[13] = puVeritcalSpeed[0];
	moveJoyData[14] = 0x00;
	moveJoyData[15] = 0x00;
	moveJoyData[16] = 0x00;

	moveJoyData[17] = 0x00;
	moveJoyData[18] = 0x00;
	moveJoyData[19] = 0x00;

	// checksum 
	CalculateCheckSum(moveJoyData);
	return moveJoyData;
}

QByteArray CDeviceConnection::GetStopData()
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x01;
	m_CommonCmd[m_iCommonBeginPos + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + 2] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + 3] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + 4] = 0x00;

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}


QByteArray CDeviceConnection::GetStopJoyData()
{   //55 AA DC 11 30 0D 00 00 05 DC 00 00 05 DC 00 00 00 00 00 2C
	QByteArray stopJoyData;
	stopJoyData.resize(20);
	stopJoyData[0] = 0x55;
	stopJoyData[1] = 0xAA;
	stopJoyData[2] = 0xDC;

	stopJoyData[3] = 0x11;
	stopJoyData[4] = 0x30;

	stopJoyData[5] = 0x0D;

	stopJoyData[6] = 0x00;
	stopJoyData[7] = 0x00;

	stopJoyData[8] = 0x05;
	stopJoyData[9] = 0xDC;
	stopJoyData[10] = 0x00;
	stopJoyData[11] = 0x00;
	stopJoyData[12] = 0x05;
	stopJoyData[13] = 0xDC;
	stopJoyData[14] = 0x00;
	stopJoyData[15] = 0x00;
	stopJoyData[16] = 0x00;

	stopJoyData[17] = 0x00;
	stopJoyData[18] = 0x00;
	stopJoyData[19] = 0x2C;
	return stopJoyData;
}




QByteArray CDeviceConnection::GetZoomInData(short uSpeed)
{
	//// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short uC1 = 0;
	uC1 |= (uSpeed << 3);
	uC1 |= (0x9 << 6);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (uC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & uC1;

	//// E1
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetZoomOutData(short uSpeed)
{
	//// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short uC1 = 0;
	uC1 |= (uSpeed << 3);
	uC1 |= (0x8 << 6);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (uC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & uC1;

	//// E1
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetStopZoomData()
{
	//// A1
	//m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	//// C1
	//short uC1 = /*(m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] << 8)*/0;
	//// uC1 |= m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1];
	//short uMask = ~(0x7F << 6);
	//uC1 &= uMask;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (uC1 >> 8);
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & uC1;

	////// E1
	////m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	////m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	////m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	//// checksum
	//CalculateCheckSum(m_CommonCmd);



	//55 AA DC 11 30 0F 00 00 00 00 00 00 00 00 00 40 00 00 00 6E
	QByteArray stopZoomData;
	stopZoomData.resize(20);
	stopZoomData[0] = 0x55;
	stopZoomData[1] = 0xAA;
	stopZoomData[2] = 0xDC;

	stopZoomData[3] = 0x11;
	stopZoomData[4] = 0x30;

	stopZoomData[5] = 0x0F;

	stopZoomData[6] = 0x00;
	stopZoomData[7] = 0x00;

	stopZoomData[8] = 0x00;
	stopZoomData[9] = 0x00;
	stopZoomData[10] = 0x00;
	stopZoomData[11] = 0x00;
	stopZoomData[12] = 0x00;
	stopZoomData[13] = 0x00;
	stopZoomData[14] = 0x00;
	stopZoomData[15] = 0x40;
	stopZoomData[16] = 0x00;

	stopZoomData[17] = 0x00;
	stopZoomData[18] = 0x00;
	stopZoomData[19] = 0x6E;
	return stopZoomData;
}

QByteArray CDeviceConnection::GetFocusInData(short uSpeed)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F;// A1 �޶���

	// C1
	short uC1 = 0;
	uC1 |= (uSpeed << 3);
	uC1 |= (0xB << 6);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (uC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & uC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetFocusOutData(short uSpeed)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F;// A1 �޶���

	// C1
	short uC1 = 0;
	uC1 |= (uSpeed << 3);
	uC1 |= (0x0A << 6);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (uC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & uC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetStopFocusData()
{
	return GetStopZoomData();
}

QByteArray CDeviceConnection::GetEnableTrackModeData(const TrackModeParam& param)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x06;

	// C1
	short uC1 = param.iTrackLight;
	uC1 |= (1 << 4);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (uC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & uC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetTrackTargetPositionData(int iX, int iY)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x0;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x0;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	// Ŀǰ��֧�֣����ٵ�ת�Ƶ�ָ��λ��
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x0A;
	// x ����
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0xFF & (iX >> 8);
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0xFF & iX;
	// y ����
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0xFF & (iY >> 8);
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0xFF & iY;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetDisableTrackModeData()
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x01;

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x01;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x01;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

//DreamSky 2021-11-8 �򿪳���ʶ��
QByteArray CDeviceConnection::GetStartCarRecognition()
{
	
	QByteArray carData;
	carData.resize(20);
	carData[0] = 0x55;
	carData[1] = 0xAA;
	carData[2] = 0xDC;

	carData[3] = 0x11;
	carData[4] = 0x30;

	carData[5] = 0x0F;

	carData[6] = 0x00;
	carData[7] = 0x00;

	carData[8] = 0x00;
	carData[9] = 0x00;
	carData[10] = 0x00;
	carData[11] = 0x00;
	carData[12] = 0x00;
	carData[13] = 0x00;
	carData[14] = 0x00;
	carData[15] = 0x00;
	carData[16] = 0x00;

	carData[17] = 0x05;
	carData[18] = 0x01;
	carData[19] = 0x2A;
	return carData;
}

// ʶ��ת����  DreamSky 2021-11-9
QByteArray CDeviceConnection::GetStartCarTrack() {
	
	QByteArray carTrackData;
	carTrackData.resize(20);
	carTrackData[0] = 0x55;
	carTrackData[1] = 0xAA;
	carTrackData[2] = 0xDC;

	carTrackData[3] = 0x11;
	carTrackData[4] = 0x30;

	carTrackData[5] = 0x0F;

	carTrackData[6] = 0x00;
	carTrackData[7] = 0x00;

	carTrackData[8] = 0x00;
	carTrackData[9] = 0x00;
	carTrackData[10] = 0x00;
	carTrackData[11] = 0x00;
	carTrackData[12] = 0x00;
	carTrackData[13] = 0x00;
	carTrackData[14] = 0x00;
	carTrackData[15] = 0x00;
	carTrackData[16] = 0x00;

	carTrackData[17] = 0x08;
	carTrackData[18] = 0x00;
	carTrackData[19] = 0x26;
	return carTrackData;
}


//DreamSky 2021-11-8 �رճ���ʶ��
QByteArray CDeviceConnection::GetStopCarRecognition()
{

	QByteArray stopCarData;
	stopCarData.resize(20);
	stopCarData[0] = 0x55;
	stopCarData[1] = 0xAA;
	stopCarData[2] = 0xDC;

	stopCarData[3] = 0x11;
	stopCarData[4] = 0x30;

	stopCarData[5] = 0x0F;

	stopCarData[6] = 0x00;
	stopCarData[7] = 0x00;

	stopCarData[8] = 0x00;
	stopCarData[9] = 0x00;
	stopCarData[10] = 0x00;
	stopCarData[11] = 0x00;
	stopCarData[12] = 0x00;
	stopCarData[13] = 0x00;
	stopCarData[14] = 0x00;
	stopCarData[15] = 0x00;
	stopCarData[16] = 0x00;

	stopCarData[17] = 0x05;
	stopCarData[18] = 0x00;
	stopCarData[19] = 0x2B;
	return stopCarData;
}

 
QByteArray CDeviceConnection::GetHomeData(bool clear)
{
	// A1   DreamSky 2021-10-16
	m_CommonCmd[m_iCommonBeginPos] = clear ? 0x0F : 0x04;
	/*QByteArray homeData;   55 AA DC 0C 1A 04 00 00 00 00 00 00 00 00  12
	homeData.resize(15);
	homeData[0] = 0x55;
	homeData[1] = 0xAA;
	homeData[2] = 0xDC;

	homeData[3] = 0x0C;
	homeData[4] = 0x1A;

	homeData[5] = 0x04;
	
	homeData[6] = 0x00;
	homeData[7] = 0x00;

	homeData[8] = 0x00;
	homeData[9] = 0x00;
	homeData[10] = 0x00;
	homeData[11] = 0x00;
	homeData[12] = 0x00;
	homeData[13] = 0x00;
	homeData[14] = 0x12;*/

	//// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	//// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd); //DreamSky 2021 - 10 - 16
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetSwitchMotorData(bool bOn)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x00;
	if (bOn)
	{
		m_CommonCmd[m_iCommonBeginPos + 1] = 0x01;
		m_CommonCmd[m_iCommonBeginPos + 2] = 0x00;
	}
	else
	{
		m_CommonCmd[m_iCommonBeginPos + 1] = 0x00;
		m_CommonCmd[m_iCommonBeginPos + 2] = 0x01;
	}

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetEnableFollowModeData(bool bEnable)
{
	// A1
	if (bEnable)
	{
		m_CommonCmd[m_iCommonBeginPos] = 0x03;
	}
	else
	{
		m_CommonCmd[m_iCommonBeginPos] = 0x0A;
	}

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

//DreamSky 20210913 Calibration
QByteArray CDeviceConnection::GetSetGimbalCalibration(int valueType,double dLat, double dLng,int altValue)
{
	
	int fLat = dLat*10000000;
	uchar* pLat = (uchar*)&fLat;
	

	int fLng = dLng * 10000000;
	uchar* pLng = (uchar*)&fLng;


	
	int faltValue = altValue * 1;

	
	uchar* pAlt = (uchar*)&faltValue;

	QByteArray calibrationData;
	calibrationData.resize(20);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x11;
	calibrationData[4] = 0x16;
	calibrationData[5] = 0x05;

	//none 0  LRF  0x04   Alt 0x06      Pitch err  0x02   Yaw err 0x01 
	if (valueType==0) {
		calibrationData[6] = 0x00; //none 0
	}else if (valueType == 82) {
		calibrationData[6] = 0x0A;//LRF  0x82   alt ������1000 
	}
	else if (valueType == 6) {
		calibrationData[6] = 0x0E;//Alt 0xc2      
	}
	else if (valueType == 4) {
		calibrationData[6] = 0x04;//Pitch err  0x04   
	}
	else if (valueType == 1) {
		calibrationData[6] = 0x01;//Yaw err 0x01 
	}
	
	calibrationData[7] = pLat[3];
	calibrationData[8] = pLat[2];
	calibrationData[9] = pLat[1];
	calibrationData[10] = pLat[0];

	calibrationData[11] = pLng[3];
	calibrationData[12] = pLng[2];
	calibrationData[13] = pLng[1];
	calibrationData[14] = pLng[0];


	if (valueType == 0) {//None
		calibrationData[15] = 0x00;          
		calibrationData[16] = 0x00;           
		calibrationData[17] = 0x00;
		calibrationData[18] = 0x00;
	}
	else if (valueType == 82) {//LRF  0x04  
		
		calibrationData[15] = 0x00;             
		calibrationData[16] = 0x00;          
		calibrationData[17] = pAlt[1];//�߶�ֵ 1�׵Ļ����Ǿ���0x00  //pitch err    
		calibrationData[18] = pAlt[0];//�߶�ֵ 1�׵Ļ����Ǿ���0x01  // pitch err 
	}
	else if (valueType == 6) {//Alt 0x06 
		calibrationData[15] = 0x00;           //Yaw err   
		calibrationData[16] = 0x00;           //Yaw err
		calibrationData[17] = pAlt[1];//�߶�ֵ 1�׵Ļ����Ǿ���0x00  //pitch err    
		calibrationData[18] = pAlt[0];//�߶�ֵ 1�׵Ļ����Ǿ���0x01  // pitch err 
	}
	else if (valueType == 4) {//pitch err  
		calibrationData[15] = 0x00;          
		calibrationData[16] = 0x00;          
		calibrationData[17] = pAlt[1];
		calibrationData[18] = pAlt[0]; 
	}
	else if (valueType == 1) {//Yaw err
		calibrationData[15] = pAlt[1];  
		calibrationData[16] = pAlt[0];
		calibrationData[17] = 0x00;
		calibrationData[18] = 0x00;
	}

	

	calibrationData[19] = 0x00;//У��Ԥ��



	/*m_CommonCmd[m_iCommonBeginPos] = 0x16;


	m_CommonCmd[m_iCommonBeginPos+1] = 0x05;


	float fLat = (float)dLat;
	uchar* pLat = (uchar*)&fLat;
	m_CommonCmd[m_iCommonBeginPos + 1] = pLat[3];
	m_CommonCmd[m_iCommonBeginPos + 2] = pLat[2];
	m_CommonCmd[m_iCommonBeginPos + 3] = pLat[1];
	m_CommonCmd[m_iCommonBeginPos + 4] = pLat[0];

	float fLng = (float)dLng;
	uchar* pLng = (uchar*)&fLng;
	m_CommonCmd[m_iCommonBeginPos + 5] = pLng[3];
	m_CommonCmd[m_iCommonBeginPos + 6] = pLng[2];
	m_CommonCmd[m_iCommonBeginPos + 7] = pLng[1];
	m_CommonCmd[m_iCommonBeginPos + 8] = pLng[0];

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH  + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH  + 1] = 0x00;
	
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;*/

	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}


//DreamSky 20210913 Calibration
QByteArray CDeviceConnection::GetSetGimbalCalibrationUp(double dLat)
{
	int fLat = dLat * 1000;
	uchar* pLat = (uchar*)&fLat;

	QByteArray calibrationData;
	calibrationData.resize(8);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x05;
	calibrationData[4] = 0x2A;
	//���� 0x01    0x02 ����
	calibrationData[5] = 0x01;
	//calibrationData[5] = 0x02;
	calibrationData[6] = fLat;
	calibrationData[7] = 0x00;//У��Ԥ��

	  
	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}

//DreamSky 2021-12-8 Calibration
QByteArray CDeviceConnection::GetSetGyroCalibration(double dLat)
{
	int16_t fLat = dLat /0.001;
	uchar* pLat = (uchar*)&fLat;

	QByteArray gyroCalibrationData;
	gyroCalibrationData.resize(15);
	gyroCalibrationData[0] = 0x55;
	gyroCalibrationData[1] = 0xAA;
	gyroCalibrationData[2] = 0xDC;

	gyroCalibrationData[3] = 0x0C;
	gyroCalibrationData[4] = 0x1A;
	gyroCalibrationData[5] = 0x10;
	
	gyroCalibrationData[6] = pLat[1];//0x00
	gyroCalibrationData[7] = pLat[0];//0x01

	gyroCalibrationData[8] = 0x00;
	gyroCalibrationData[9] = 0x00;
	gyroCalibrationData[10] = 0x00;
	gyroCalibrationData[11] = 0x00;
	gyroCalibrationData[12] = 0x00;
	gyroCalibrationData[13] = 0x00;

	gyroCalibrationData[14] = 0x00;//У��Ԥ��


	// checksum 
	CalculateCheckSum(gyroCalibrationData);
	return gyroCalibrationData;
}


QByteArray CDeviceConnection::GetSetGimbalCalibrationDown(double dLat)
{
	int fLat = dLat * 1000;
	uchar* pLat = (uchar*)&fLat;

	QByteArray calibrationData;
	calibrationData.resize(8);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x05;
	calibrationData[4] = 0x2A;
	//���� 0x01    0x02 ����
	calibrationData[5] = 0x01;
	//calibrationData[5] = 0x02;
	calibrationData[6] = -fLat;
	calibrationData[7] = 0x00;//У��Ԥ��


	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}

//DreamSky 20210913 Calibration
QByteArray CDeviceConnection::GetSetGimbalCalibrationLeft(double dLat)
{
	int fLat = dLat * 1000;
	uchar* pLat = (uchar*)&fLat;

	QByteArray calibrationData;
	calibrationData.resize(8);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x05;
	calibrationData[4] = 0x2A;
	//���� 0x01    0x02 ����
	//calibrationData[5] = 0x01;
	calibrationData[5] = 0x02;
	calibrationData[6] = -fLat;
	calibrationData[7] = 0x00;//У��Ԥ��


	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}

//DreamSky 20210913 Calibration
QByteArray CDeviceConnection::GetSetGimbalCalibrationRight(double dLat)
{
	int fLat = dLat * 1000;
	uchar* pLat = (uchar*)&fLat;

	QByteArray calibrationData;
	calibrationData.resize(8);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x05;
	calibrationData[4] = 0x2A;
	//���� 0x01    0x02 ����
	//calibrationData[5] = 0x01;
	calibrationData[5] = 0x02;
	calibrationData[6] = fLat;
	calibrationData[7] = 0x00;//У��Ԥ��


	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}


QByteArray CDeviceConnection::GetSetTemAlarm(int tem)
{
	int fLat = tem * 1000;
	uchar* pLat = (uchar*)&fLat;

	QByteArray calibrationData;
	calibrationData.resize(8);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x05;
	calibrationData[4] = 0x2A;
	//���� 0x01    0x02 ����
	//calibrationData[5] = 0x01;
	calibrationData[5] = 0x02;
	calibrationData[6] = fLat;
	calibrationData[7] = 0x00;//У��Ԥ��


	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}




//DreamSky 2021-12-16 DZoomMax
QByteArray CDeviceConnection::GetSetDZoomMax(double dLat)
{
	int16_t fLat = dLat / 0.1;
	uchar* pLat = (uchar*)&fLat;

	QByteArray DZoomMaxData;
	DZoomMaxData.resize(9);
	DZoomMaxData[0] = 0x55;
	DZoomMaxData[1] = 0xAA;
	DZoomMaxData[2] = 0xDC;
	DZoomMaxData[3] = 0x06;
	DZoomMaxData[4] = 0x2C;
	DZoomMaxData[5] = 0x08;
	DZoomMaxData[6] = pLat[1];//0x00
	DZoomMaxData[7] = pLat[0];//0x01
	DZoomMaxData[8] = 0x00;//У��Ԥ��
	// checksum 
	CalculateCheckSum(DZoomMaxData);
	return DZoomMaxData;
}


//DreamSky 20210913 Calibration
QByteArray CDeviceConnection::GetSetGimbalCalibrationT1(double dLat, double dLng, double dLat1, double dLng1)
{
	int fLat = dLat * 10000000;
	uchar* pLat = (uchar*)&fLat;


	int fLng = dLng * 10000000;
	uchar* pLng = (uchar*)&fLng;

	int fLat1 = dLat1 * 10000000;
	uchar* pLat1 = (uchar*)&fLat1;


	int fLng1 = dLng * 10000000;
	uchar* pLng1 = (uchar*)&fLng1;


	QByteArray calibrationData;
	calibrationData.resize(28);
	calibrationData[0] = 0x55;
	calibrationData[1] = 0xAA;
	calibrationData[2] = 0xDC;

	calibrationData[3] = 0x19;
	calibrationData[4] = 0x17;
	calibrationData[5] = 0x00;
	calibrationData[6] = 0x00;

	calibrationData[7] = pLat[3];
	calibrationData[8] = pLat[2];
	calibrationData[9] = pLat[1];
	calibrationData[10] = pLat[0];

	calibrationData[11] = pLng[3];
	calibrationData[12] = pLng[2];
	calibrationData[13] = pLng[1];
	calibrationData[14] = pLng[0];

	calibrationData[15] = 0x00;
	calibrationData[16] = 0x00;

	calibrationData[17] = pLat1[3];
	calibrationData[18] = pLat1[2];
	calibrationData[19] = pLat1[1];
	calibrationData[20] = pLat1[0];

	calibrationData[21] = pLng1[3];
	calibrationData[22] = pLng1[2];
	calibrationData[23] = pLng1[1];
	calibrationData[24] = pLng1[0];

	calibrationData[25] = 0x00;
	calibrationData[26] = 0x00;

	calibrationData[27] = 0x00;//У��Ԥ��

	// checksum 
	CalculateCheckSum(calibrationData);
	return calibrationData;
}




QByteArray CDeviceConnection::GetTurnToData(double dYaw, double dPitch)
{
	//DreamSky 20210807 TurnTo
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0B;
	short sRate = (SHRT_MAX - SHRT_MIN) / 360;

	if (dYaw > YAW_MAX) dYaw = YAW_MAX;
	else if (dYaw < YAW_MIN) dYaw = YAW_MIN;
	short sYawAngle = dYaw * sRate;
	m_CommonCmd[m_iCommonBeginPos + 1] = 0xFF & (uchar)(sYawAngle >> 8);
	m_CommonCmd[m_iCommonBeginPos + 2] = 0xFF & (uchar)sYawAngle;

	if (dPitch > PITCH_MAX) dPitch = PITCH_MAX;
	else if (dPitch < PITCH_MIN) dPitch = PITCH_MIN;
#ifdef __REVERSE_PITCH__
	dPitch *= -1;
#endif
	short sPitchAngle = dPitch * sRate;
	m_CommonCmd[m_iCommonBeginPos + 3] = 0xFF & (uchar)(sPitchAngle >> 8);
	m_CommonCmd[m_iCommonBeginPos + 4] = 0xFF & (uchar)sPitchAngle;

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetStartTrackData()
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x06;

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x03;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetStopTrackData()
{
	//DreamSky 2021-11-1
	QByteArray trackData;
	trackData.resize(9);
	trackData[0] = 0x55;
	trackData[1] = 0xAA;
	trackData[2] = 0xDC;

	trackData[3] = 0x06;
	trackData[4] = 0x1E;

	trackData[5] = 0x00;

	trackData[6] = 0x01;
	trackData[7] = 0x00;

	trackData[8] = 0x19;
	return trackData;

	//// A1
	//m_CommonCmd[m_iCommonBeginPos] = 0x01;

	//// C1
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	//// E1
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x01;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x01;

	//// checksum
	//CalculateCheckSum(m_CommonCmd);
	//return m_CommonCmd;
}

QByteArray CDeviceConnection::GetTrackTemplateSizeData(int iSize)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x01;
	if (iSize == 0) m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x28;
	else if (iSize == 1) m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x21;
	else if (iSize == 2) m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x22;
	else if (iSize == 3) m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x23;
	else m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x28;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetImageColorData(ImageType emImgType, bool bEnablePIP, IRColor emIRColor)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	if (emImgType == IMT_Visible1)
	{
		if (bEnablePIP) sC1 = 0x03;
		else sC1 = 0x01;
	}
	else if (emImgType == IMT_Visible2)
	{
		if (bEnablePIP) sC1 = 0x03;
		else sC1 = 0x05;
	}
	else if (emImgType == IMT_IR1 || emImgType == IMT_IR2)
	{
		if (bEnablePIP) sC1 = 0x04;
		else sC1 = 0x02;
	}
	else if (emImgType == IMT_Fusion)
	{
		sC1 = 0x06;
	}

	if (emIRColor == IRC_WhiteHot) sC1 |= (0x0E << 6); // ����
	else if (emIRColor == IRC_BlackHot) sC1 |= (0x0F << 6);// ����
	else if (emIRColor == IRC_PseudoHot) sC1 |= (0x12 << 6); // ����һ���Զ����ʺ磩

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetImageColorDataV2(uchar cImgType)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	// sC1 = 0xFF & (m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0]);
	// sC1 = (sC1 << 8);
	// sC1 |= m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1];
	sC1 |= cImgType;

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetImageColorDataV3(uchar cIRColor)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	// sC1 = 0xFF & (m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0]);
	// sC1 = (sC1 << 8);
	// sC1 |= m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1];

	sC1 |= (cIRColor << 6);

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetPhotographData(bool clear)
{
	//// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	if (clear)
	{
		sC1 &= (~0x2 << 3);
	}
	else
	{
		sC1 |= (0x2 << 3);
	}

	if (clear)
	{
		sC1 &= (~0x13 << 6); // �������
	}
	else
	{
		sC1 |= (0x13 << 6); // ����
	}

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	//// E1
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetSwitchRecordData(bool bOn, bool clear)
{
	//// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	if (clear)
	{
		sC1 &= (~0x2 << 3);
	}
	else
	{
		sC1 |= (0x2 << 3);
	}

	if (bOn)
	{
		if (clear)
		{
			sC1 &= (~0x14 << 6); // �����ʼ¼��
		}
		else
		{
			sC1 |= (0x14 << 6); // ��ʼ¼��
		}
	}
	else
	{
		if (clear)
		{
			sC1 &= (~0x15 << 6); // ���ֹͣ¼��
		}
		else
		{
			sC1 |= (0x15 << 6); // ֹͣ¼��
		}
	}

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	//// E1
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	//m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetSwitchDefogData(bool bOn)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = bOn ? 0x17 : 0x16;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetSetRecordModeData(bool iMode)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	sC1 |= (0x2 << 3);

	/*
	if (iMode == 0) // ����ģʽ
	{
		sC1 |= (0x16 << 6);
	}
	else // ¼��ģʽ
	{
		sC1 |= (0x17 << 6);
	}
	*/

	sC1 |= (0x18 << 6);

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

int CDeviceConnection::GetRecordMode()
{
	return 0;
}

QByteArray CDeviceConnection::GetSetFocusModeData(bool iMode)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	sC1 |= (0x2 << 3);

	if (iMode == 0) // �Զ��۽�
	{
		sC1 |= (0x19 << 6);
	}
	else // �ֶ��۽�
	{
		sC1 |= (0x1a << 6);
	}
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetSwitchLaserData(bool bOn)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x74;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = bOn ? 0x01 : 0x02;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetLaserZoomInData(short uSpeed)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x74;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = 0x04;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetLaserZoomOutData(short uSpeed)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x74;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = 0x05;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetLaserStopZoomData()
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = 0x05;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetSetLaserZoomModeData(int iMode)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x74;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = iMode == 0 ? 0x06 : 0x07;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetZoomToData(float fMag)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = 0x53;
	short sMag = fMag * 10;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = 0xFF & (sMag >> 8);
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0xFF & sMag;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;
	
	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}


QByteArray CDeviceConnection::GetIRDigitalZoomInData(short uSpeed)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	sC1 |= (0x2 << 3);

	sC1 |= (0x1b << 6);

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetIRDigitalZoomOutData(short uSpeed)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x0F; // A1 �޶���

	// C1
	short sC1 = 0;
	sC1 |= (0x2 << 3);

	sC1 |= (0x1c << 6);

	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0xFF & (sC1 >> 8);
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0xFF & sC1;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetSwitchEODigitalZoomData(bool bOn)
{
	// A2
	m_UncommonCmd[m_iUncommonBeginPos] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + 1] = 0x00;

	// C2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 0] = bOn ? 0x06 : 0x07;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + 2] = 0x00;

	// E2
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 0] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 1] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 2] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 3] = 0x00;
	m_UncommonCmd[m_iUncommonBeginPos + A2_LENGTH + C2_LENGTH + 4] = 0x00;

	// checksum
	CalculateCheckSum(m_UncommonCmd);
	return m_UncommonCmd;
}

QByteArray CDeviceConnection::GetCurFirmwareVersioData()
{
	return m_CommonCmd;
}

QByteArray CDeviceConnection::GetEnterSBUSModeData()
{
	QByteArray frame;
	frame.resize(5);

	frame[0] = 0xaa;
	frame[1] = 0x55;
	frame[2] = 0x08;
	frame[3] = 0x07;
	frame[4] = 0xff;

	return frame;
}

QByteArray CDeviceConnection::GetExitSBUSModeData()
{
	QByteArray frame;
	frame.resize(5);

	frame[0] = 0xaa;
	frame[1] = 0x55;
	frame[2] = 0x08;
	frame[3] = 0x06;
	frame[4] = 0xff;

	return frame;
}


//OSD״̬��Ϣ��ѯ
QByteArray CDeviceConnection::GetQueryDevConfigurationData()
{
	QByteArray frame;
	frame.resize(6);

	frame[0] = 0x3e;
	frame[1] = 0x2a;
	frame[2] = 0x00;
	frame[3] = 0x2a;
	frame[4] = 0x00;
	frame[5] = 0x00;

#if 0
	PrintHexData(frame, "query dev configuration");
#endif // 1
	return frame;
}

//����ʱ��
QByteArray CDeviceConnection::GetTimeZoneData(char tz)
{  //DreamSky 20210812    
	QByteArray frame;
	frame.resize(5);

	frame[0] = 0xAA;
	frame[1] = 0x55;
	frame[2] = 0x04;
	frame[3] = tz;
	frame[4] = 0xFF;

#if 0
	PrintHexData(frame, "set osd");
#endif // 1

	return frame;
}


//����ʱ��
QByteArray CDeviceConnection::GetTimeZoneDataV2(char tz)
{     //DreamSky 20210812    
	  /*  55 AA DC 05 01 04 00   ��ʱ��
		55 AA DC 05 01 04 01     ��1
		55 AA DC 05 01 04 02     ��2
		55 AA DC 05 01 04 08 08  ������*/
	QByteArray frameTimeZone;
	frameTimeZone.resize(8);

	frameTimeZone[0] = 0x55;
	frameTimeZone[1] = 0xAA;
	frameTimeZone[2] = 0xDC;
	frameTimeZone[3] = 0x05;
	frameTimeZone[4] = 0x01;
	frameTimeZone[5] = 0x04;
	frameTimeZone[6] = tz;
	frameTimeZone[7] = 0x00;

	// checksum
	CalculateCheckSum(frameTimeZone);
	return frameTimeZone;
}


QByteArray CDeviceConnection::hexStrToByteArrayTest(const QString& str)
{
	QByteArray senddata;
	int hexdata, lowhexdata;
	int hexdatalen = 0;
	int len = str.length();
	senddata.resize(len / 2);
	char lstr, hstr;

	for (int i = 0; i < len;) {
		hstr = str.at(i).toLatin1();
		if (hstr == ' ') {
			i++;
			continue;
		}

		i++;
		if (i >= len) {
			break;
		}

		lstr = str.at(i).toLatin1();
		hexdata = convertHexCharsTest(hstr);
		lowhexdata = convertHexCharsTest(lstr);

		if ((hexdata == 16) || (lowhexdata == 16)) {
			break;
		}
		else {
			hexdata = hexdata * 16 + lowhexdata;
		}

		i++;
		senddata[hexdatalen] = (char)hexdata;
		hexdatalen++;
	}

	senddata.resize(hexdatalen);
	return senddata;
}

char CDeviceConnection::convertHexCharsTest(char ch)
{
	if ((ch >= '0') && (ch <= '9')) {
		return ch - 0x30;
	}
	else if ((ch >= 'A') && (ch <= 'F')) {
		return ch - 'A' + 10;
	}
	else if ((ch >= 'a') && (ch <= 'f')) {
		return ch - 'a' + 10;
	}
	else {
		return (-1);
	}
}



//DreamSky 2021-12-6  OSDV2����
QByteArray CDeviceConnection::GetSetOSDDataRight(uchar uParam1, uchar uParam2) {

		QByteArray frame;
		frame.resize(8);
	
		frame[0] = 0x55;
		frame[1] = 0xAA;
		frame[2] = 0xDC;
		frame[3] = 0x05;
		frame[4] = 0x01; //该字节的值应该是机芯类型，由设备端自动填充
		frame[5] = 0x07;
		frame[6] = uParam2;
		frame[7] = 0x00;
	CalculateCheckSum(frame);
	
	#if 0
		PrintHexData(frame, "set Osd_V2");
	#endif
	
		return frame;

}


//DreamSky 2022-0304-6  OSDV2
QByteArray CDeviceConnection::GetSetOSDDataLeft(uchar uParam1,uchar uParam2) {
    //55 AA DC 05 01 05 00
    QByteArray frame;
    frame.resize(8);


    frame[0] = 0x55;
    frame[1] = 0xAA;
    frame[2] = 0xDC;
    frame[3] = 0x05;
    frame[4] = 0x01; // 该字节的值应该是机芯类型，由设备端自动填充
    frame[5] = 0x05;
    frame[6] = uParam1;
    frame[7] = 0x00;
    CalculateCheckSum(frame);

#if 0
    PrintHexData(frame, "set Osd_V2");
#endif

    return frame;


}



QByteArray CDeviceConnection::GetSetOSDData(uchar uParam1, uchar uParam2)
{
	QByteArray frame;
	frame.resize(48);
	frame.fill(0x00);

	frame[0] = 0x7e;
	frame[1] = 0x7e;
	frame[2] = 0x44;
	frame[3] = uParam1;
	frame[4] = 0; // ���ֽڵ�ֵӦ���ǻ�о���ͣ����豸���Զ����
	frame[5] = 0x83;
	frame[6] = uParam2;

	CalculateCheckSumV2(frame);
#if 0
	PrintHexData(frame, "set osd");
#endif // 1

	return frame;
}



QByteArray CDeviceConnection::GetSetTemData(uchar uParam1, uchar uParam2)
{
	QByteArray frame;
	frame.resize(48);
	frame.fill(0x00);

	frame[0] = 0x7e;
	frame[1] = 0x7e;
	frame[2] = 0x44;
	frame[3] = uParam1;
	frame[4] = 0; // ���ֽڵ�ֵӦ���ǻ�о���ͣ����豸���Զ����
	frame[5] = 0x83;
	frame[6] = uParam2;

	CalculateCheckSumV2(frame);
#if 0
	PrintHexData(frame, "set osd");
#endif // 1

	return frame;
}

QByteArray CDeviceConnection::GetSetWirelessCtrlChnlMapData(const VLK_ChannelsMap& ChnlsMap)
{
	QByteArray frame;  
	frame.resize(11);
	frame.fill(0x00);


	frame[0] = 0xAA;// DreamSky 20210812 yuanben   
	frame[1] = 0x55;
	frame[2] = 0x11;

	frame[3] = ChnlsMap.uYW;
	frame[4] = ChnlsMap.uPT;
	frame[5] = ChnlsMap.uMO;
	frame[6] = ChnlsMap.uZM;
	frame[7] = ChnlsMap.uFC;
	frame[8] = ChnlsMap.uRP;
	frame[9] = ChnlsMap.uMU;
	frame[10] = 0xFF;

#if 0
	PrintHexData(frame, "########### Set wireless channel");
#endif
	return frame;
}

QByteArray CDeviceConnection::GetSetWirelessCtrlChnlInvertFlagDataPart1(const VLK_ChannelsInvertFlag& ChnlInvertFlag)
{
	QByteArray frame;
	frame.resize(5);
	frame.fill(0x00);

	frame[0] = 0xAA;
	frame[1] = 0x55;
	frame[2] = 0x1A;
	frame[3] = ChnlInvertFlag.uYPMZInvertFlag;
	frame[4] = 0xFF;

#if 0
	PrintHexData(frame, __FUNCTION__);
#endif
	return frame;
}

QByteArray CDeviceConnection::GetSetWirelessCtrlChnlInvertFlagDataPart2(const VLK_ChannelsInvertFlag& ChnlInvertFlag)
{
	QByteArray frame;
	frame.resize(5);
	frame.fill(0x00);

	frame[0] = 0xAA;
	frame[1] = 0x55;
	frame[2] = 0x1B;
	frame[3] = ChnlInvertFlag.uFPRMInvertFlag;
	frame[4] = 0xFF;

#if 0
	PrintHexData(frame, __FUNCTION__);
#endif
	return frame;
}

QByteArray CDeviceConnection::GetSetDevConfigurationData(const VLK_DevConfig& DeviceCfg)
{
	QByteArray frame;
	int iHeadSize = sizeof(VLK_HEAD_V3);
	int iDataSize = sizeof(DeviceCfg);
	frame.resize(sizeof(VLK_HEAD_V3) + sizeof(DeviceCfg) + 1);
	frame.fill(0x00);

	// ����֡ͷ
	for (int i = 0; i < (int)sizeof(VLK_HEAD_V3); ++i)
	{
		frame[i] = VLK_HEAD_V3[i];
	}

	// ������������
	const char* pSrc = (const char*)&DeviceCfg;
	char* pDst = frame.data() + sizeof(VLK_HEAD_V3);
	memcpy(pDst, pSrc, sizeof(DeviceCfg));

	// У��cheksum
	int iCheckSum = 0;
	for (int i = sizeof(VLK_HEAD_V3); i < frame.size() - 1; ++i)
	{
		iCheckSum += frame.at(i);
	}
	frame[frame.size() - 1] = (uchar)(0xFF & iCheckSum);

#if 0
	PrintHexData(frame, "Set device cfg");
#endif

	return frame;
}

QByteArray CDeviceConnection::GetSetGimbalAimPointData(double dLat, double dLng, double dAlt)
{
	// A1
	m_CommonCmd[m_iCommonBeginPos] = 0x08;
	float fLat = (float)dLat;
	uchar* pLat = (uchar*)&fLat;
	m_CommonCmd[m_iCommonBeginPos + 1] = pLat[3];
	m_CommonCmd[m_iCommonBeginPos + 2] = pLat[2];
	m_CommonCmd[m_iCommonBeginPos + 3] = pLat[1];
	m_CommonCmd[m_iCommonBeginPos + 4] = pLat[0];

	float fLng = (float)dLng;
	uchar* pLng = (uchar*)&fLng;
	m_CommonCmd[m_iCommonBeginPos + 5] = pLng[3];
	m_CommonCmd[m_iCommonBeginPos + 6] = pLng[2];
	m_CommonCmd[m_iCommonBeginPos + 7] = pLng[1];
	m_CommonCmd[m_iCommonBeginPos + 8] = pLng[0];


	// C1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + 1] = 0x00;

	// E1
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 0] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 1] = 0x00;
	m_CommonCmd[m_iCommonBeginPos + A1_LENGTH + C1_LENGTH + 2] = 0x00;

	// checksum 
	CalculateCheckSum(m_CommonCmd);
	return m_CommonCmd;
}



int CDeviceConnection::GetFocusMode()
{
	return m_iFocusMode;
}

void CDeviceConnection::GetDataByFrameID(uchar frameid, QByteArray& data, int& begin, int& end)
{
	data.resize(sizeof(VLK_HEAD) + GetDataSizeByFrameID(frameid) + 3);
	data.fill(0x00);

	// ֡ͷ
	int iIndex = 0;
	for (; iIndex < (int)sizeof(VLK_HEAD); ++iIndex)
	{
		data[iIndex] = VLK_HEAD[iIndex];
	}

	// body length
	data[iIndex] = GetDataSizeByFrameID(frameid) + 3;

	// frame id
	data[iIndex + 1] = frameid;

	// �������ݿ�ʼ�ͽ�������
	begin = iIndex + 2;
	data[begin] = frameid == 0x30 ? 0x0F : 0x00; // ����Э���ʼ���� 0x0F
	end = iIndex + GetDataSizeByFrameID(frameid) + 2;
}

void CDeviceConnection::DoReadBuffer()
{
	m_lockBuffer.lock();
	if (m_Buffer.isEmpty())
	{
		m_lockBuffer.unlock();
		return;
	}
	//PrintHexData(m_Buffer, "m_Buffer yuan:::");

#if 0
	//PrintHexData(m_Buffer, "@@@@@@@@@@@ m_Buffer");
#endif

	// ����V1֡
	int iIndex = m_Buffer.indexOf(s_VLKHead);
	while (iIndex >= 0)
	{
		// qDebug() << "@@@@@@@ iIndex: " << iIndex;
		int frame_len = ((uchar)m_Buffer[iIndex + (int)sizeof(VLK_HEAD)]) & 0x3F;

		if (iIndex + (int)sizeof(VLK_HEAD) + frame_len > m_Buffer.size()) // ���ֻ�û�н������֡
		{
			QByteArray frame = m_Buffer.mid(iIndex, m_Buffer.size() - iIndex);
#if 0
			PrintHexData(frame, "incomplete frame");
#endif
			
// 			qCritical("iIndex = %d, frame_len = %d, m_Buffer.size() = %d, iIndex + (int)sizeof(VLK_HEAD) + frame_len = %d, waiting receive rest part of this frame",
// 				iIndex, frame_len, m_Buffer.size(), iIndex + (int)sizeof(VLK_HEAD) + frame_len);
			break;
		}

		QByteArray frame = m_Buffer.mid(iIndex, sizeof(VLK_HEAD) + frame_len);
#if 0
		PrintHexData(frame, "@@@@@@@ complete frame");
#endif // 1
		ProcReceivedFrame(frame);
		m_Buffer.remove(iIndex, sizeof(VLK_HEAD) + frame_len);
		iIndex = m_Buffer.indexOf(s_VLKHead);
	
	}

	// ����V2֡
	int iIndexV2 = m_Buffer.indexOf(s_VLKHeadV2);

	//qDebug() << "------->iIndexV2---------: " << iIndexV2;
	while (iIndexV2 >= 0)
	{
		// qDebug() << "@@@@@@@ iIndexV2: " << iIndexV2;
		int frame_lenV2 = VLK_HEAD_V2[2];
		// +1 ����Ϊ�����һ���ֽڵ�У����
		if (iIndexV2 + (int)sizeof(VLK_HEAD_V2) + frame_lenV2 + 1 > m_Buffer.size()) // ���ֻ�û�н������֡
		{
			QByteArray frame = m_Buffer.mid(iIndexV2, m_Buffer.size() - iIndexV2);
#if 0
			PrintHexData(frame, "incomplete frame");
#endif
			qCritical("iIndex = %d, frame_lenV2 = %d, m_Buffer.size() = %d, iIndexV2 + (int)sizeof(VLK_HEAD_V2) + frame_lenV2 + 1 = %d, waiting receive rest part of this frame V2",
				iIndexV2, frame_lenV2, m_Buffer.size(), iIndexV2 + (int)sizeof(VLK_HEAD_V2) + frame_lenV2 + 1);
			qDebug() << "waiting receive rest part of this frame V2 ";
			break; 
		}

		QByteArray frame = m_Buffer.mid(iIndexV2, sizeof(VLK_HEAD_V2) + frame_lenV2 + 1);
		
#if 0
		PrintHexData(frame, "@@@@@@@ complete frame");
#endif 
		//qDebug() << "-------frame-------:" << frame;
		ProcReceivedFrameV2(frame);
		m_Buffer.remove(iIndexV2, sizeof(VLK_HEAD_V2) + frame_lenV2 + 1);
		iIndexV2 = m_Buffer.indexOf(s_VLKHeadV2);
	}
	

	
	if (iIndex < 0 && iIndexV2 < 0)
	{  
		m_Buffer.clear();  
	}
	else
	{
		m_Buffer.remove(0, max(iIndex, iIndexV2));

		
	}

	m_lockBuffer.unlock();
}
                       
bool CDeviceConnection::ProcReceivedFrameV2(const QByteArray& frame)
{
	// У��֡ͷ
	int iIndex = 0;
	for (; iIndex < (int)sizeof(VLK_HEAD_V2); ++iIndex)
	{
		if ((uchar)frame[iIndex] != VLK_HEAD_V2[iIndex])
		{
			qCritical() << "bad frame head!!!!!!!!!!!!!!!!";
			return false;
		}
	}

	// У�����ݴ�С
	if (frame.size() != (int)(sizeof(VLK_HEAD_V2) + VLK_HEAD_V2[2] + 1))
	{
		qCritical("bad frame size, it should be  %d (sizeof(VLK_HEAD_V2)+VLK_HEAD_V2[2])  but we got frame size = %d",
			(int)(sizeof(VLK_HEAD_V2) + VLK_HEAD_V2[2]), frame.size());
		return false;
	}

	// У��cheksum
	int iCheckSum = 0;
	for (int i = sizeof(VLK_HEAD_V2); i < frame.size() - 1; ++i)
	{
		iCheckSum += frame.at(i);
	}
	uchar uCheckSum = (uchar)(0xFF & iCheckSum);
	if (uCheckSum != (uchar)frame[frame.size() - 1])
	{
		qCritical("bad frame checksum, it should be %02x, but we got frame[%d] = %02x",
			uCheckSum, frame.size() - 1, (uchar)frame[frame.size() - 1]);
		qDebug() << "11111!!!!!!!!!!!!!!!!";
		return false;
	}

	int iFrameLen = VLK_HEAD_V2[2];
	const char* szSrc = frame.data() + sizeof(VLK_HEAD_V2);
	memcpy(&m_DevConfig, szSrc, iFrameLen < (int)sizeof(m_DevConfig) ? iFrameLen : sizeof(m_DevConfig));

#if 0
	PrintHexData(frame, "Device config");
	QByteArray temp = frame.mid(17, 7);
	PrintHexData(temp, "########### Get wireless channel");

	temp = frame.mid(26, 2);
	PrintHexData(temp, "########### Get wireless channel invert flag");
#endif
	qDebug() << "EVT_QUERY_DEVICE_CONFIG_RESULT";
	CQueryDeviceCfgResultEventArg* pArg = new CQueryDeviceCfgResultEventArg();
	pArg->iErrorCode = 0;
	pArg->DeviceCfg = m_DevConfig;
	m_pContext->FireEvent(EVT_QUERY_DEVICE_CONFIG_RESULT, pArg, false, false);//z
	pArg->Release();

#ifdef __DUMP_RECEIVE_DATA__
	uchar uTimeZone = (uchar)frame[(int)sizeof(VLK_HEAD_V2)];
	uchar uOSDCfg = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 1];
	uchar uMagneticVariation = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 2];
	uchar uOSDInput = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 3];
	uchar uBaudRate = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 4];
	uchar uEODigitalZoom = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 5];

	short sTemperatureAlarmLine = ((short)frame[(int)sizeof(VLK_HEAD_V2) + 6]) << 8;
	sTemperatureAlarmLine |= frame[(int)sizeof(VLK_HEAD_V2) + 7];

	uchar uTrack = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 8];
	uchar uLaser = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 9];
	uchar uRecordDefinition = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 10];
	uchar uOSDGPS = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 11];
	uchar uSBUSChnlMap = (uchar)frame[(int)sizeof(VLK_HEAD_V2) + 12];
#endif // __DUMP_RECEIVE_DATA__
	return true;
}

bool CDeviceConnection::ProcReceivedFrame(const QByteArray& frame)
{
	
	// У��֡ͷ
	int iIndex = 0;
	for (; iIndex < (int)sizeof(VLK_HEAD); ++iIndex)
	{

		if ((uchar)frame[iIndex] != VLK_HEAD[iIndex])
		{
			qCritical() << "bad frame head!!!!!!!!!!!!!!!!";
			return false;
		}
	}
	
	// У�����ݴ�С
	if (frame.size() != (int)((frame[iIndex] & 0x3F) + sizeof(VLK_HEAD)))
	{
		
		qCritical("bad frame size, it should be  %d (frame[%d]&0x3F)  but we got frame size = %d",
			(int)((frame[iIndex] & 0x3F) + sizeof(VLK_HEAD)), iIndex, frame.size());
		return false;
	}

	// У��cheksum
	uchar uCheckSum = frame[(int)sizeof(VLK_HEAD)];
	for (int i = sizeof(VLK_HEAD) + 1; i < frame.size() - 1; ++i)
	{
		
		uCheckSum = uCheckSum ^ frame[i];
	}
	if (uCheckSum != (uchar)frame[frame.size() - 1])
	{
		
		qCritical("bad frame CheckSum, it should be 0x%02x, but frame[%d] = 0x%02x",
			uCheckSum, frame.size() - 1, (uchar)frame[frame.size() - 1]);

#if 0
		qDebug() << "================begin frame==================";
		PrintHexData(frame, "frame");
		qDebug() << "================end frame==================";
#endif
	
		return false;
	}


	if ((uchar)frame[iIndex + 1] == 0x00) // ���ְ��Ļظ�
	{
		// qDebug() << "response of say hello";
		ProcSayHelloResponse(frame);
	}
	else if ((uchar)frame[iIndex + 1] == (0x10 | m_ucDeviceType)) // �������Ļظ�
	{
		// qDebug() << "response of keep alive";
		ProcKeepAliveResponse(frame);
	}
	else if ((uchar)frame[iIndex + 1] == 0x40)
	{
		// qDebug() << "response 0x40 frame";
		ProcFrameData_0x40(frame);
	}
	else if ((uchar)frame[iIndex + 1] == 0x41)
	{
		// qDebug() << "response  0x41 frame";
		ProcFrameData_0x41(frame);
	}
	else if ((uchar)frame[iIndex + 1] == 0x02)
	{
		// qDebug() << "response  0x02 frame";
		ProcFrameData_0x02(frame);
	}

	return false;
}

bool CDeviceConnection::ProcSayHelloResponse(const QByteArray& frame)
{
	
	if ((int)sizeof(VLK_HEAD) + 2 >= frame.size()) // ��ֹԽ��
	{
		qCritical() << "bad input parameters, sizeof(VLK_HEAD) + 2 >= frame.size()";
		return false;
	}
#if 0
	PrintHexData(frame, "say hello response");
#endif 
	uchar uDeviceType = frame[(int)sizeof(VLK_HEAD) + 2];
	ILocalOptionsService* pLocalOptionService = (ILocalOptionsService*)m_pContext->QueryService(VLK_SERVICE_LOCAL_OPTIONS);
	if (NULL == pLocalOptionService)
	{
		qCritical("query service VLK_SERVICE_LOCAL_OPTIONS return NULL");
		return false;
	}
	QString strProductModel = pLocalOptionService->GetModelNameByID((int)uDeviceType);
	if (strProductModel.isEmpty())
	{
		qCritical() << "unsupported device type :" << uDeviceType;
		qCritical() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!you could not control current connected device!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

		CDeviceModelChangedEventArg* pArg = new CDeviceModelChangedEventArg();
		pArg->strDeviceModelName = strProductModel;
		

		m_pContext->FireEvent(EVT_DEVICE_MODEL_CHANGED, pArg, false, false);
		pArg->Release();

		m_bIsValidDevice = false;
		return false;
	}

	qDebug() << "current connected device type: " << strProductModel;
	qDebug() << "hand shake success! enjoy !!!";
	m_bIsValidDevice = true;
	m_ucDeviceType = uDeviceType;
	m_tmPreActiveTime = QTime::currentTime();

	CDeviceModelChangedEventArg* pArg = new CDeviceModelChangedEventArg();
	pArg->strDeviceModelName = strProductModel;

	m_pContext->FireEvent(EVT_DEVICE_MODEL_CHANGED, pArg, false, false);
	pArg->Release();


	return true;
}

bool CDeviceConnection::ProcKeepAliveResponse(const QByteArray& frame)
{
	if ((int)sizeof(VLK_HEAD) + 2 >= frame.size()) // ��ֹԽ��
	{
		qCritical() << "bad input parameters";
		// m_bIsValidDevice = false;
		return false;
	}

#if 0
	PrintHexData(m_Buffer, "keepalive response");
#endif

	// У���豸����
	if (frame[(int)sizeof(VLK_HEAD) + 1] != (0x10 | m_ucDeviceType))
	{
		qCritical("bad device type, frame[%d] should be mask of %0x(0x10 | %0x), but we got %0x",
			(int)sizeof(VLK_HEAD) + 1, (uchar)(0x10 | m_ucDeviceType), m_ucDeviceType, (uchar)frame[(int)sizeof(VLK_HEAD) + 1]);
		// m_bIsValidDevice = false;
		return false;
	}

	// ����ѭ��֡�ţ��ҵ���Ӧ��s_CrcTabԪ�ص�index
	uchar ucFrameNo = frame[(int)sizeof(VLK_HEAD)] & 0xC0;
	uchar ucCrcTabElementIndex = 0;
	for (int i = 0; i < LOOP_FRAME_NO_COUNT; ++i)
	{
		uchar highbyte = (m_sCrcTabElementIndex[i] >> 8) & 0xFF;
		if (highbyte == ucFrameNo)
		{
			ucCrcTabElementIndex = ((uchar)m_sCrcTabElementIndex[i] & 0xFF);
			break;
		}
		// qDebug("##### %02x:%02x", highbyte, m_sCrcTabElementIndex[i] & 0xFF);
	}
	// qDebug("##### receive ucFrameNo=%02x, ucCrcTabElementIndex = %02x, s_CrcTab[%02x] = %02x, frame[2] = %02x", 
	//	ucFrameNo, ucCrcTabElementIndex, ucCrcTabElementIndex, s_CrcTab[ucCrcTabElementIndex], (uchar)frame[(int)sizeof(VLK_HEAD) + 2]);

	// ����ucCrcTabElementIndexȥ��������s_CrcTab�в��Ҷ�Ӧ��ֵ���������豸������ֵ֪��һֱ
	if (ucCrcTabElementIndex < 0 || ucCrcTabElementIndex >= sizeof(s_CrcTab))
	{
		qCritical("ucCrcTabElementIndex out of range, should be in [0, %d), but ucCrcTabElementIndex = %d",
			sizeof(s_CrcTab), ucCrcTabElementIndex);
		// m_bIsValidDevice = false;
		return false;
	}
	if ((uchar)s_CrcTab[ucCrcTabElementIndex] != (uchar)frame[(int)sizeof(VLK_HEAD) + 2])
	{
		qCritical("!!!!!!!!!!!!! magic number mismatch s_CrcTab[%02x] = %02x, but frame[%d] = %02x",
			ucCrcTabElementIndex, (uchar)s_CrcTab[ucCrcTabElementIndex],
			(int)sizeof(VLK_HEAD) + 2, (uchar)frame[(int)sizeof(VLK_HEAD) + 2]);
		// m_bIsValidDevice = false;
		return false;
	}

	// �����յ�������ʱ��
	m_tmPreActiveTime = QTime::currentTime();
	m_bIsValidDevice = true;
	return true;
}

bool CDeviceConnection::ProcFrameData_0x40(const QByteArray& frame)
{
	m_Latest0x40Frame = frame;
	double dDroneLatitude = 0.0;
	double dDroneLongitude = 0.0;
	short iDroneAltitude = 0;

	double dTargetLatitude = 0.0;
	double dTargetLongitude = 0.0;
	short iTargetAltitude = 0;
	// T1
	{
		int iT1Begin = (int)sizeof(VLK_HEAD) + 2;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse T1 ===============";
#endif
		// Ŀ�������Դ����bit 0~2
		uchar cTargetDistanceSrc = frame[iT1Begin] & 0x7;

		// GPS �źŲ���׶�bit 3~4
		uchar cGPSCatchPeriod = (frame[iT1Begin] >> 3) & 0x03;

		// GPS ˮƽ�ź�����bit 5~7
		uchar uGPSHorizonSignalQuality = (frame[iT1Begin] >> 5) & 0x07;

		// GPS �߶��ź�����bit 0~2
		uchar uGPSAltitudeSignalQuality = frame[iT1Begin + 1] & 0x07;

		// S2 ��ָ����Ӧbit 3
		uchar uS2CmdResponse = (frame[iT1Begin + 1] >> 3) & 0x01;

		// N ��ָ����Ӧbit 4~7
		uchar uNCmdResponse = (frame[iT1Begin + 1] >> 4) & 0x0F;

		// �ػ���γ�ȸ߶�����
		int iDroneLatitude = ((int)(uchar)frame[iT1Begin + 2]) << 24;
		iDroneLatitude |= ((int)(uchar)frame[iT1Begin + 3]) << 16;
		iDroneLatitude |= ((int)(uchar)frame[iT1Begin + 4]) << 8;
		iDroneLatitude |= (uchar)frame[iT1Begin + 5];
		dDroneLatitude = (double)iDroneLatitude / 10000000;

		int iDroneLongitude = ((int)(uchar)frame[iT1Begin + 6]) << 24;
		iDroneLongitude |= ((int)(uchar)frame[iT1Begin + 7]) << 16;
		iDroneLongitude |= ((int)(uchar)frame[iT1Begin + 8]) << 8;
		iDroneLongitude |= (uchar)frame[iT1Begin + 9];
		dDroneLongitude = (double)iDroneLongitude / 10000000;

		iDroneAltitude = ((short)(uchar)frame[iT1Begin + 10]) << 8;
		iDroneAltitude |= (uchar)frame[iT1Begin + 11];

		// Ŀ�꾭γ�ȸ߶�����
		int iTargetLatitude = ((int)(uchar)frame[iT1Begin + 12]) << 24;
		iTargetLatitude |= ((int)(uchar)frame[iT1Begin + 13]) << 16;
		iTargetLatitude |= ((int)(uchar)frame[iT1Begin + 14]) << 8;
		iTargetLatitude |= (uchar)frame[iT1Begin + 15];
		dTargetLatitude = (double)iTargetLatitude / 10000000;

		int iTargetLongitude = ((int)(uchar)frame[iT1Begin + 16]) << 24;
		iTargetLongitude |= ((int)(uchar)frame[iT1Begin + 17]) << 16;
		iTargetLongitude |= ((int)(uchar)frame[iT1Begin + 18]) << 8;
		iTargetLongitude |= (uchar)frame[iT1Begin + 19];
		dTargetLongitude = (double)iTargetLongitude / 10000000;

		iTargetAltitude = ((short)(uchar)frame[iT1Begin + 20]) << 8;
		iTargetAltitude |= (uchar)frame[iT1Begin + 21];


#ifdef	__DUMP_RECEIVE_DATA__
		if (cTargetDistanceSrc == 0x00) qDebug() << "T1 no target";
		else if (cTargetDistanceSrc == 0x01) qDebug() << "T1 laser range finder";
		else if (cTargetDistanceSrc == 0x02) qDebug() << "T1 altitude estimated value";
		else if (cTargetDistanceSrc == 0x03) qDebug() << "T1 RF";
		else qDebug() << "T1 unknown source type: " << (int)cTargetDistanceSrc;

		if (cGPSCatchPeriod == 0x00) qDebug() << "T1 no signal";
		else if (cGPSCatchPeriod == 0x01) qDebug() << "T1 time locked";
		else if (cGPSCatchPeriod == 0x02) qDebug() << "T1 2D locked";
		else if (cGPSCatchPeriod == 0x03) qDebug() << "T1 3D locked";
		else qDebug() << "T1 unknown GPS catch period: " << (int)cGPSCatchPeriod;

		qDebug() << "T1 GPS horizon signal quality: " << (int)uGPSHorizonSignalQuality;

		qDebug() << "T1 GPS altitude signal quality: " << (int)uGPSAltitudeSignalQuality;

		qDebug() << "T1 S2 cmd response: " << (int)uS2CmdResponse;

		if (uNCmdResponse == 1) qDebug() << "T1 gyro offset auto adjusting";
		else if (uNCmdResponse == 2) qDebug() << "T1 gyro offset saving";
		else if (uNCmdResponse == 3) qDebug() << "T1 Gyro offset recovered to factory default value";
		else if (uNCmdResponse == 4) qDebug() << "T1 0 angle position of AHRS adjusted";
		else if (uNCmdResponse == 5) qDebug() << "T1 AHRS attitude offset Saving";
		else if (uNCmdResponse == 6) qDebug() << "T1 AHRS attitude offset reset";
		else if (uNCmdResponse == 7) qDebug() << "T1 Calibrating gyro temperature drift";
		else if (uNCmdResponse == 8) qDebug() << "T1 Gyro temperature drift calibrated over";

		qDebug("T1 camera latitude: %.02f, longitude: %.02f, altitude: %d", dCameraLatitude, dCameraLongitude, iCameraAltitude);

		qDebug("T1 target latitude: %.02f, longitude: %.02f, altitude: %d", dTargetLatitude, dTargetLongitude, iTargetAltitude);

		qDebug() << "=============== end parse T1 ===============";
#endif
	}

	uchar cTrackerStatus = 0;
	QString strTrackStatus;
	// F1
	{
		int iF1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse F1 ===============";
#endif

		// ���ڸ��ٵĴ�����bit 0~2
		uchar cTrackSensor = frame[iF1Begin] & 0x7;

		// �������ĵ�ǰ״̬bit 3~4
		cTrackerStatus = (frame[iF1Begin] >> 3) & 0x03;
		
#ifdef	__DUMP_RECEIVE_DATA__
		if (cTrackSensor = 0x00) qDebug() << "F1 track sensor: visible light 1";
		else if (cTrackSensor == 0x1) qDebug() << "F1 track sensor: IR";
		else if (cTrackSensor == 0x2) qDebug() << "F1 track sensor: visible light 2";
		else qDebug() << "F1 track sensor: unknown " << (int)cTrackSensor;


		if (cTrackerStatus == 0x00) strTrackStatus = "stopped";
		else if (cTrackerStatus == 0x01) strTrackStatus = "searching";
		else if (cTrackerStatus == 0x02) strTrackStatus = "tracking";
		else if (cTrackerStatus == 0x03) strTrackStatus = "lost";
		else strTrackStatus = "unknown";
		qDebug() << "F1 tracker status: " << strTrackStatus;

		qDebug() << "=============== end parse F1 ===============";
#endif
	}

	double dYaw = 0.0;
	double dPitch = 0.0;
	// B1
	{
		int iB1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH + F1_LENGTH;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse B1 ===============";
#endif
		uchar cGimbalStatus = (frame[iB1Begin] >> 4) & 0x0F;

		// �ڶ����ֽڱ���
		// frame[iB1Begin +1]

		// ��λ�Ƕ�
		double dRate = (SHRT_MAX - SHRT_MIN) / 360.0;
		short sYaw = ((short)(uchar)frame[iB1Begin + 2]) << 8;
		sYaw |= (uchar)frame[iB1Begin + 3];

		// �����Ƕ�
		short sPitch = ((short)(uchar)frame[iB1Begin + 4]) << 8;
		sPitch |= (uchar)frame[iB1Begin + 5];

		dYaw = (double)sYaw / dRate;
		dPitch = (double)sPitch / dRate;
#ifdef __REVERSE_PITCH__
		dPitch *= -1;
#endif


#ifdef	__DUMP_RECEIVE_DATA__
		if (cGimbalStatus == 0x00) qDebug() << "B1 gimbal status: shut down";
		else if (cGimbalStatus == 0x01) qDebug() << "B1 gimbal status: manual operation";
		else if (cGimbalStatus == 0x02) qDebug() << "B1 gimbal status: favorite";
		else if (cGimbalStatus == 0x03) qDebug() << "B1 gimbal status: follow";
		else if (cGimbalStatus == 0x04) qDebug() << "B1 gimbal status: lock";
		else if (cGimbalStatus == 0x05) qDebug() << "B1 gimbal status: yaw scan";
		else if (cGimbalStatus == 0x06) qDebug() << "B1 gimbal status: track";
		else if (cGimbalStatus == 0x07) qDebug() << "B1 gimbal status: pitch scan";
		else if (cGimbalStatus == 0x08) qDebug() << "B1 gimbal status: fixed point follow";
		else if (cGimbalStatus == 0x09) qDebug() << "B1 gimbal status: manual operation(sky to ground)";
		else if (cGimbalStatus == 0x0A) qDebug() << "B1 gimbal status: follow current geographic position";
		else if (cGimbalStatus == 0x0B) qDebug() << "B1 gimbal status: follow space angle";
		else if (cGimbalStatus == 0x0F) qDebug() << "B1 gimbal status: shut down because of malfunction";
		else qDebug() << "B1 gimbal status: unknown";

		qDebug("B1 gimbal yaw: %f, pitch: %f", dYaw, dPitch);

		qDebug() << "=============== end parse B1 ===============";
#endif
	}

	uchar cOpticalSensor = 0x00;
	QString strOpticalSensor;
	double dEOOpticalZoom = 0.0;
	int iEODigitalZoom = 0;
	int iIRDigitalZoom = 0;
	ushort sLaserResult1 = 0;
	uchar uIR = 0;
	uchar uIRExtent = 0;
	uchar uRecordStatus = 0;
	// D1
	{
		int iD1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH + F1_LENGTH + B1_LENGTH;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse D1 ===============";
#endif
		// ��ѧ������bit 0~2
		cOpticalSensor = frame[iD1Begin] & 0x7;

		// �ȳ�����ӷŴ���bit 3~4
		uchar cIRDigitalZoom = (frame[iD1Begin] >> 3) & 0x03;
		iIRDigitalZoom = cIRDigitalZoom + 1;

		// �ɼ�����ӷŴ���bit 5~6
	/*	uchar cEODigitalZoom = (frame[iD1Begin] >> 5) & 0x03;
		iEODigitalZoom = cEODigitalZoom + 1;*/

		//�ɼ�����ӷŴ���  DreamSky  2021-12-15
		ushort cEODigitalZoom = ((ushort(frame[iD1Begin + 2])) << 8) | (0xff & (ushort(frame[iD1Begin + 3])));
		ushort cEODigitalZooms =  cEODigitalZoom >> 6;
		iEODigitalZoom = (double)cEODigitalZooms + 1;

		// ����״̬
		uIR = (uchar)(frame[iD1Begin] >> 7) & 0x01;

		// ¼��״̬ bit 0~1
		ushort sByte2_3 = ((ushort)frame[iD1Begin + 2]) << 8;
		sByte2_3 |= (uchar)frame[iD1Begin + 3];
		uRecordStatus = sByte2_3 & 0x3;

		// ����״̬��չ
		uIRExtent = (uchar)((sByte2_3 >> 2) & 0x0F);

		// �״��෵��ֵ
		sLaserResult1 = ((ushort)frame[iD1Begin + 4]) << 8;
		sLaserResult1 |= (uchar)frame[iD1Begin + 5];

		ushort sLaserResult2 = ((ushort)frame[iD1Begin + 6]) << 8;
		sLaserResult2 |= (uchar)frame[iD1Begin + 7];

		ushort sLaserResult3 = ((ushort)frame[iD1Begin + 8]) << 8;
		sLaserResult3 |= (uchar)frame[iD1Begin + 9];


		ushort sOpticalZoom = ((ushort(frame[iD1Begin + 10])) << 8) | (0xff & (ushort(frame[iD1Begin + 11])));
		dEOOpticalZoom = (double)sOpticalZoom * 0.1;
		//qDebug() << "-------------:" << dEOOpticalZoom;
		

#ifdef	__DUMP_RECEIVE_DATA__
		if (cOpticalSensor == 0x00) strOpticalSensor = "Visible 1";
		else if (cOpticalSensor == 0x01) strOpticalSensor = "IR";
		else if (cOpticalSensor == 0x02) strOpticalSensor = "Visible 1 + IR PIP";
		else if (cOpticalSensor == 0x03) strOpticalSensor = "IR + Visible 1 PIP";
		else if (cOpticalSensor == 0x04) strOpticalSensor = "Visible 2";
		qDebug() << "D1 optical sensor: " << strOpticalSensor;

		if (cIRDigitalZoom == 0x00) qDebug() << "D1 magnification times: no interpolating";
		else if (cIRDigitalZoom == 0x01) qDebug() << "D1 magnification times: interpolating 1 time";
		else qDebug() << "D1 magnification times: unknown";

		if (cEODigitalZoom == 0x00) qDebug() << "D1 Elec Amplify: X1";
		else if (cEODigitalZoom == 0x01) qDebug() << "D1 Elec Amplify: X2";
		else qDebug() << "D1 Elec Amplify: unknown";

		// ���Ⱥ���״̬bit 7
		if ((frame[iD1Begin] >> 7) & 0x01) qDebug() << "D1 black hot";
		else qDebug() << "D1 white hot";

		// �����������bit 0
		if ((frame[iD1Begin + 1] >> 0) & 0x01) qDebug() << "D1 laser counter: 1";
		else qDebug() << "D1 laser counter: 0";

		// ��������ʱbit 2~7
		qDebug() << "D1 laser delay: " << (int)((frame[iD1Begin + 1] >> 2) & 0x3F) << "ms";

		// 
		if (uRecordStatus == 0x00) qDebug() << "D1 record stopped";
		else if (uRecordStatus == 0x01) qDebug() << "D1 recording";
		else if (uRecordStatus == 0x10) qDebug() << "D1 take photo mode";
		else qDebug() << "D1 records status: unknown " << uRecordStatus;

		qDebug("D1 laser return result1: %d m, result2: %d m, result3: %d m",
			sLaserResult1, sLaserResult2, sLaserResult3);

		qDebug() << "D1 current sensor magnified times: " << sOpticalZoom;
		qDebug() << "=============== end parse D1 ===============";
#endif
	}

	// ��״̬���µ�����
	CTelemetryDataEventArg* pArg = new CTelemetryDataEventArg;
	pArg->telemetrydata.dYaw = dYaw;
	pArg->telemetrydata.dPitch = dPitch;
	pArg->telemetrydata.uSensorType = cOpticalSensor;
	pArg->telemetrydata.strSensorType = strOpticalSensor;
	pArg->telemetrydata.uTrackerStatus = cTrackerStatus;
	pArg->telemetrydata.strTrackStatus = strTrackStatus;

	pArg->telemetrydata.dDroneLatitude = dDroneLatitude;
	pArg->telemetrydata.dDroneLongitude = dDroneLongitude;
	pArg->telemetrydata.dDroneAltitude = (double)iDroneAltitude;

	pArg->telemetrydata.dTargetLatitude = dTargetLatitude;
	pArg->telemetrydata.dTargetLongitude = dTargetLongitude;
	pArg->telemetrydata.dTargetAltitude = (double)iTargetAltitude;

	pArg->telemetrydata.dEOOpticalZoom = dEOOpticalZoom;
	pArg->telemetrydata.iEODigitalZoom = iEODigitalZoom;
	pArg->telemetrydata.iIRDigitalZoom = iIRDigitalZoom;
	pArg->telemetrydata.sLaserDistance = sLaserResult1;
	pArg->telemetrydata.uIR = uIR;
	pArg->telemetrydata.uIRExtent = uIRExtent;
	pArg->telemetrydata.uRecordStatus = uRecordStatus;
	m_pContext->FireEvent(EVT_DEVICE_TELEMETRY_DATA, pArg, false, false);
	pArg->Release();

	return false;
}

bool CDeviceConnection::ProcFrameData_0x41(const QByteArray& frame)
{
	m_Latest0x41Frame = frame;

	double dAngleRate = (SHRT_MAX - SHRT_MIN) / 360.0;
	// T2
	{
		int iT2Begin = (int)sizeof(VLK_HEAD) + 2;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse T2 ===============";
#endif
		// ��һ���ֽڱ���
		// frame[iT2Begin]

		// ���˻�������ʱ��
		short sUAVDate = ((short)(uchar)frame[iT2Begin + 1]) << 8;
		sUAVDate |= (uchar)frame[iT2Begin + 2];
		uchar cYear = sUAVDate & 0x7F;
		uchar cMonth = (sUAVDate >> 7) & 0x0F;
		uchar cDay = (sUAVDate >> 11) & 0x1F;

		int iUAVTime = ((int)(uchar)frame[iT2Begin + 3]) << 16;
		iUAVTime |= ((int)(uchar)frame[iT2Begin + 4]) << 8;
		iUAVTime |= (uchar)frame[iT2Begin + 5];



		// GPS ����
		short sUAVGPSDirection = ((short)(uchar)frame[iT2Begin + 6]) << 8;
		sUAVGPSDirection |= (uchar)frame[iT2Begin + 7];

		short sUAVYaw = ((short)(uchar)frame[iT2Begin + 8]) << 8;
		sUAVYaw |= (uchar)frame[iT2Begin + 9];

		short sUAVPitch = ((short)(uchar)frame[iT2Begin + 10]) << 8;
		sUAVPitch |= (uchar)frame[iT2Begin + 11];

		short sUAVRoll = ((short)(uchar)frame[iT2Begin + 12]) << 8;
		sUAVRoll |= (uchar)frame[iT2Begin + 13];

		double dUAVGPSDirection = (double)sUAVGPSDirection / dAngleRate;
		double dUAVYaw = (double)sUAVYaw / dAngleRate;
		double dUAVPitch = (double)sUAVPitch / dAngleRate;
		double dUAVRoll = (double)sUAVRoll / dAngleRate;

#ifdef	__DUMP_RECEIVE_DATA__
		qDebug("T2 UAV date: %04d-%02d-%02d, UAV UTC time: %d", (int)cYear + 2000, (int)cMonth, (int)cDay, iUAVTime);
		qDebug("T2 UAVGPSDirection %.02f, UAVYaw: %.02f, UAVPitch: %.02f, UAVRoll: %.02f");

		qDebug() << "=============== end parse T2 ===============";
#endif
	}

	// F2
	{
		int iF2Begin = (int)sizeof(VLK_HEAD) + 2 + T2_LENGTH;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse F2 ===============";
#endif
		// 0~10�ֽڱ���

		short sYawTargetPixelDiff = ((short)(uchar)frame[iF2Begin + 11]) << 8;
		sYawTargetPixelDiff |= (uchar)frame[iF2Begin + 12];

		short sPitchTargetPixelDiff = ((short)(uchar)frame[iF2Begin + 13]) << 8;
		sPitchTargetPixelDiff |= (uchar)frame[iF2Begin + 14];
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug("F2 tracker stauts YawTargetPixelDiff: %d, sPitchTargetPixelDiff: %d", sYawTargetPixelDiff, sPitchTargetPixelDiff);
		qDebug() << "=============== end parse F2 ===============";
#endif
	}

	// B2
	{
		int iB2Begin = (int)sizeof(VLK_HEAD) + 2 + T2_LENGTH + F2_LENGTH;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse B2 ===============";
		qDebug() << "B2 gimbal timer: " << (int)frame[iB2Begin] << " ms";
#endif

		// �ŷ�������Ӧbit 0~4
		uchar uActionResponse = frame[iB2Begin + 1] & 0x0F;

		// ������֡������bit 6~7
		uchar uFrameCounter = (frame[iB2Begin + 1] >> 6) & 0x03;

		// ����Ƕ�
		short sRollAngle = ((short)(uchar)frame[iB2Begin + 3]) << 8;
		sRollAngle |= (uchar)frame[iB2Begin + 4];
		double dRollAngle = (double)sRollAngle / dAngleRate;

		// ������ٶ�
		short sRollAngleSpeed = ((short)(uchar)frame[iB2Begin + 5]) << 8;
		sRollAngleSpeed |= (uchar)frame[iB2Begin + 6];

		// ��λ���ٶ�
		short sYawAngleSpeed = ((short)(uchar)frame[iB2Begin + 7]) << 8;
		sYawAngleSpeed |= (uchar)frame[iB2Begin + 8];

		// �������ٶ�
		short sPitchAngleSpeed = ((short)(uchar)frame[iB2Begin + 9]) << 8;
		sPitchAngleSpeed |= (uchar)frame[iB2Begin + 10];

#ifdef	__DUMP_RECEIVE_DATA__
		if (uActionResponse == 0x00) qDebug() << "action response: no action";
		else if (uActionResponse == 0x08) qDebug() << "action response: adjust zero drift";
		else if (uActionResponse == 0x09) qDebug() << "action response: correct zero drift";
		else qDebug() << "B2 action response: unknown " << (int)uActionResponse;

		qDebug() << "B2 frame counter: " << (int)uFrameCounter;

		// ������Ϣ
		qDebug() << "B2 malfunction code: " << frame[iB2Begin + 2];

		qDebug("B2 roll angle: %.02f, roll angle speed: %d, yaw angle speed: %d, pitch angle speed: %d",
			dRollAngle, sRollAngleSpeed, sYawAngleSpeed, sPitchAngleSpeed);

		qDebug() << "=============== end parse B2 ===============";
#endif
	}

	// D2
	{
		int iD2Begin = (int)sizeof(VLK_HEAD) + 2 + T2_LENGTH + F2_LENGTH + B2_LENGTH;
#ifdef	__DUMP_RECEIVE_DATA__
		qDebug() << "=============== begin parse D2 ===============";
#endif
		// ��ǰ̽��������bit 0~2
		uchar uCurSensorType = frame[iD2Begin] & 0x07;

		// ���ϱ�ʶbit 3~7
		uchar uMalfunction = (frame[iD2Begin] >> 3) & 0x1F;

		// ��ѧ������������
		uchar uOpticalSensorResolution = frame[iD2Begin + 1] & 0x7F;

		// ��ǰ̽�����Ƿ�����̽����
		uchar uIsMainSensor = ((frame[iD2Begin] >> 7) & 0x01);

#ifdef	__DUMP_RECEIVE_DATA__
		if (uCurSensorType == 0x00) qDebug() << "D2 current sensor type: visible 1";
		else if (uCurSensorType == 0x01) qDebug() << "D2 current sensor type: IR";
		else if (uCurSensorType == 0x02) qDebug() << "D2 current sensor type: visible 1 + IR PIP";
		else if (uCurSensorType == 0x03) qDebug() << "D2 current sensor type: IR + visible 1 PIP";
		else if (uCurSensorType == 0x04) qDebug() << "D2 current sensor type: visible 2";
		else qDebug() << "D2 current sensor type: unknown " << uCurSensorType;


		if (uMalfunction == 0) qDebug() << "D2 malfunction: no malfunction";
		else qDebug() << "D2 malfunction: " << (int)uMalfunction;


		if (uOpticalSensorResolution == 1) qDebug() << "D2 optical sensor resolution: 1080P";
		else if (uOpticalSensorResolution == 2) qDebug() << "D2 optical sensor resolution: 2K";
		else if (uOpticalSensorResolution == 3) qDebug() << "D2 optical sensor resolution: 4K";
		else if (uOpticalSensorResolution == 4) qDebug() << "D2 optical sensor resolution: 960P";
		else if (uOpticalSensorResolution == 5) qDebug() << "D2 optical sensor resolution: 720P";
		else if (uOpticalSensorResolution == 6) qDebug() << "D2 optical sensor resolution: 640P";
		else qDebug() << "D2 optical sensor resolution: unknown " << uOpticalSensorResolution;

		// ��ǰ̽�����Ƿ�����̽����
		if ((frame[iD2Begin] >> 7) & 0x01) qDebug() << "D2 current sensor is main sensor";
		else qDebug() << "D2 current sensor is not main sensor";

		qDebug() << "=============== end parse D2 ===============";
#endif
	}

	return false;
}

bool CDeviceConnection::ProcFrameData_0x02(const QByteArray& frame)
{
	m_Latest0x02Frame = frame;

	int iVBegin = (int)sizeof(VLK_HEAD) + 2;
#ifdef	__DUMP_RECEIVE_DATA__
	qDebug() << "=============== begin parse V ===============";
#endif
	// �Ƿ񱣴�bit 0~3
	uchar uSave = frame[iVBegin] & 0x0F;

	// Э����ʽbit 4~7
	uchar uProtocol = (frame[iVBegin] >> 4) & 0x0F;

	// bit 0~3����
	// frame[iVBegin +1 ]
	// ������
	uchar uBaudRate = (frame[iVBegin + 1] >> 4) & 0x0F;

#ifdef	__DUMP_RECEIVE_DATA__
	if (uSave == 0x00) qDebug() << "V save: not save";
	else if (uSave == 0x01) qDebug() << "V save: save communicate port and baud rate ";
	else qDebug() << "V save: unknown " << uSave;


	if (uProtocol == 0x00) qDebug() << "V protocol: no action";
	else if (uProtocol == 0x02) qDebug() << "V protocol: use complete protocol";
	else if (uProtocol == 0x03) qDebug() << "V protocol: use brief protocol";
	else qDebug() << "V protocol: unknown " << uProtocol;


	if (uBaudRate == 0x00) qDebug() << "V baud rate: no action";
	else if (uBaudRate == 0x02) qDebug() << "V baud rate: 9600";
	else if (uBaudRate == 0x03) qDebug() << "V baud rate: 19200";
	else if (uBaudRate == 0x04) qDebug() << "V baud rate: 38400";
	else if (uBaudRate == 0x05) qDebug() << "V baud rate: 57600";
	else if (uBaudRate == 0x06) qDebug() << "V baud rate: 115200";
	else if (uBaudRate == 0x07) qDebug() << "V baud rate: 230400";
	else if (uBaudRate == 0x08) qDebug() << "V baud rate: 921600";
	else qDebug() << "V baud rate: unknown " << uBaudRate;

	qDebug() << "=============== end parse V ===============";
#endif
	return false;
}

void CDeviceConnection::CalculateCheckSum(QByteArray& data)
{
	if (sizeof(VLK_HEAD) >= data.size())
	{
		qCritical() << "bad parameters sizeof(VLK_HEAD) " << sizeof(VLK_HEAD) << ", data.size() = " << data.size();
		return;
	}

	data[data.size() - 1] = data[(int)sizeof(VLK_HEAD)];
	for (int i = sizeof(VLK_HEAD) + 1; i < data.size() - 1; ++i)
	{
		data[data.size() - 1] = data[data.size() - 1] ^ data[i];
	}
	//PrintHexData(data, "Home");
}

void CDeviceConnection::CalculateCheckSumV2(QByteArray& data)
{
	int iCheckSum = 0;
	for (int i = 0; i < data.size() - 1; ++i)
	{
		iCheckSum += data.at(i);
	}
	data[data.size() - 1] = (uchar)(0xFF & iCheckSum);
	
}

size_t CDeviceConnection::GetDataSizeByFrameID(uchar frameid)
{
	size_t data_size = 0;
	if (frameid == 0x00)
	{
		data_size = KEEP_ALIVE_LENGTH;
	}
	else if (frameid == 0x30)
	{
		data_size = A1_LENGTH + C1_LENGTH + E1_LENGTH;
	}
	else if (frameid == 0x31)
	{
		data_size = A2_LENGTH + C2_LENGTH + E2_LENGTH;
	}
	else if (frameid == 0x01)
	{
		data_size = U_LENGTH;
	}
	else if (frameid == 0x16)
	{
		data_size = S1_LENGTH;
	}
	else if (frameid == 0x40)
	{
		data_size = T1_LENGTH + F1_LENGTH + B1_LENGTH + D1_LENGTH;
	}
	else if (frameid == 0x41)
	{
		data_size = T2_LENGTH + F2_LENGTH + B2_LENGTH + D2_LENGTH;
	}
	else if (frameid == 0x02)
	{
		data_size = V_LENGTH;
	}
	else if (frameid == 0xB1)
	{
		data_size = M_LENGTH;
	}
	else
	{
		qDebug("unknown frame id %02x", frameid);
	}

	return data_size;
}

void CDeviceConnection::ParseTelemetryData(const QByteArray& data)
{
	qDebug("data.size() = %d, GetDataSizeByFrameID(0x40) = %d, GetDataSizeByFrameID(0x41) = %d",
		data.size(), GetDataSizeByFrameID(0x40), GetDataSizeByFrameID(0x41));
}

bool CDeviceConnection::IsMotorOn()
{
	int iB1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH + F1_LENGTH;
	uchar cGimbalStatus = (m_Latest0x40Frame[iB1Begin] >> 4) & 0x0F;

	return cGimbalStatus != 0x00 && cGimbalStatus != 0x0F;
}

bool CDeviceConnection::IsFollowMode()
{
	int iB1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH + F1_LENGTH;
	uchar cGimbalStatus = (m_Latest0x40Frame[iB1Begin] >> 4) & 0x0F;

	return cGimbalStatus == 0x03;
}

bool CDeviceConnection::IsTracking()
{
	int iB1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH + F1_LENGTH;
	uchar cGimbalStatus = (m_Latest0x40Frame[iB1Begin] >> 4) & 0x0F;

	return cGimbalStatus == 0x06;
}

bool CDeviceConnection::IsRecording()
{
	int iD1Begin = (int)sizeof(VLK_HEAD) + 2 + T1_LENGTH + F1_LENGTH + B1_LENGTH;
	short sByte2_3 = ((short)m_Latest0x40Frame[iD1Begin + 2]) << 8;
	sByte2_3 |= (uchar)m_Latest0x40Frame[iD1Begin + 3];

	// qDebug() << "################### sByte2_3: " << sByte2_3;
	uchar uRecordStatus = sByte2_3 & 0x3;

	return uRecordStatus == 0x01;
}

bool CDeviceConnection::IsDefogOn()
{
	return false;
}
