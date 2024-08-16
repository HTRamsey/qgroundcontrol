#pragma once
#include "DeviceConnection.h"
#include "serial/serial.h"
#include <list>
#include <string>

class CSerialPortConnection : public CDeviceConnection
{
public:
    CSerialPortConnection(const std::string& strPortName = "", int iBaudRate = 115200);
	virtual ~CSerialPortConnection();

    virtual void SetConnectInfo(const std::string& strPortName, int iBaudRate);
    void GetConnectInfo(std::string& strPortName, int& iBaudRate);
	CEncoderObject* GetEncoderObject() { return NULL; }
	virtual bool Connect();
	virtual bool IsConnected();
	virtual void Disconnect();

	virtual void Move(short uHorizontalSpeed, short uVeritcalSpeed);
	virtual void Stop();
	virtual void ZoomIn(short uSpeed);
	virtual void ZoomOut(short uSpeed);
	virtual void StopZoom();
    virtual void EnableTrackMode(const VLK_TRACK_MODE_PARAM& param);
	virtual void TrackTargetPosition(int iX, int iY);
    virtual void TrackTargetPositionEx(const VLK_TRACK_MODE_PARAM& param, int iX, int iY);
	virtual void DisableTrackMode();
	virtual void FocusIn(short uSpeed);
	virtual void FocusOut(short uSpeed);
	virtual void StopFocus();
    virtual void StopCarRecognition();
    virtual void StartCarTrack();
    virtual void StartCarRecognition();
	virtual void Home();
	virtual void SwitchMotor(bool bOn);
	virtual void EnableFollowMode(bool bEnable);
	virtual void TurnTo(double dYaw, double dPitch);
                virtual void SetGimbalCalibration(double dLat, double dLng);
                virtual void SetGimbalCalibrationT1(double dLat, double dLng, double dLat1, double dLng1);
	virtual void StartTrack();
	virtual void StopTrack();
	// 0: auto; 1: 32; 2: 64: 3: 128;
	virtual void SetTrackTemplateSize(int iSize);

    virtual void SetImageColor(VLK_IMAGE_TYPE emImgType, bool bEnablePIP, VLK_IR_COLOR emIRColor);
	// cImgType: 0x01, 0x02, 0x03, 0x04, 0x05
	virtual void SetImageColorV2(uchar cImgType);
	// cIRColor: 0x0E, 0x0F, 0x12
	virtual void SetImageColorV3(uchar cIRColor);

	virtual void Photograph();
	virtual void SwitchRecord(bool bOn);
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
	// ����ʱ��
	virtual void SetTimeZone(char tz);
	// �����豸OSD
	virtual void SetOSD(uchar uParam1, uchar uParam2);
	// ��������ң����ͨ������
    virtual void SetWirelessCtrlChnlMap(const VLK_CHANNELS_MAP& ChnlsMap);
	virtual void SetWirelessCtrlChnlMapV2(const VLK_CHANNELS_MAP& ChnlsMap, const VLK_CHANNELS_INVERT_FLAG& ChnlInvertFlag);
	virtual void SendExtentCmd(const QByteArray& cmd);

	// �޸��豸����
	// �ýӿ���Ҫ���ڲ��������豸���ͺź��豸���к�
    virtual void SetDevConfiguration(const VLK_DEV_CONFIG& DeviceCfg);

    void SendKeepAlive();
	virtual void Reconnect(){}
protected:
    virtual void DoReadBuffer();
private:
    serial::Serial m_SerialPort;
};

