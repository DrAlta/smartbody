#ifndef _SBCONTROLLER_
#define _SBCONTROLLER_

#include "controllers/me_controller.h"

namespace SmartBody {

class SBController : public MeController
{
	public:
		SBController();
		SBController(const SBController& controller);
		~SBController();

		const std::string& getType();

		SBController* getParent();			// how to get parent?
		SBController* getChild(int index);
		int getNumChildren();
		void addChannel(std::string jointName, std::string channelName);

		const std::string& getCharacterName();

		void setIgnore(bool val);
		bool isIgnore();
		void setDebug(bool val);
		bool isDebug();

		void printInfo();
		void startRecordSkm(int maxFrame);
		void startRecordBvh(int maxFrame);
		void writeRecord(std::string prefix);
		void stopRecord();
		void clearRecord();
		double getDuration();

		void setTiming(float indt, float outdt, float empht);

		virtual bool controller_evaluate ( double t, MeFrameData& frame ) { return true;}
		virtual double controller_duration () { return 0.0; }
		virtual SkChannelArray& controller_channels () { return channelArray; }
		virtual const std::string& controller_type () const { return controllerType; }

	private:
		SkChannelArray channelArray;
		std::string controllerType;


};

}

#endif