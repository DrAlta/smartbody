#include "BMLGestureObject.h"

BMLGestureObject::BMLGestureObject() : BMLObject()
{
	setName("gesture");

	std::vector<std::string> gestures;
	gestures.push_back("POINT");
	gestures.push_back("REACH");
	gestures.push_back("BEAT");
	gestures.push_back("DEPICT");
	gestures.push_back("SIGNAL");
	SmartBody::StringAttribute* typeAttr = createStringAttribute("type", "", "", "Basic", 50, false, false, false, "Type of gesture");
	typeAttr->setValidValues(gestures);


	createStringAttribute("name", "", "", "Basic", 60, false, false, false, "Name of the gesture for DEPICT or SIGNAL gestures.");
	createStringAttribute("target", "", "", "Basic", 70, false, false, false, "Target for POINT and REACH gestures.");

	createStringAttribute("start", "", "", "Basic", 100, false, false, false, "Time when gesture starts.");
	createStringAttribute("ready", "", "", "Basic", 110, false, false, false, "Time when gesture is fully blended in.");
	createStringAttribute("stroke", "", "", "Basic", 120, false, false, false, "Time of gesture's stroke.");
	createStringAttribute("relax", "", "", "Basic", 130, false, false, false, "Time when gesture begins to blend out.");
	createStringAttribute("end", "", "", "Basic", 140, false, false, false, "Time when gesture is fully blended out.");
}

BMLGestureObject::~BMLGestureObject()
{
}

void BMLGestureObject::notify(SBSubject* subject)
{
	BMLObject::notify(subject);
}

