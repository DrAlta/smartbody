#include "BMLHeadObject.h"

BMLHeadObject::BMLHeadObject() : BMLObject()
{
	setName("head");

	SmartBody::StringAttribute* typeAttr = createStringAttribute("type", "", true, "Basic", 20, false, false, false, "type of head movement");
	std::vector<std::string> headTypes;
	headTypes.push_back("NOD");
	headTypes.push_back("SHAKE");
	headTypes.push_back("TOSS");
	headTypes.push_back("WIGGLE");
	headTypes.push_back("WAGGLE");
	typeAttr->setValidValues(headTypes);

	createDoubleAttribute("repeats", 1, true, "Basic", 30, false, false, false, "number of times a NOD or SHAKE repeats");


	createDoubleAttribute("amount", 0, true, "Basic", 70, false, false, false, "magnitude of head movement, from 0 to 1");
	createDoubleAttribute("sbm:smooth", 0, true, "Basic", 80, false, false, false, "smoothing parameter when starting and finishing head movements");
	createDoubleAttribute("sbm:period", 0, true, "Basic", 90, false, false, false, "period of nod cycle, default is .5");
	createDoubleAttribute("sbm:warp", 0, true, "Basic", 100, false, false, false, "warp parameter for wiggle and waggle");
	createDoubleAttribute("sbm:accel", 0, true, "Basic", 110, false, false, false, "acceleration parameter for wiggle and waggle");
	createDoubleAttribute("sbm:pitch", 0, true, "Basic", 120, false, false, false, "pitch parameter for wiggle and waggle");
	createDoubleAttribute("sbm:decay", 0, true, "Basic", 130, false, false, false, "decay parameter for wiggle and waggle");


	createStringAttribute("start", "", true, "Basic", 150, false, false, false, "when the head movements starts");
	createStringAttribute("ready", "", true, "Basic", 160, false, false, false, "preparation phase of head movement"); 
	createStringAttribute("stroke", "", true, "Basic", 170, false, false, false, "the mid-point of the head movement");
	createStringAttribute("relax", "", true, "Basic", 180, false, false, false, "finishing phase of head movement");
	createStringAttribute("end", "", true, "Basic", 190, false, false, false, "when the head movement ends");
}

BMLHeadObject::~BMLHeadObject()
{
}

void BMLHeadObject::notify(SBSubject* subject)
{
	BMLObject::notify(subject);
}

