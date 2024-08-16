// ViewLink.cpp : 定义 DLL 应用程序的导出函数。
//

#if (defined _WIN32) || (defined _WIN64)
// #include <windows.h>
#else
#endif
#undef  LOG_TAG
#define LOG_TAG     "ViewLink.cpp"



#include "ViewLink.h"
#include "Utility/VLKLog.h"
#include "TCPConnection.h"
#include "SerialPortConnection.h"
#include "EncoderObject.h"

const char g_szVersion[] = "3.3.3";

CDeviceConnection* g_pDeviceConnection = NULL;
VLK_CONN_TYPE g_emPreferCon = VLK_CONN_TYPE_SERIAL_PORT;
int g_iKeepAliveInterval = KEEP_ALIVE_INTERVAL;
VLK_DevStatus_CB g_pDevStatusCB = NULL;
void* g_pDevStatusUserParam = NULL;

CDeviceConnection* GetPreferConnection()
{
   /*if (g_SerialPortConnection.IsConnected() && !g_TCPConnection.IsConnected())
   {
       return &g_SerialPortConnection;
   }
   else if (!g_SerialPortConnection.IsConnected() && g_TCPConnection.IsConnected())
   {
       return &g_TCPConnection;
   }
   else
   {
       if (g_emPreferCon == VLK_CONN_TYPE_SERIAL_PORT)
       {
           return &g_SerialPortConnection;
       }
       else if (g_emPreferCon == VLK_CONN_TYPE_TCP)
       {
           return &g_TCPConnection;
       }
   }*/

   return g_pDeviceConnection;
}

VLK_API const char* VLK_CALL GetSDKVersion()
{
	return g_szVersion;
}

VLK_API	int	VLK_CALL VLK_Init()
{
	setLogWriteLevel(log_level_debug);
    DEBUG_LOG("########### ViewLink SDK %s initialized ###########", g_szVersion);
	return 0;
}

VLK_API	void	VLK_CALL VLK_UnInit()
{
    VLK_Disconnect();
    DEBUG_LOG("########### ViewLink SDK %s uninitialized ###########", g_szVersion);
}

VLK_API	int	VLK_CALL VLK_Connect(const VLK_CONN_PARAM* pConnParam, VLK_ConnStatus_CB pConnStatusCB, void* pUserParam)
{
	if (NULL == pConnParam)
	{
		ERROR_LOG("bad input parameters, pConnParam == NULL");
		return VLK_ERROR_INVALID_PARAM;
	}

	if (g_pDeviceConnection != NULL)
	{
		g_pDeviceConnection->Disconnect();
		g_pDeviceConnection = NULL;
	}

	DEBUG_LOG(__FUNCTION__);
	if (pConnParam->emType == VLK_CONN_TYPE_SERIAL_PORT)
	{
	   g_pDeviceConnection = new CSerialPortConnection();

	   g_pDeviceConnection->RegisterConnStatusCB(pConnStatusCB, pUserParam);
	   g_pDeviceConnection->SetConnectInfo(pConnParam->ConnParam.SerialPort.szSerialPortName,
                                           pConnParam->ConnParam.SerialPort.iBaudRate);
	   g_pDeviceConnection->SetKeepAliveInterval(g_iKeepAliveInterval);
	   if (g_pDevStatusCB != NULL)
	   {
		   g_pDeviceConnection->RegisterDevStatusCB(g_pDevStatusCB, g_pDevStatusUserParam);
	   }
	   g_pDeviceConnection->Connect();
	}
	else if (pConnParam->emType == VLK_CONN_TYPE_TCP)
	{
		g_pDeviceConnection = new CTCPConnection();
		g_pDeviceConnection->RegisterConnStatusCB(pConnStatusCB, pUserParam);
		g_pDeviceConnection->SetConnectInfo(pConnParam->ConnParam.IPAddr.szIPV4,
											pConnParam->ConnParam.IPAddr.iPort);
		g_pDeviceConnection->SetKeepAliveInterval(g_iKeepAliveInterval);
		if (g_pDevStatusCB != NULL)
		{
			g_pDeviceConnection->RegisterDevStatusCB(g_pDevStatusCB, g_pDevStatusUserParam);
		}
		g_pDeviceConnection->Connect();
	}
    else
    {
       ERROR_LOG("unsupported connection type: %d", pConnParam->emType);
       return VLK_ERROR_INVALID_PARAM;
    }

	

	return VLK_ERROR_NO_ERROR;
}

