#ifndef _SBGESTUREMAP_H_
#define _SBGESTUREMAP_H_

#include <string>
#include <map>

namespace SmartBody {

class SBCharacter;

class SBGestureInfo;
class SBGestureMap
{
	public:
		SBGestureMap();
		SBGestureMap(SBCharacter* character);
		~SBGestureMap();

		SBCharacter* getCharacter();
		SBGestureMap* copy();

		void addGestureMapping(const std::string& name, const std::string& type, const std::string& posture, const std::string& hand);
		std::string getGestureByInfo(const std::string& type, const std::string& posture, const std::string& hand);
		std::string getGestureByIndex(int i);
		int getNumMappings();

		std::string getGestureType(const std::string& name);
		std::string getGestureHand(const std::string& name);
		std::string getGesturePosture(const std::string& name);

	protected:
		SBCharacter* _character;
		std::map<std::string, SBGestureInfo*> _gestureMap;
};

class SBGestureInfo
{
	public:
		SBGestureInfo(const std::string& type, const std::string& posture, const std::string& hand);
		~SBGestureInfo();

		void setType(const std::string& type);
		const std::string& getType();
		void setPosture(const std::string& posture);
		const std::string& getPosture();
		void setHand(const std::string& hand);
		const std::string& getHand();

		bool matchingAll(const std::string& type, const std::string& posture, const std::string& hand);
		
	private:
		std::string _type;
		std::string _posture;
		std::string _hand;
};

}

#endif