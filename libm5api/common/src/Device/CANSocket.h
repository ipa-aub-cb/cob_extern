//-----------------------------------------------
// Neobotix 
// www.neobotix.de
// Copyright (c) 2003. All rights reserved.

// author: Oliver Barth
//-----------------------------------------------
#ifndef CANSOCKET_INCLUDEDEF_H
#define CANSOCKET_INCLUDEDEF_H
//-----------------------------------------------
//#include <Neobotix/Drivers/Can/CanItf.h>
//#include <libpcan.h>
//#include <Neobotix/Utilities/IniFile.h>
#include "CanItf.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
 
#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>
 
/* At time of writing, these constants are not defined in the headers */
#ifndef PF_CAN
#define PF_CAN 29
#endif
 
#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif

//-----------------------------------------------
//#include <windows.h>
#include <iostream>
#include <cstdio>
#include <errno.h>
//#include <cob_forcetorque/CanItf.h>
//#include <libntcan/ntcan.h>
//#include <Neobotix/Utilities/IniFile.h>
//#include <cob_forcetorque/Mutex.h>




//-----------------------------------------------

class CANSocket : public CanItf
{
public:
	// --------------- Interface
	CANSocket();//const char* cIniFile);
	~CANSocket();
	void init();
	void destroy() {};
	bool transmitMsg(CanMsg CMsg, bool bBlocking = true);
	bool receiveMsg(CanMsg* pCMsg);
	bool receiveMsgRetry(CanMsg* pCMsg, int iNrOfRetry);
	bool isObjectMode() { return false; }

private:
	// --------------- Types
	//HANDLE m_skt;
	
	bool m_bInitialized;
	//IniFile m_IniFile;
	bool m_bSimuEnabled;
	int m_skt;
	bool m_bIsTXError;
	int m_LastID;
	static const int c_iInterrupt;
	static const int c_iPort;
	void initIntern();
};
//-----------------------------------------------
#endif

