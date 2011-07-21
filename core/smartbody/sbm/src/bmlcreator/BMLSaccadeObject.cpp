#include "BMLSaccadeObject.h"

BMLSaccadeObject::BMLSaccadeObject() : BMLObject()
{
	setName("saccade");
	StringAttribute* modeAttr = createStringAttribute("mode", "", true, "Basic", 20, false, false, false, "Which mode is saccade in. Default is lisen mode");
	std::vector<std::string> modes;
	modes.push_back("talk");
	modes.push_back("listen");
	modes.push_back("think");
	modeAttr->setValidValues(modes);

	createStringAttribute("angle-limit", "", true, "Basic", 10, false, false, false, "angle limit, default is 5 deg for talking mode, 4 deg for listening mode");

	StringAttribute* finishAttr = createStringAttribute("finish", "", true, "Basic", 20, false, false, false, "Turn on or off saccade");
	std::vector<std::string> toggles;
	toggles.push_back("true");
	toggles.push_back("false");
	finishAttr->setValidValues(toggles);

	createStringAttribute("sbm:duration", "", true, "Basic", 10, false, false, false, "duration, only used for explicitly define a saccade");
	createStringAttribute("direction", "", true, "Basic", 10, false, false, false, "direction on a 2D plane, valid value is from -180deg to 180deg, only used for explicitly define a saccade");
	createStringAttribute("magnitude", "", true, "Basic", 10, false, false, false, "magnitude of saccade in degree, only used for explicitly define a saccade");
}

BMLSaccadeObject::~BMLSaccadeObject()
{
}

void BMLSaccadeObject::notify(DSubject* subject)
{
	BMLObject::notify(subject);
}