VLK_API	int	VLK_CALL VLK_IsConnected()
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL)
	{
		return g_pDeviceConnection->IsConnected() ? 1 : 0;
	}

    return 0;
}

VLK_API	int     VLK_CALL    VLK_IsTCPConnected()
{
    DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL &&
		g_pDeviceConnection->GetConnectionType() == CT_TCP &&
		g_pDeviceConnection->IsConnected())
	{
		return 1;
	}
	
	return 0;
}



VLK_API	void	VLK_CALL VLK_StartCarRecognition()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StartCarRecognition();
    }
}
VLK_API	void	VLK_CALL VLK_StopCarRecognition()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StopCarRecognition();
    }
}

VLK_API	void	VLK_CALL VLK_StartCarTrack()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StartCarTrack();
    }
}

VLK_API	int     VLK_CALL    VLK_IsSerialPortConnected()
{
    DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL &&
		g_pDeviceConnection->GetConnectionType() == CT_SerialPort &&
		g_pDeviceConnection->IsConnected())
	{
		return 1;
	}

	return 0;
}

VLK_API	void	VLK_CALL VLK_Disconnect()
{
    DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL)
	{
		g_pDeviceConnection->Disconnect();
	}
}

VLK_API	void	VLK_CALL    VLK_DisconnectTCP()
{
    DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL && g_pDeviceConnection->GetConnectionType() == CT_TCP)
	{
		g_pDeviceConnection->Disconnect();
	}
}

VLK_API	void	VLK_CALL    VLK_DisconnectSerialPort()
{
    DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL && g_pDeviceConnection->GetConnectionType() == CT_SerialPort)
	{
		g_pDeviceConnection->Disconnect();
	}
}

VLK_API void	VLK_CALL VLK_SetKeepAliveInterval(int iMS)
{
    DEBUG_LOG(__FUNCTION__);

	if (g_pDeviceConnection != NULL)
	{
		g_pDeviceConnection->SetKeepAliveInterval(iMS);
	}
	
	g_iKeepAliveInterval = iMS;
}

VLK_API	void	VLK_CALL VLK_RegisterDevStatusCB(VLK_DevStatus_CB pDevStatusCB, void* pUserParam)
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection != NULL)
	{
		g_pDeviceConnection->RegisterDevStatusCB(pDevStatusCB, pUserParam);
	}

	g_pDevStatusCB = pDevStatusCB;
	g_pDevStatusUserParam = pUserParam;
}

VLK_API	void	VLK_CALL VLK_Move(short sHorizontalSpeed, short sVeritcalSpeed)
{
    // DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->Move(sHorizontalSpeed, sVeritcalSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_Stop()
{
    // DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->Stop();
    }
}

VLK_API	void	VLK_CALL VLK_ZoomIn(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->ZoomIn(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_ZoomOut(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->ZoomOut(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_StopZoom()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StopZoom();
    }
}

VLK_API	void	VLK_CALL VLK_EnableTrackMode(const VLK_TRACK_MODE_PARAM* pParam)
{
    if (NULL == pParam)
	{
		ERROR_LOG("bad input parameters");
		return;
	}

	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->EnableTrackMode(*pParam);
    }
}

VLK_API	void	VLK_CALL VLK_TrackTargetPosition(int iX, int iY, int iVideoWidth, int iVideoHeight)
{
	if (iVideoWidth == 0 || iVideoHeight == 0)
	{
		ERROR_LOG("bad input parametes, iVideoWidth = %d, iVideoHeight = %d", iVideoWidth, iVideoHeight);
		return;
	}

	DEBUG_LOG(__FUNCTION__);
	iX = ((float)1920 / (float)iVideoWidth) * iX;
	iY = ((float)1080 / (float)iVideoHeight) * iY;
	iX -= (1920 >> 1);
	iY -= (1080 >> 1);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->TrackTargetPosition(iX, iY);
    }
}

