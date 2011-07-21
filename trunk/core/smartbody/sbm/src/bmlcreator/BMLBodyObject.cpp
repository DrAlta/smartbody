#include "BMLBodyObject.h"

BMLBodyObject::BMLBodyObject() : BMLObject()
{
	setName("body");

	DAttribute* postureAttr = createStringAttribute("posture", "", true, "Basic", 20, false, false, false, "Motion name of the idle posture");

	DAttribute* startAttr = createStringAttribute("start", "", true, "Basic", 100, false, false, false, "When the new posture will be activated and begin blending with the existing posture");
	DAttribute* readyAttr = createStringAttribute("ready", "", true, "Basic", 110, false, false, false, "When the new posture will be fully blended in and the old posture will be gone ");
}

BMLBodyObject::~BMLBodyObject()
{
}

void BMLBodyObject::notify(DSubject* subject)
{
	BMLObject::notify(subject);

	notifyObservers();
}
