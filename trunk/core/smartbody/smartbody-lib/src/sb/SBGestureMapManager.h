#ifndef _SBGESTUREMAPMANAGER_H_
#define _SBGESTUREMAPMANAGER_H_

#include <string>
#include <map>
namespace SmartBody {

class SBGestureMap;

class SBGestureMapManager
{
	public:
		SBGestureMapManager();
		~SBGestureMapManager();

		SBGestureMap* createGestureMap(std::string characterName);
		void removeGestureMap(std::string characterName);
		int getNumGestureMaps();
		SBGestureMap* getGestureMap(std::string characterName);

	protected:
		std::map<std::string, SBGestureMap*> _gestureMaps;
};

}
#endif