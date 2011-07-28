#include "BMLStateObject.h"

BMLStateObject::BMLStateObject() : BMLObject()
{
	setName("sbm:states");
	StringAttribute* modeAttr = createStringAttribute("mode", "", "", "Basic", 10, false, false, false, "Mode. Either scheduling new state or updating current state, default setting is schedule");
	std::vector<std::string> modes;
	modes.push_back("schedule");
	modes.push_back("update");
	modeAttr->setValidValues(modes);
	StringAttribute* stateAttr = createStringAttribute("name", "", "", "Basic", 20, false, false, false, "State Name.");
	createStringAttribute("x", "", "", "Basic", 30, false, false, false, "Parameter X, would be used in 1D|2D|3D space.");
	createStringAttribute("y", "", "", "Basic", 30, false, false, false, "Parameter Y, would be used in 1D|2D space.");
	createStringAttribute("z", "", "", "Basic", 30, false, false, false, "Parameter Z, would be used in 3D space.");

	StringAttribute* loopAttr = createStringAttribute("loop", "", "", "Basic", 20, false, false, false, "Is it a loop mode. Only used for scheduling, default is false");
	std::vector<std::string> loops;
	loops.push_back("true");
	loops.push_back("false");
	loopAttr->setValidValues(loops);
	StringAttribute* startNowAttr = createStringAttribute("sbm:startnow", "", "", "Basic", 20, false, false, false, "Is it being scheduled immediately or after the previous state finishes. Only used for scheduling, default is true");
	std::vector<std::string> startNow;
	startNow.push_back("true");
	startNow.push_back("false");
	startNowAttr->setValidValues(startNow);
}

BMLStateObject::~BMLStateObject()
{
}

void BMLStateObject::notify(DSubject* subject)
{
	BMLObject::notify(subject);

	notifyObservers();
}
