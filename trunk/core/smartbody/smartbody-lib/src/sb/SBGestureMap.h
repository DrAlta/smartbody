#ifndef _SBGESTUREMAP_H_
#define _SBGESTUREMAP_H_

#include <string>
#include <map>

namespace SmartBody {

class SBCharacter;

class SBGestureMap
{
	public:
		SBGestureMap();
		SBGestureMap(SBCharacter* character);
		~SBGestureMap();

		SBCharacter* getCharacter();
		SBGestureMap* copy();

		void addGestureMapping(const std::string& name, const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture);
		std::string getGestureByInfo(const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture);
		std::string getGestureByIndex(int i);
		int getNumMappings();

	public:
		struct GestureInfo
		{
			std::string _lexeme;
			std::string _type;
			std::string _hand;
			std::string _style;
			std::string _posture;
		};

	protected:
		SBCharacter* _character;
		std::map<std::string, GestureInfo> _gestureMap;
};

}

#endif