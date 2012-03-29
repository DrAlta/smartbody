#ifndef _SBJOINTMAP_H_
#define _SBJOINTMAP_H_

#include <map>
#include <vector>

namespace SmartBody {

class SBMotion;
class SBSkeleton;

class SBJointMap
{
	public:
		SBJointMap();
		~SBJointMap();

		void applyMotion(SmartBody::SBMotion* motion);
		void applySkeleton(SmartBody::SBSkeleton* skeleton);

		void setMapping(const std::string& from, const std::string& to);
		void removeMapping(const std::string& from);

		std::string getMapTarget(const std::string& to);
		std::string getMapSource(const std::string& from);

		int getNumMappings();
		std::string getTarget(int num);
		std::string getSource(int num);


	private:
		std::vector<std::pair<std::string, std::string> > _map;
};

}


#endif