#ifndef _SBJOINTMAP_H_
#define _SBJOINTMAP_H_

#include <map>
#include <vector>
#include <string>
#include <sk/sk_joint.h>

namespace SmartBody {

class SBMotion;
class SBSkeleton;
class SKJoint;

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

		// Automatic joint name matching to standard SmartBody names
		bool guessMapping(SmartBody::SBSkeleton* skeleton);


	private:
		std::vector<std::pair<std::string, std::string> > _map;

	protected:

		// get joint index from joint array (linear search)
		int getJointIndex(SkJoint* j);

		// get joint hierachy level (how many levels below j_top), use root() as j_top if not specified
		int getJointHierarchyLevel(SkJoint* j, SkJoint* j_top=0);

		// count all children joints below given joint in the hierachy (recursive)
		int countChildren(SkJoint* j);

		// count given char in string
		int countChar(const char* string, char c, bool isCaseSensitive);

		// guess which joint is left/right by counting letters in joint names (ja, jb)
		void guessLeftRightFromJntNames(SkJoint* ja, SkJoint* jb, SkJoint*& l_j, SkJoint*& r_j);

		// push all children joints into given list, make sure list is empty! (recursive)
		void listChildrenJoints(SkJoint* j, std::vector<SkJoint*>& j_list);
		// get joint with deepest hierachy level from list (first one found if multiple)
		SkJoint* getDeepestLevelJoint(const std::vector<SkJoint*>& j_list);

		void setJointMap(const char* SB_jnt, SkJoint* j);
};

}


#endif
