#ifndef _SBGESTUREMAP_H_
#define _SBGESTUREMAP_H_

#include <string>
#include <vector>
#include <sb/SBObject.h>

namespace SmartBody {

class SBGestureMap : public SBObject 
{
	public:
		struct GestureInfo
		{
			std::string _animation;
			std::string _lexeme;
			std::string _type;
			std::string _hand;
			std::string _style;
			std::string _posture;
		};

	public:
		SBGestureMap();
		SBGestureMap(const std::string& name);
		~SBGestureMap();

		SBGestureMap* copy();

		void addGestureMapping(const std::string& name, const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture);
		std::string getGestureByInfo(const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture, const std::string& policy);
		GestureInfo& getGestureByIndex(int i);
		int getNumMappings();
		void validate();

	private:
		bool gestureInfoCompare(const std::string& glexeme, const std::string& gtype, const std::string& ghand, const std::string& gstyle, const std::string& gposture,
								const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture, 
								std::string compType);

	protected:
		std::vector<GestureInfo> _gestureMaps;
		GestureInfo defaultGestureInfo;
};

}

#endif