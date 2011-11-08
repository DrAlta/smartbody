#include "BMLEventObject.h"

BMLEventObject::BMLEventObject() : BMLObject()
{
	setName("sbm:event");
	createStringAttribute("message", "", true, "Basic", 50, false, false, false, "Command to be sent when event is triggered.");

	createStringAttribute("stroke", "", true, "Basic", 100, false, false, false, "When this event will be triggered.");
}

BMLEventObject::~BMLEventObject()
{
}

void BMLEventObject::notify(SBSubject* subject)
{
	BMLObject::notify(subject);
}