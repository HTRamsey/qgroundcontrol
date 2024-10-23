#pragma once
#include "DeviceConnection.h"
#include <QUdpSocket>

class CUDPConnection : public QObject, public CDeviceConnection
{
	Q_OBJECT
public:
	CUDPConnection(IApplicationContext* pContext);
	virtual ~CUDPConnection();

	void SetConnectInfo(const QString& strRemoteIPAddr, int iRemotePort, const QString& strLocalIPAddr, int iLocalPort);
	void GetConnectInfo(QString& strRemoteIPAddr, int& iRemotePort, QString& strLocalIPAddr, int& iLocalPort) const;

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

	virtual void SetRecordMode(int iMode);
	virtual void SetFocusMode(int iMode);

	// 激光控制指令
	virtual void SwitchLaser(bool bOn);
	virtual void LaserZoomIn(short uSpeed);
	virtual void LaserZoomOut(short uSpeed);
	virtual void LaserStopZoom();
	// iMode: 0 automatically follow EO; 1 manually
	virtual void SetLaserZoomMode(int iMode);

	// 可见光放到到指定倍数字
	virtual void ZoomTo(float fMag);

	// 热像仪电子放大缩小
	virtual void IRDigitalZoomIn(short uSpeed);
	virtual void IRDigitalZoomOut(short uSpeed);

	// 打开关闭EO digital zoom
	virtual void SwitchEODigitalZoom(bool bOn);

	// 进入SBUS模式
	virtual void EnterSBUSMode();

	// 退出SBUS模式
	virtual void ExitSBUSMode();

	// 查询设备当前配置
	virtual void QueryDevConfiguration();
	virtual void SetTimeZone(bool newOSDVersion,char tz);

	// 设置设备OSD
	virtual void SetOSD(bool newOSDVersion,bool newOSDVersionLeft,uchar uParam1, uchar uParam2);
	virtual void SetTem(bool check,uchar uParam1, uchar uParam2);
	virtual void SetWirelessCtrlChnlMap(const VLK_ChannelsMap& ChnlsMap);
	virtual void SetWirelessCtrlChnlMapV2(const VLK_ChannelsMap& ChnlsMap, const VLK_ChannelsInvertFlag& ChnlInvertFlag);

	// 发送扩展自定义命令
	virtual void SendExtentCmd(const QByteArray& cmd);

	virtual void SetGyroCalibration(double dLat);
	virtual void SetTemAlarm(int tem);

	virtual void SetDZoomMax(double dLat);
	

	//DreamSky 20210918
	virtual void SendExtentCmdReceiveData(bool cmd);

	// 修改设备配置
	// 该接口主要用于产线配置设备序型号和设备序列号
	virtual void SetDevConfiguration(const VLK_DevConfig& DeviceCfg);

	// 设置云台定点随动
	virtual void SetGimbalAimPoint(double dLat, double dLng, double dAlt);

	//DreamSky 20210913 Calibration
	virtual void SetGimbalCalibration(int valueType, double dLat, double dLng, int altValue);
	virtual void SetGimbalCalibrationUp(double dLat);
	virtual void SetGimbalCalibrationDown(double dLat);
	virtual void SetGimbalCalibrationLeft(double dLat);
	virtual void SetGimbalCalibrationRight(double dLat);
	//DreamSky 20210913 Calibration
	virtual void SetGimbalCalibrationT1(double dLat, double dLng, double dLat1, double dLng1);
	

protected:
	virtual void timerEvent(QTimerEvent *event) override;
private:
	QByteArray Package(const QByteArray& cmd);
private slots:
	void slot_hostfound();
	void slot_connected();
	void slot_read_data();
	void slot_udp_error(QAbstractSocket::SocketError error);

private:
	QString m_strRemoteIPAddr, m_strLocalIPAddr;
	int m_iRemotePort, m_iLocalPort;

	QUdpSocket m_UdpSocket;

	int m_iReadBufferTimer;
	int m_iRepeatStopTimer;
	int m_count;
	QByteArray m_StopCommand;
};
