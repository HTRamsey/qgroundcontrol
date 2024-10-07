#pragma once
#include "DeviceConnection.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include "EncoderObject.h"

class CTCPConnection : public QObject, public CDeviceConnection
{
	friend class CEncoderObject;
	Q_OBJECT
public:
	CTCPConnection(IApplicationContext* pContext);
	virtual ~CTCPConnection();

	void SetConnectInfo(const QString& strIP, int iPort);
	void GetConnectInfo(QString& strIP, int& iPort);
	CEncoderObject* GetEncoderObject();
	void SuspendKeepAlive();
	void ResumeKeepAlive();

	virtual bool Connect();
	virtual bool IsConnected() const;
	
	virtual void Disconnect();

	virtual void Move(short uHorizontalSpeed, short uVeritcalSpeed);
	virtual void MoveJoy(short uHorizontalSpeed, short uVeritcalSpeed);
	virtual void Stop();
	virtual void StopJoy();
	virtual void ZoomIn(short uSpeed);
	virtual void ZoomOut(short uSpeed);
	virtual void StopZoom();
	virtual void EnableTrackMode(const TrackModeParam& param);
	virtual void TrackTargetPosition(int iX, int iY);
	virtual void TrackTargetPositionEx(const TrackModeParam& param, int iX, int iY);
	virtual void DisableTrackMode();
	virtual void FocusIn(short uSpeed);
	virtual void FocusOut(short uSpeed);
	virtual void StopFocus();

	virtual void Home(bool clear);
	virtual void SwitchMotor(bool bOn);
	virtual void EnableFollowMode(bool bEnable);
	virtual void TurnTo(double dYaw, double dPitch);
	virtual void SetGyroCalibration(double dYaw);
	
	//DreamSky 20210913
		// ������̨�����涯
	virtual void SetGimbalCalibration(int valueType, double dLat, double dLng, int altValue);
	virtual void SetGimbalCalibrationUp(double dLat);
	virtual void SetGimbalCalibrationDown(double dLat);
	virtual void SetGimbalCalibrationLeft(double dLat);
	virtual void SetGimbalCalibrationRight(double dLat);
	virtual void SetTemAlarm(int tem);
	virtual void SetDZoomMax(double dLat);
	virtual void SetGimbalCalibrationT1(double dLat, double dLng,double dLat1, double dLng1);
	virtual void StartTrack();
	virtual void StartCarRecognition();
	virtual void StartCarTrack();
	virtual void StopCarRecognition();
	virtual void StopTrack();
	// 0: auto; 1: 32; 2: 64: 3: 128;
	virtual void SetTrackTemplateSize(int iSize);

	virtual void SetImageColor(ImageType emImgType, bool bEnablePIP, IRColor emIRColor);
	// cImgType: 0x01, 0x02, 0x03, 0x04, 0x05
	virtual void SetImageColorV2(uchar cImgType);
	// cIRColor: 0x0E, 0x0F, 0x12
	virtual void SetImageColorV3(uchar cIRColor);

	virtual void Photograph(bool clear);
	virtual void SwitchRecord(bool bOn, bool clear);
	virtual void SwitchDefog(bool bOn);

	// ����¼��ģʽ
	// iMode: 0 photo mode; 1 record mode
	virtual void SetRecordMode(int iMode);

	// ���þ۽�ģʽ
	// iMode: 0 automatically; 1 manually
	virtual void SetFocusMode(int iMode);

	// �������ָ��
	virtual void SwitchLaser(bool bOn);
	virtual void LaserZoomIn(short uSpeed);
	virtual void LaserZoomOut(short uSpeed);
	virtual void LaserStopZoom();
	// iMode: 0 automatically follow EO; 1 manually
	virtual void SetLaserZoomMode(int iMode);

	// �ɼ���ŵ���ָ��������
	virtual void ZoomTo(float fMag);
	// �����ǵ��ӷŴ���С
	virtual void IRDigitalZoomIn(short uSpeed);
	virtual void IRDigitalZoomOut(short uSpeed);
	// �򿪹ر�EO digital zoom
	virtual void SwitchEODigitalZoom(bool bOn);

	// ����SBUSģʽ
	virtual void EnterSBUSMode();
	
	// �˳�SBUSģʽ
	virtual void ExitSBUSMode();

	// ��ѯ�豸��ǰ����
	virtual void QueryDevConfiguration();
	virtual void SetTimeZone(bool newOSDVersion,char tz);
	
	// �����豸OSD
	virtual void SetOSD(bool newOSDVersion,bool newOSDVersionLeft,uchar uParam1, uchar uParam2);
	virtual void SetTem(bool check,uchar uParam1, uchar uParam2);
	virtual void SetWirelessCtrlChnlMap(const VLK_ChannelsMap& ChnlsMap);
	virtual void SetWirelessCtrlChnlMapV2(const VLK_ChannelsMap& ChnlsMap, const VLK_ChannelsInvertFlag& ChnlInvertFlag);

	// ������չ�Զ�������
	virtual void SendExtentCmd(const QByteArray& cmd);

	//DreamSky 20210918
	virtual void SendExtentCmdReceiveData(bool cmd);

	// �޸��豸����
	// �ýӿ���Ҫ���ڲ��������豸���ͺź��豸���к�
	virtual void SetDevConfiguration(const VLK_DevConfig& DeviceCfg);

	// ������̨�����涯
	virtual void SetGimbalAimPoint(double dLat, double dLng, double dAlt);

	void sendMessage(QString msg);//�źź���ֻ������������Ҫд����
	
signals:
	void sendInfoText(QString str);

protected:
	virtual void timerEvent(QTimerEvent *event) override;

private:
	QByteArray Package(const QByteArray& cmd);
	
private slots:
	void onConnected();
	void onReadyRead();
	
	void onSocketError(QAbstractSocket::SocketError);

private:
	QString m_strIP;
	int m_iPort;

	QTcpSocket m_TcpSocket;

	int m_iReadBufferTimer;
	int m_iRepeatStopTimer;
	int m_count;
	QByteArray m_StopCommand;

	CEncoderObject* m_pEncoderObject;

	bool m_bKeepAliveSuspended;

	bool isGetInfoCome;

	bool isReceiveData;
};

