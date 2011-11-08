#include "BMLHeadObject.h"

BMLHeadObject::BMLHeadObject() : BMLObject()
{
	setName("head");

	SmartBody::StringAttribute* typeAttr = createStringAttribute("type", "", true, "Basic", 20, false, false, false, "type of head movement");
	std::vector<std::string> headTypes;
	headTypes.push_back("NOD");
	headTypes.push_back("SHAKE");
	headTypes.push_back("TOSS");
	headTypes.push_back("ORIENT");
	headTypes.push_back("WIGGLE");
	headTypes.push_back("WAGGLE");
	typeAttr->setValidValues(headTypes);

	createDoubleAttribute("repeats", 1, true, "Basic", 30, false, false, false, "number of times a NOD or SHAKE repeats");
	

	createStringAttribute("direction", "", true, "Basic", 40, false, false, false, "?");
	createStringAttribute("target", "", true, "Basic", 50, false, false, false, "?");
	createDoubleAttribute("angle", 0, true, "Basic", 60, false, false, false, "?");


	createDoubleAttribute("amount", 0, true, "Basic", 70, false, false, false, "?");
	createDoubleAttribute("sbm:smooth", 0, true, "Basic", 80, false, false, false, "?");
	createDoubleAttribute("sbm:period", 0, true, "Basic", 90, false, false, false, "?");
	createDoubleAttribute("sbm:warp", 0, true, "Basic", 100, false, false, false, "?");
	createDoubleAttribute("sbm:accel", 0, true, "Basic", 110, false, false, false, "?");
	createDoubleAttribute("sbm:pitch", 0, true, "Basic", 120, false, false, false, "?");
	createDoubleAttribute("sbm:decay", 0, true, "Basic", 130, false, false, false, "?");


	createStringAttribute("start", "", true, "Basic", 150, false, false, false);
	createStringAttribute("ready", "", true, "Basic", 160, false, false, false);
	createStringAttribute("stroke", "", true, "Basic", 170, false, false, false);
	createStringAttribute("relax", "", true, "Basic", 180, false, false, false);
	createStringAttribute("end", "", true, "Basic", 190, false, false, false);
}

BMLHeadObject::~BMLHeadObject()
{
}

void BMLHeadObject::notify(SBSubject* subject)
{
	BMLObject::notify(subject);
}

