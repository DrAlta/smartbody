#ifndef _SBFACESHIFTMANAGER_H_
#define _SBFACESHIFTMANAGER_H_

#include <vhcl.h>
#include <sb/SBTypes.h>
#include <sb/SBService.h>
#include <sr/sr_quat.h>
#include <string>

#ifdef WIN32
#include <WinSock2.h>
namespace SmartBody {


class SBFaceShiftManager : public SBService
{
	public:
		SBAPI SBFaceShiftManager();
		SBAPI ~SBFaceShiftManager();

		SBAPI virtual void setEnable(bool val);
		SBAPI virtual bool isEnable();
		SBAPI virtual void start();
		SBAPI virtual void stop();

		SBAPI void initConnection();
		SBAPI void stopConnection();
		SBAPI virtual void update(double time);
		SBAPI double getCoeffValue(const std::string& blendName);
		SBAPI SrQuat getHeadRotation();
	protected:
		SOCKET connectSocket;	
		std::vector<std::string> blendShapeNames;
		std::map<std::string, double> coeffTable;
		SrQuat headRotation;
};

}

#else // no support for FaceShift in non-Windows build
namespace SmartBody {


	class SBFaceShiftManager : public SBService
	{
	public:
		SBAPI SBFaceShiftManager() {}
		SBAPI ~SBFaceShiftManager() {}

		SBAPI virtual void setEnable(bool val) {}
		SBAPI virtual bool isEnable() {}

		SBAPI void initConnection() {}
		SBAPI void stopConnection() {}			
	};

}
#endif

#endif