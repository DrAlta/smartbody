#ifndef _SBASSETMANAGER_H_
#define _SBASSETMANAGER_H_

#include <sb/SBTypes.h>
#include <sb/SBObject.h>
#include <map>
#include <boost/filesystem/path.hpp>

namespace SmartBody {

class SBSkeleton;
class SBMotion;

class SBAssetManager : public SBObject
{
	public:
		SBAPI SBAssetManager();
		SBAPI ~SBAssetManager();

		SBAPI SBSkeleton* createSkeleton(const std::string&char_name);
		SBAPI SBSkeleton* getSkeleton(const std::string& name);
		SBAPI void addAssetPath(const std::string& type, const std::string& path);
		SBAPI std::vector<std::string> getAssetPaths(const std::string& type);
		SBAPI std::vector<std::string> getLocalAssetPaths(const std::string& type);
		SBAPI void removeAssetPath(const std::string& type, const std::string& path);
		SBAPI void removeAllAssetPaths(const std::string& type);
		SBAPI void loadAssets();
		SBAPI void loadAsset(const std::string& assetPath);
		SBAPI void loadAssetsFromPath(const std::string& assetPath);
		SBAPI void loadMotions();
		SBAPI void addMotion(SmartBody::SBMotion* motion);
		SBAPI void removeMotion(SmartBody::SBMotion* motion);
		SBAPI void addMotions(const std::string& path, bool recursive);
		SBAPI SBSkeleton* addSkeletonDefinition(const std::string& skelName);
		SBAPI SBMotion* addMotionDefinition(const std::string& motionName, double duration);
		SBAPI SBMotion* getMotion(const std::string& name);
		SBAPI int getNumMotions();
		SBAPI std::vector<std::string> getMotionNames();
		SBAPI int getNumSkeletons();
		SBAPI std::vector<std::string> getSkeletonNames();
		
		SBAPI int load_motion( const void* data, int sizeBytes, const char* motionName );


protected:
		int load_motions( const char* pathname, bool recursive );

		int load_me_motions( const char* pathname, std::map<std::string, SBMotion*>& map, bool recurse_dirs, double scale );
		int load_me_motions_impl( const boost::filesystem::path& pathname, std::map<std::string, SmartBody::SBMotion*>& map, bool recurse_dirs, double scale, const char* error_prefix);
		int load_me_motion_individual( SrInput & input, const std::string & motionName, std::map<std::string, SBMotion*>& map, double scale );


		std::map<std::string, SBSkeleton*> _skeletons;
		std::map<std::string, SBMotion*> _motions;

};

}

#endif