VLK_API	void	VLK_CALL VLK_TrackTargetPositionEx(const VLK_TRACK_MODE_PARAM* pParam, int iX, int iY, int iVideoWidth, int iVideoHeight)
{
                  DEBUG_LOG("TrackPosition ---ViewLink.cpp -TrackTargetPosition--20210817-- !!!");
	if (pParam == NULL || iVideoWidth == 0 || iVideoHeight == 0)
	{
		ERROR_LOG("bad input parametes, pParam = %p, iVideoWidth = %d, iVideoHeight = %d", pParam, iVideoWidth, iVideoHeight);
		return;
	}
                ERROR_LOG("TrackPosition, pParam = %p, iX = %d, iY = %d", pParam, iX, iY);
	DEBUG_LOG(__FUNCTION__);
	iX = ((float)1920 / (float)iVideoWidth) * iX;
	iY = ((float)1080 / (float)iVideoHeight) * iY;
	iX -= (1920 >> 1);
	iY -= (1080 >> 1);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        ERROR_LOG("TrackPosition---, pParam = %p, iX = %d, iY = %d", pParam, iX, iY);
        pPreferConn->TrackTargetPositionEx(*pParam, iX, iY);
    }
}

VLK_API	void	VLK_CALL VLK_DisableTrackMode()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->DisableTrackMode();
    }
}

VLK_API	void	VLK_CALL VLK_FocusIn(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->FocusIn(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_FocusOut(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->FocusOut(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_StopFocus()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StopFocus();
    }
}

VLK_API	void	VLK_CALL VLK_Home()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->Home();
    }
}

VLK_API	void	VLK_CALL VLK_SwitchMotor(int iOn)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
         pPreferConn->SwitchMotor(!!iOn);
    }
}

VLK_API	int	VLK_CALL VLK_IsMotorOn()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        return pPreferConn->IsMotorOn() ? 1 : 0;
    }

    return 0;
}

VLK_API	void	VLK_CALL VLK_EnableFollowMode(int iEnable)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->EnableFollowMode(!!iEnable);
    }
}

VLK_API	int	VLK_CALL VLK_IsFollowMode()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       return pPreferConn->IsFollowMode() ? 1 : 0;
    }

    return 0;
}

VLK_API	void	VLK_CALL VLK_TurnTo(double dYaw, double dPitch)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->TurnTo(dYaw, dPitch);
    }
}

VLK_API	void	VLK_CALL VLK_SetGimbalCalibration(double dLat, double dLng)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SetGimbalCalibration(dLat, dLng);
    }
}

VLK_API	void	VLK_CALL VLK_SetGimbalCalibrationT1(double dLat, double dLng, double dLat1, double dLng1)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SetGimbalCalibrationT1(dLat, dLng,dLat1, dLng1);
    }
}

VLK_API	void	VLK_CALL VLK_StartTrack()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StartTrack();
    }
}

VLK_API	void	VLK_CALL VLK_StopTrack()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->StopTrack();
    }
}

VLK_API	int	VLK_CALL VLK_IsTracking()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        return pPreferConn->IsTracking() ? 1 : 0;
    }

    return 0;
}

VLK_API	void	VLK_CALL VLK_SetTrackTemplateSize(VLK_TRACK_TEMPLATE_SIZE emSize)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       pPreferConn->SetTrackTemplateSize(emSize);
    }
}

VLK_API	void	VLK_CALL VLK_SetImageColor(VLK_IMAGE_TYPE emImgType, int iEnablePIP, VLK_IR_COLOR emIRColor)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SetImageColor(emImgType, !!iEnablePIP, emIRColor);
    }
}

VLK_API	void	VLK_CALL VLK_Photograph()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->Photograph();
    }
}

VLK_API	void	VLK_CALL VLK_SwitchRecord(int iOn)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SwitchRecord(!!iOn);
    }
}

