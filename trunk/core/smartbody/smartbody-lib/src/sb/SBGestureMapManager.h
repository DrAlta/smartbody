#ifndef _SBGESTUREMAPMANAGER_H_
#define _SBGESTUREMAPMANAGER_H_

#include <string>
#include <map>
#include <vector>

namespace SmartBody {

class SBGestureMap;

class SBGestureMapManager
{
	public:
		SBGestureMapManager();
		~SBGestureMapManager();

		SBGestureMap* createGestureMap(std::string gestureName);
		void removeGestureMap(std::string gestureName);
		int getNumGestureMaps();
		std::vector<std::string>& getGestureMapNames();
		SBGestureMap* getGestureMap(std::string gestureName);

	protected:
		std::map<std::string, SBGestureMap*> _gestureMaps;
};

}
#endif