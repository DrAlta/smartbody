#include "SBFaceShiftManager.h"
#include "external/faceshift/fsbinarystream.h"
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBJoint.h>
#include <sb/SBSimulationManager.h>
#include <vhcl.h>
#include <iostream>
#include <sstream>

#if WIN32
#include <WS2tcpip.h>
#include <tchar.h>
#include <Windows.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "33433"
#define DEFAULT_IPADDR "127.0.0.1"
#define DEFAULT_BUFLEN 1024

namespace SmartBody {

SBFaceShiftManager::SBFaceShiftManager() : SBService()
{
	setEnable(false);
	setName("FaceShift");
	connectSocket = INVALID_SOCKET;
	createIntAttribute("defaultPort", 33433, true, "Basic", 60, false, false, false, "Port No. for connection with FaceShift software.");
	createStringAttribute("targetCharacter", "", true, "Basic", 60, false, false, false, "Target character to set the FaceShift results.");
}

SBFaceShiftManager::~SBFaceShiftManager()
{
	
}

SBAPI  void SBFaceShiftManager::setEnable( bool val )
{	
	SBService::setEnable(val);	
	if (val)
		start();
	else
		stop();
}

SBAPI  bool SBFaceShiftManager::isEnable()
{
	return SBService::isEnable();
}

SBAPI  void SBFaceShiftManager::start()
{
	initConnection();
}

SBAPI  void SBFaceShiftManager::stop()
{
	stopConnection();
}


SBAPI  void SBFaceShiftManager::update( double time )
{
	int iResult;
	fs::fsBinaryStream parserIn, parserOut;
	fs::fsMsgPtr msg;
	static char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	SmartBody::SBCharacter* sbChar = SmartBody::SBScene::getScene()->getCharacter(getStringAttribute("targetCharacter"));

	static int blendShapeIds[] = { 18, 19, 20, 25, 26, 31, 32 };
	int numBlendID = 7;
	iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
	if ( iResult > 0 ){
		parserIn.received(iResult, recvbuf);
		while(msg=parserIn.get_message()) 
		{
#if 0
			if (dynamic_cast<fs::fsMsgBlendshapeNames*>(msg.get()))
			{
				// has the marker name
				if (sbChar)
				{
					LOG("has blendshape names");
					fs::fsMsgBlendshapeNames* bs = dynamic_cast<fs::fsMsgBlendshapeNames*>(msg.get());
					std::vector<std::string>& bnames = bs->blendshape_names();					
					SmartBody::SBSkeleton* skel = sbChar->getSkeleton();
					if (blendShapeNames.size() != bnames.size())
						blendShapeNames = bnames;
					for (unsigned int i=0;i<bnames.size();i++)
					{
						LOG("blendShape name = %s", bnames[i].c_str());
						if (!skel->getJointByName(bnames[i]))
						{
							sbChar->addBlendShapeChannel(bnames[i]);
						}
					}
				}
			}
#endif
			
			
			if(dynamic_cast<fs::fsMsgTrackingState*>(msg.get())) 
			{
				
				fs::fsMsgTrackingState *ts = dynamic_cast<fs::fsMsgTrackingState*>(msg.get());
				const fs::fsTrackingData &data = ts->tracking_data();
				
				if (sbChar)
				{
					//LOG("check tracking state");
					SmartBody::SBSkeleton* skel = sbChar->getSkeleton();
					//for (unsigned int i=0;i<blendShapeNames.size();i++)
					//LOG("coeff size = %d", data.m_coeffs.size());
					for (unsigned int i=0;i<data.m_coeffs.size();i++)
					{
						std::string blendName = boost::lexical_cast<std::string>(i+1);						
						coeffTable[blendName] = data.m_coeffs[i];						
					}
					headRotation = SrQuat(data.m_headRotation.w, data.m_headRotation.x, data.m_headRotation.y, data.m_headRotation.z);				
				}
				// Do something with the Tracking Data (change controllers in DAZ or record into the timeline, depending 
				// on the state of the plugin
				//LOG("head rotation = %f %f %f %f",data.m_headRotation.w, data.m_headRotation.x, data.m_headRotation.y, data.m_headRotation.z);
				//LOG("head translation: %f %f %f",data.m_headTranslation.x,data.m_headTranslation.y,data.m_headTranslation.z);						
			} 
		}
	}
	if(!parserIn.valid()) {
		printf("parser in invalid state\n");
		parserIn.clear();
	}
}

SBAPI void SBFaceShiftManager::initConnection()
{
	int err = 0;
	WSADATA wsaData;
	WORD wVersionRequested;
	
	struct sockaddr_in clientService;
	
	int iResult;	
	wVersionRequested = MAKEWORD(2,2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return;
	}


	// Create a SOCKET for connecting to server
	connectSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET) {	
		WSACleanup();
		return;
	}

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	clientService.sin_port = htons( 33433 );


	iResult = connect( connectSocket, (SOCKADDR*) &clientService, sizeof(clientService));
	if ( iResult == SOCKET_ERROR) {
		closesocket (connectSocket);
		WSACleanup();
		return;
	}

	if (connectSocket == INVALID_SOCKET) {
		closesocket(connectSocket);
		WSACleanup();
	}
}

SBAPI void SBFaceShiftManager::stopConnection()
{
	closesocket(connectSocket);
	WSACleanup();
}

SBAPI double SBFaceShiftManager::getCoeffValue( const std::string& blendName )
{
	double coeff = 0.0;
	if (coeffTable.find(blendName) != coeffTable.end())
	{
		coeff = coeffTable[blendName];
	}
	if (coeff < 0.0) coeff = 0.0;
	return coeff;
}

SBAPI SrQuat SBFaceShiftManager::getHeadRotation()
{
	return headRotation;
}
}

#endif