VLK_API	int	VLK_CALL VLK_IsRecording()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       return pPreferConn->IsRecording() ? 1 : 0;
    }

    return 0;
}

VLK_API	void	VLK_CALL VLK_SwitchDefog(int iOn)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SwitchDefog(!!iOn);
    }
}

VLK_API	int	VLK_CALL VLK_IsDefogOn()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        return pPreferConn->IsDefogOn() ? 1 : 0;
    }

    return 0;
}

VLK_API	void	VLK_CALL VLK_SetRecordMode(VLK_RECORD_MODE emMode)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SetRecordMode(emMode);
    }
}

VLK_API	 int	VLK_CALL VLK_GetRecordMode()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        return pPreferConn->GetRecordMode();
    }

    return VLK_RECORD_MODE_NONE;
}

VLK_API	void	VLK_CALL VLK_SetFocusMode(VLK_FOCUS_MODE emMode)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       pPreferConn->SetFocusMode(emMode);
    }
}

VLK_API	int	VLK_CALL VLK_GetFocusMode()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        return pPreferConn->GetFocusMode();
    }

    return VLK_FOCUS_MODE_AUTO;
}

VLK_API	void	VLK_CALL VLK_ZoomTo(float fMag)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
      pPreferConn->ZoomTo(fMag);
    }
}

VLK_API	void	VLK_CALL VLK_IRDigitalZoomIn(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->IRDigitalZoomIn(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_IRDigitalZoomOut(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       pPreferConn->IRDigitalZoomOut(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_SwitchEODigitalZoom(int iOn)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       pPreferConn->SwitchEODigitalZoom(!!iOn);
    }
}

VLK_API	void	VLK_CALL VLK_EnterSBUSMode()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
       pPreferConn->EnterSBUSMode();
    }
}

VLK_API	void	VLK_CALL VLK_ExitSBUSMode()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->ExitSBUSMode();
    }
}

VLK_API	void	VLK_CALL VLK_QueryDevConfiguration()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->QueryDevConfiguration();
    }
}

VLK_API	void	VLK_CALL    VLK_SetTimeZone(char cTZ)
{
	DEBUG_LOG(__FUNCTION__);
	CDeviceConnection* pPreferConn = GetPreferConnection();
	if (pPreferConn != NULL)
	{
		pPreferConn->SetTimeZone(cTZ);
	}
}

VLK_API	void	VLK_CALL VLK_SetOSD(const VLK_OSD_PARAM* pParam)
{
	if (NULL == pParam)
	{
		ERROR_LOG("bad input parameters, pConnParam == NULL");
		return;
	}

	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        char cOSD = ~pParam->cOSD;
        pPreferConn->SetOSD(cOSD, pParam->cOSDInput);
    }
}

VLK_API	void	VLK_CALL VLK_SetWirelessCtrlChnlMap(const VLK_CHANNELS_MAP* pChnlsMap)
{
	if (NULL == pChnlsMap)
	{
		ERROR_LOG("bad input parameters, pConnParam == NULL");
		return;
	}

	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SetWirelessCtrlChnlMap(*pChnlsMap);
    }
}

VLK_API	void	VLK_CALL    VLK_SetWirelessCtrlChnlMapV2(const VLK_CHANNELS_MAP* pChnlsMap, const VLK_CHANNELS_INVERT_FLAG* pChnlInvertFlag)
{
	if (NULL == pChnlsMap)
	{
		ERROR_LOG("bad input parameters, pConnParam == NULL");
		return;
	}

	DEBUG_LOG(__FUNCTION__);
	CDeviceConnection* pPreferConn = GetPreferConnection();
	if (pPreferConn != NULL)
	{
		pPreferConn->SetWirelessCtrlChnlMapV2(*pChnlsMap, *pChnlInvertFlag);
	}
}

