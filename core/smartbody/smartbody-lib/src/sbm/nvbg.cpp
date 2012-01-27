#include "nvbg.h"

Nvbg::Nvbg()
{

}

Nvbg::~Nvbg()
{
}

void Nvbg::objectEvent(std::string character, std::string name, bool isAnimate, SrVec position, SrVec velocity, SrVec relativePosition, SrVec relativeVelocity)
{
	LOG("Object event for %s from %s", character.c_str(), name.c_str());
}

bool Nvbg::execute(std::string character, std::string to, std::string messageId, std::string xml)
{
	LOG("Executing NVBG for %s %s %s %s", character.c_str(), to.c_str(), messageId.c_str(), xml.c_str());
	return true;
}

void Nvbg::notify(SmartBody::SBSubject* subject)
{
	SmartBody::SBObject::notify(subject);
}







