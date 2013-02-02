#ifndef _SBRETARGET_H_
#define _SBRETARGET_H_

#include <sb/SBTypes.h>
#include <sr/sr_quat.h>
#include <string>
#include <vector>
#include <map>

namespace SmartBody {

typedef std::pair<SrQuat,SrQuat> QuatPair;
class SBRetarget
{
	public:
		SBAPI SBRetarget();
		SBAPI SBRetarget(std::string srcName, std::string tgtName);		
		SBAPI ~SBRetarget();	
		SBAPI bool initRetarget(std::vector<std::string>& endJoints, std::vector<std::string>& relativeJoints);
		SBAPI SrQuat applyRetargetJointRotation(std::string jointName, SrQuat& inQuat);
		SBAPI SrQuat applyRetargetJointRotationInverse(std::string jointName, SrQuat& inQuat);
		SBAPI float  applyRetargetJointTranslation(std::string jointName, float inPos);
	protected:
		std::string srcSkName;
		std::string tgtSkName;
		std::map<std::string, QuatPair> jointPrePostRotMap;
		float heightRatio;
};

}


#endif