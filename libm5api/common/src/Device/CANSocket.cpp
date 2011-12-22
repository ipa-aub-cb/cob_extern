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

// #define DEBUG 1

//#include "Neobotix/Compat.h"


struct can_frame frame;

//-----------------------------------------------
CANSocket::CANSocket() : m_hDevice(0), m_hSyncDevice(0), m_iDeviceId(-1), m_uiBaudRate(0), m_uiQueueSize(128), m_uiTimeOut(3)		// ESD C331 //const char* cIniFile)
{
	initMessage("CANSocket", g_iDebugLevel, g_bDebug, g_bDebugFile);
	m_DeviceName = (char*) malloc(200 * sizeof(char));
	memset(m_DeviceName,0,sizeof(m_DeviceName));
	m_bInitialized = false;

}

//-----------------------------------------------
CANSocket::~CANSocket()
{
	if (m_bInitialized)
	{
		//CAN_Close(m_handle);
	}
}
int CANSocket::init()
{
return init(0x014);//CAN_BAUD_250K);
}
//-----------------------------------------------
int CANSocket::init(const char* acInitString)
{
#ifdef DEBUG
	std::cout << "Trying to open CAN on can0 ...\n"<< std::endl;
#endif
	
	m_skt = socket( PF_CAN, SOCK_RAW, CAN_RAW );
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "can0");
	int ret = ioctl(m_skt, SIOCGIFINDEX, &ifr); /* ifr.ifr_ifindex gets filled with that device's index */
	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	int ret1 = bind( m_skt, (struct sockaddr*)&addr, sizeof(addr) );
	m_iErrorState = 0;
	m_bInitFlag = true;

	updateModuleIdMap();

#ifdef DEBUG
	std::cout << "finished updateModuleIdMap "<< std::endl;
#endif
	
	if (! m_skt)
	{
		// Fatal error
		std::cout << "Cannot open CAN on USB: " << strerror(errno)<< std::endl;
	}
        else 
          {
			printf("Open CAN on USB suceeded!\n");
			m_bInitialized = true;
		  }
		  
#ifdef DEBUG
	std::cout << "m_iErrorState before returning of CANSocket::init(const char* acInitString): "<< m_iErrorState;	
#endif

	return m_iErrorState;
}

//-------------------------------------------

int CANSocket::init(unsigned long baudRate)
{
	return m_iErrorState;
}

//-------------------------------------------
int CANSocket::writeDevice(CProtocolMessage& rclProtocolMessage)
{
#ifdef DEBUG
	std::cout << "CANSocket::writeDevice "<< std::endl;
#endif
	
	int bytes_sent =-1;
#ifdef DEBUG
	std::cout << "m_skt " << m_skt << std::endl;
#endif	
	if (m_bInitialized == false) return false;
	
	frame.can_dlc = (int)rclProtocolMessage.m_ucMessageLength;
	
#ifdef DEBUG
	std::cout << "frame can_ldc is " << frame.can_dlc << std::endl;
	std::cout << "clProtocolMessage.m_ucMessageLength: " << (unsigned int)rclProtocolMessage.m_ucMessageLength << std::endl;
#endif	

	frame.can_id = rclProtocolMessage.m_uiMessageId;

#ifdef DEBUG
	std::cout << "frame can_id is " << frame.can_id << std::endl;
	std::cout << "rclProtocolMessage.m_uiMessageId: " << rclProtocolMessage.m_uiMessageId << std::endl;
#endif
	
	// copy data from ProtocolMessage to frame.data (byte-wise or string)
	
	//memcpy(frame.data, "", 2);
	//memcpy(frame.data, rclProtocolMessage.m_aucMessageData, rclProtocolMessage.m_ucMessageLength);
	
	for(int i=0; i<(unsigned int)rclProtocolMessage.m_ucMessageLength; i++)
	{
		
		frame.data[i] = rclProtocolMessage.m_aucMessageData[i];
		#ifdef DEBUG
			std::cout << "rclProtocolMessage.m_aucMessageData[i] " << rclProtocolMessage.m_aucMessageData[i] << std::endl;
			std::cout << "frame data is " << frame.data << std::endl;
			std::cout << "rclProtocolMessage.m_aucMessageData: " << rclProtocolMessage.m_aucMessageData << std::endl;
		#endif
	}
#ifdef DEBUG	
	std::cout << "sizeof frame : " << sizeof(frame) << std::endl;
	std::cout << "start write:" << std::endl;
#endif
	
	int bRet = 1;
	
	//writing
	bytes_sent = write( m_skt, &frame, sizeof(frame));

	if( bytes_sent < 0)
	{
	#ifdef DEBUG
		std::cout << "error in CanSocket::transmitMsg: bytes sent <0" << std::endl;
	#endif
		bRet = 1;
	}

	
	if ( bytes_sent > 0) 
	{
	bRet =0;
#ifdef DEBUG
	std::cout << "bytes gesendet in CanSocket::transmitMsg:  " << bytes_sent << std::endl;
#endif
}

#ifdef DEBUG
	std::cout << "bRet: " << bRet << std::endl;
#endif

return bRet;

}