VLK_API	void	VLK_CALL VLK_SendExtentCmd(const char* szCmd, int iLen)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        ERROR_LOG("zhangtao vlk_sendExtentCmd: %s",szCmd);
        QByteArray cmd;
  
      

        cmd.insert(cmd.end(), szCmd, szCmd + iLen);

         
    for (int i=0; i<cmd.size(); i++) {
      ERROR_LOG("zhangtao --------- insert -----: %d", cmd[i]); 
}
       // ERROR_LOG("zhangtao vlk_sendExtentCmd insert -----: %s",szCmd->cmd);
          /* QByteArray data;
           data.resize(19);
		data[0] = 0xEB;
		data[1] = 0x90;
		data[2] = 0x0F;
		data[3] = 0x55;
		data[4] = 0xAA;
		data[5] = 0xDC;
		data[6] = 0x0C;
		data[7] = 0x1A;
		data[8] = 0x0E;
		data[9] = 0x00;
		data[10] = 0x00;
		data[11] = 0xFF;
		data[12] = 0x88;
		data[13] = 0x00;
		data[14] = 0x00;
		data[15] = 0x01;
		data[16] = 0x18;
		data[17] = 0x76;
		data[18] = 0x25;*/
         PrintHexData(cmd, "zhangtao hexData");
         ERROR_LOG("zhangtao --------- >SendExtentCmd-----:"); 
         pPreferConn->SendExtentCmd(cmd);

        
    }
}

VLK_API	void	VLK_CALL VLK_SwitchLaser(int iOn)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SwitchLaser(!!iOn);
    }
}

VLK_API	void	VLK_CALL VLK_LaserZoomIn(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->LaserZoomIn(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_LaserZoomOut(short sSpeed)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->LaserZoomOut(sSpeed);
    }
}

VLK_API	void	VLK_CALL VLK_LaserStopZoom()
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->LaserStopZoom();
    }
}

VLK_API	void	VLK_CALL VLK_SetLaserZoomMode(VLK_LASER_ZOOM_MODE emMode)
{
	DEBUG_LOG(__FUNCTION__);
    CDeviceConnection* pPreferConn = GetPreferConnection();
    if (pPreferConn != NULL)
    {
        pPreferConn->SetLaserZoomMode(emMode);
    }
}

VLK_API	void	VLK_CALL    VLK_QueryEncoderStatus()
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pEncoderObject != NULL)
	{
		pEncoderObject->QueryEncoderStatus();
	}
}

VLK_API	void	VLK_CALL    VLK_QueryEncoderFirmwareVersion()
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pEncoderObject != NULL)
	{
		pEncoderObject->QueryDevVersion();
	}
}

VLK_API	void	VLK_CALL    VLK_QueryEncoderConfiguration()
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pEncoderObject != NULL)
	{
		pEncoderObject->GetVideoConfiguration();
	}
}

VLK_API	void	VLK_CALL    VLK_SetEncoderConfiguration(const VLK_DEV_IP_ENCODER_CONFIGURE* pConfig)
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pConfig != NULL && pEncoderObject != NULL)
	{
		pEncoderObject->SetVideoConfiguration(*pConfig);
	}
}

VLK_API	void	VLK_CALL    VLK_SetEncoderNetworkConfig(const VLK_DEV_IP_ENCODER_NETWORK* pConfig)
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pConfig != NULL && pEncoderObject != NULL)
	{
		pEncoderObject->SetNetworkConfiguration(*pConfig);
	}
}

VLK_API	void	VLK_CALL    VLK_SetEncoderBandwidth(VLK_DEV_IP_ENCODER_BANDWIDTH emBandwith)
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pEncoderObject != NULL)
	{
		pEncoderObject->SetBandwidth(emBandwith);
	}
}

VLK_API	void	VLK_CALL    VLK_SetEncoderDHCP(int iOn)
{
	DEBUG_LOG(__FUNCTION__);
	if (g_pDeviceConnection == NULL || !g_pDeviceConnection->IsConnected())
	{
		ERROR_LOG("TCP is not connected, could not access ip encoder");
		return;
	}

	CEncoderObject* pEncoderObject = g_pDeviceConnection->GetEncoderObject();
	if (pEncoderObject != NULL)
	{
		pEncoderObject->SetDHCP(!!iOn);
	}
}
