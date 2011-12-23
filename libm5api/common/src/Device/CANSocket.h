//-----------------------------------------------
// Neobotix 
// www.neobotix.de
// Copyright (c) 2003. All rights reserved.

// author: Oliver Barth
//-----------------------------------------------
#ifndef CANSOCKET_INCLUDEDEF_H
#define CANSOCKET_INCLUDEDEF_H
//-----------------------------------------------

#include "CanItf.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
 
#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>
 
#ifndef PF_CAN
#define PF_CAN 29
#endif
 
#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif

//-----------------------------------------------

#include <iostream>
#include <cstdio>
#include <errno.h>
#include "../Device/ProtocolDevice.h"


//-----------------------------------------------

class CANSocket : public CProtocolDevice
{
public:
	// --------------- Interface
	CANSocket();
	~CANSocket();
	//void destroy() {};
	bool receiveMsgRetry(CanMsg* pCMsg, int iNrOfRetry);
	//bool isObjectMode() { return false; }
	int init();
	int init(unsigned long baudRate);
	int init(const char* acInitString);
	int exit();
	int waitForStartMotionAll();
	void setQueueSize(unsigned short uiQueueSize);
	void setTimeOut(unsigned long uiTimeOut);


protected:
	// --------------- Types
	bool m_bInitialized;
	bool m_bSimuEnabled;
	int m_skt;
	//bool m_bIsTXError;
	//int m_LastID;
	//static const int c_iInterrupt;
	//static const int c_iPort;
	//void initIntern();
	//int m_hDevice;
	//int m_hSyncDevice;
	//int m_iDeviceId;
	//unsigned long  m_uiBaudRate;	
	//unsigned short m_uiQueueSize;
	//unsigned long m_uiTimeOut;
	int getDeviceError(int iErrorState);
	int setBaudRate();
	int setBaudRate(unsigned char iBaudRate);
	int setMessageId(unsigned long uiMessageId);
	int clearReadQueue();
	int reinit(unsigned char ucBaudRateId);
	int readDevice(CProtocolMessage& rclProtocolMessage);
	int writeDevice(CProtocolMessage& rclProtocolMessage);
	int m_iNoOfRetries;
	//char * m_DeviceName;
};
//-----------------------------------------------
#endif
