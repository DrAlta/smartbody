#include "BMLGestureObject.h"

BMLGestureObject::BMLGestureObject() : BMLObject()
{
	setName("gesture");

	// TODO: align the bml specification
/*
	std::vector<std::string> gestures;
	gestures.push_back("POINT");
	gestures.push_back("REACH");
	gestures.push_back("BEAT");
	gestures.push_back("DEPICT");
	gestures.push_back("SIGNAL");
*/
	SmartBody::StringAttribute* typeAttr = createStringAttribute("lexeme", "", "", "Basic", 50, false, false, false, "Type of gesture");
//	typeAttr->setValidValues(gestures);

	createStringAttribute("name", "", "", "Basic", 60, false, false, false, "Name of the gesture for DEPICT or SIGNAL gestures.");

	std::vector<std::string> modes;
	modes.push_back("LEFT_HAND");
	modes.push_back("RIGHT_HAND");
	modes.push_back("BOTH_HANDS");
	SmartBody::StringAttribute* modeAttr = createStringAttribute("mode", "", "", "Basic", 70, false, false, false, "Which hand is involved. Should be one of the following: left, right, both.");
	modeAttr->setValidValues(modes);

	createStringAttribute("sbm:style", "", "", "Basic", 80, false, false, false, "Style of the gestures. Used to differentiate gestures with same type, posture and mode.");

	createStringAttribute("target", "", "", "Basic", 90, false, false, false, "Target for POINT and REACH gestures.");
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