//-------------------------------------------


int CANSocket::readDevice(CProtocolMessage& rclProtocolMessage)
{
	// set frame to zero
	
	frame.data[0] = 0;
	frame.data[1] = 0;
	frame.data[2] = 0;
	frame.data[3] = 0;
	frame.data[4] = 0;
	frame.data[5] = 0;
	frame.data[6] = 0;
	frame.data[7] = 0;
	frame.can_dlc = 8;	//rclProtocolMessage.m_ucMessageLength;
	frame.can_id = 0; //rclProtocolMessage.m_uiMessageId;
	
	int bRet = 1;
	
	//read
	int bytes_read = read( m_skt, &frame, sizeof(frame) );
	
#ifdef DEBUG
	std::cerr << "sizeof(frame)" << sizeof(frame) << std::endl;
#endif
	
	
	if( !isObjectMode() ) 
	{
		if (bytes_read > 0)  // no problem occured
		{
			
			#ifdef DEBUG
				std::cout << "bytes read: " << bytes_read << std::endl;
				std::cout << "data: " << frame.data << std::endl;
				std::cout << "ID: " << frame.can_id << std::endl;
				std::cout << "length: " << frame.can_dlc << std::endl;
			#endif
		//  copy read data from frame to ProtocolMessage (byte-wise or string)
		
		//	frame.data[1], frame.data[2], frame.data[3],frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			
			rclProtocolMessage.m_uiMessageId = frame.can_id;
			rclProtocolMessage.m_ucMessageLength = frame.can_dlc;
			memcpy(rclProtocolMessage.m_aucMessageData, frame.data, rclProtocolMessage.m_ucMessageLength);
						
			bRet = 0;
		}
		else 	// reading problem occured
		{
			// no message, errror
			std::cout << "error in CanSocket::receiveMsg: " << std::endl;//GetErrorStr(ret) << std::endl;
			bRet = 1;
		}
	}

#ifdef DEBUG
	std::cout << "bRet before return from CANSocket::readDevice: " << bRet << std::endl;
#endif	
	return bRet;

}


int CANSocket::reinit(unsigned char ucBaudRateId)
{
	return m_iErrorState;
}

//-------------------------------------------

bool CANSocket::receiveMsgRetry(CanMsg* pCMsg, int iNrOfRetry)
{
	can_frame frame;
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


int CANSocket::exit()
{
	return m_iErrorState;
}


int CANSocket::waitForStartMotionAll()
{
	return m_iErrorState;
}


void CANSocket::setQueueSize(unsigned short uiQueueSize)
{
	m_uiQueueSize = uiQueueSize;
}


void CANSocket::setTimeOut(unsigned long uiTimeOut)
{
	m_uiTimeOut= uiTimeOut;
}

int CANSocket::getDeviceError(int iErrorState)
{
	return m_iErrorState;
}


int CANSocket::setBaudRate(unsigned char iBaudRate)
{
	m_uiBaudRate = (unsigned long) iBaudRate;
	return setBaudRate();
}


int CANSocket::setBaudRate()
{

	return m_iErrorState;
}


int CANSocket::setMessageId(unsigned long uiMessageId)
{

	return m_iErrorState;
}


int CANSocket::clearReadQueue()
{
	can_frame frame;
    frame.can_dlc = 8;
	frame.can_id = 0;
	return m_iErrorState;
}
