//-----------------------------------------------
// Neobotix 
// www.neobotix.de
// Copyright (c) 2003. All rights reserved.

// author: Oliver Barth
//-----------------------------------------------
#include "CANSocket.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include "Neobotix/Compat.h"


struct can_frame frame;

//-----------------------------------------------
CANSocket::CANSocket()//const char* cIniFile)
{
	m_bInitialized = false;

	// read IniFile
	//_IniFile.SetFileName(cIniFile, "CANSocket.cpp");

	init();
}

//-----------------------------------------------
CANSocket::~CANSocket()
{
	if (m_bInitialized)
	{
		//CAN_Close(m_handle);
	}
}

//-----------------------------------------------
void CANSocket::init()
{
	//m_handle = LINUX_CAN_Open("/dev/pcan32", O_RDWR | O_NONBLOCK);
	printf("Trying to open CAN on /dev/can0 ...\n");
    //m_handle = LINUX_CAN_Open("/dev/pcan32", O_RDWR);
	m_skt = socket( PF_CAN, SOCK_RAW, CAN_RAW );
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "can0");
	int ret = ioctl(m_skt, SIOCGIFINDEX, &ifr); /* ifr.ifr_ifindex gets filled with that device's index */
	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	int ret1 = bind( m_skt, (struct sockaddr*)&addr, sizeof(addr) );
	
	if (! m_skt)
	{
		// Fatal error
		std::cout << "Cannot open CAN on USB: " << strerror(errno);
		//Sleep(3000);
		//exit(0);
	}
        else 
          {printf("success!\n");
		m_bInitialized = true;}
	
}

//-------------------------------------------
bool CANSocket::transmitMsg(CanMsg CMsg, bool bBlocking)
{
	int bytes_sent =1;
	if (m_bInitialized == false) return false;
	//char data1[8];
	//struct can_frame frame;
	frame.can_id = CMsg.m_iID;
	//std::cout << "frame id is " << frame.can_id << std::endl;
	
	for(int i=0; i<8; i++)
	{
		frame.data[i] = CMsg.getAt(i);
		//std::cout << "Msg: " << CMsg.getAt(i) << std::endl;
	}

	
	frame.can_dlc = CMsg.m_iLen;
	//frame.can_type = CMsg.m_iType;
	bool bRet = true;
	if (bBlocking)
		bytes_sent = write( m_skt, &frame, sizeof(frame) );
		//std::cout << "bblocking bytes sent " << bytes_sent << std::endl;}
	else
		bytes_sent = write( m_skt, &frame, sizeof(frame) );
		//std::cout << "not bblocking bytes sent " << bytes_sent << std::endl;}

	if( bytes_sent < 0)
	{
		std::cout << "error in CanSocket::transmitMsg: " << std::endl;//GetErrorStr(ret) << std::endl;
		bRet = false;
	}

	m_LastID = frame.data[0];

	if (bytes_sent == 0) 
		//m_bIsTXError = true;
	return !m_bIsTXError;

return bRet;

}

//-------------------------------------------
bool CANSocket::receiveMsg(CanMsg* pCMsg)
{
	frame.data[0] = 0;
	frame.data[1] = 0;
	frame.data[2] = 0;
	frame.data[3] = 0;
	frame.data[4] = 0;
	frame.data[5] = 0;
	frame.data[6] = 0;
	frame.data[7] = 0;
	//frame.msg_lost = 0;
	frame.can_id = 0;
	frame.can_dlc = 0;
	//int iRet = CAN_ERR_OK;
	bool bRet = false;
	if (m_bInitialized == false) return false;
	
	int bytes_read = read( m_skt, &frame, sizeof(frame) );
	
	if( !isObjectMode() ) 
	{
		if (bytes_read > 0) 
		{
			// message received
			pCMsg->m_iID = frame.can_id;
			//pCMsg->m_iLen = NTCANMsg.len;
			pCMsg->set(frame.data[0], frame.data[1], frame.data[2], frame.data[3],
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			bRet = true;
		}
		else
		{
			// no message, errror
			std::cout << "error in CanSocket::receiveMsg: " << std::endl;//GetErrorStr(ret) << std::endl;
				
			pCMsg->m_iID = frame.can_id;
			pCMsg->set(0,0,0,0,0,0,0,0);
	
//			std::cout << "len=" << len << "; ret = " << GetErrorStr(ret) << std::endl;
			bRet = false;
		}
	}
	else 
	{
		if (bytes_read > 0) 
		{
			// message received
			pCMsg->m_iID = frame.can_id;
			//pCMsg->m_iLen = NTCANMsg.len;
			pCMsg->set(frame.data[0], frame.data[1], frame.data[2], frame.data[3],
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			bRet = true;
		}
		else
		{
			// no message, errror
			std::cout << "error in CanSocket::receiveMsg: " << std::endl;//GetErrorStr(ret) << std::endl;		
			pCMsg->m_iID = frame.can_id;
			pCMsg->set(0,0,0,0,0,0,0,0);
//			std::cout << "len=" << len << "; ret = " << GetErrorStr(ret) << std::endl;
			bRet = false;
		}
	}
	
	return bRet;
	
}

//-------------------------------------------
bool CANSocket::receiveMsgRetry(CanMsg* pCMsg, int iNrOfRetry)
{
	int i, iRet;

	frame.can_id = 0;
	frame.can_dlc = 0;

	if (m_bInitialized == false) return false;

	// wait until msg in buffer
	bool bRet = true;
	iRet = 0;//CAN_ERR_OK;
	i=0;
	do
	{
		int iRet = read( m_skt, &frame, sizeof(frame) );

		if(iRet == 0)//CAN_ERR_OK)
			break;

		i++;
		//Sleep(100);
	}
	while(i < iNrOfRetry);

	// eval return value
	if(iRet != 0)//CAN_ERR_OK)
	{
		//LOG_APP( "CANPeakSysUSB::receiveMsgRetry, errorcode= " << nGetLastError() );
		std::cout << "error in CanSocket::receiveMsgRetry " << std::endl;
		pCMsg->set(0, 0, 0, 0, 0, 0, 0, 0);
		bRet = false;
	}
	else
	{
			pCMsg->m_iID = frame.can_id;
			pCMsg->set(frame.data[0], frame.data[1], frame.data[2], frame.data[3],
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			bRet = true;
	}

	return bRet;
}

