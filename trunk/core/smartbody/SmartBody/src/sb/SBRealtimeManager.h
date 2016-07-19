#ifndef _SBREALTIMEMANAGER_H_
#define _SBREALTIMEMANAGER_H_

#include <vhcl.h>
#include <sb/SBTypes.h>
#include <sb/SBService.h>
#include <sr/sr_quat.h>
#include <string>
#include <sb/SBSubject.h>

#if defined(LINUX_BUILD) || defined(MAC_BUILD) || defined(IPHONE_BUILD) || defined(ANDROID_BUILD) || defined(FLASH_BUILD)
#elif defined (WIN_BUILD)
#define USE_PERCEPTIONNEURON 0
#endif





#if USE_PERCEPTIONNEURON > 0
#include <Windows.h>
#include "NeuronDataReader.h"
//#include "SocketCommand.h"
#endif

namespace SmartBody {


class SBRealtimeManager : public SBService
{
	public:
		SBAPI SBRealtimeManager();
		SBAPI ~SBRealtimeManager();

		SBAPI virtual void setEnable(bool val);
		SBAPI virtual bool isEnable();
		SBAPI virtual void start();
		SBAPI virtual void stop();

		SBAPI virtual void setChannelNames(const std::string& names);
		SBAPI std::vector<std::string>& getChannelNames();
		
		SBAPI void notify(SBSubject* subject);

		SBAPI void initConnection();
		SBAPI void stopConnection();
		SBAPI virtual void update(double time);
		SBAPI void setData(const std::string& channel, const std::string& data);
		SBAPI std::string getData(const std::string& channel);
		SBAPI double getDataDouble(const std::string& channel);
		SBAPI int getDataInt(const std::string& channel);
		SBAPI SrVec getDataVec(const std::string& channel);
		SBAPI SrQuat getDataQuat(const std::string& channel);
		SBAPI SrMat getDataMat(const std::string& channel);

#if USE_PERCEPTIONNEURON > 0
		void startPerceptionNeuron();
		void restartPerceptionNeuron();
		void stopPerceptionNeuron();
		bool isPerceptionNeuronRunning();

		static void CALLBACK myFrameDataReceived(void* customedObj, SOCKET_REF sender, BvhDataHeader* header, float* data);
		static void CALLBACK myCalculationDataReceived(void* customedObj, SOCKET_REF sender, CalcDataHeader* pack, float* data);
		static void CALLBACK mySocketStatusChanged(void* customedObj, SOCKET_REF sender, SocketStatus status, char* message);
#endif

	protected:
		std::vector<std::string> blendShapeNames;
		std::map<std::string, std::string> channelTable;
		std::vector<std::string> channelNames;
#if USE_PERCEPTIONNEURON > 0
		SOCKET_REF m_sockTCPRef;
		SOCKET_REF m_sockUDPRef;
		float* _valuesBuffer;
		int _valuesBufferLength;
		int _frameCount;
		std::map<int, std::string> _dataIndexMap;
#endif
};

}

#endif
