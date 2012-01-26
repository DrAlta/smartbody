#ifndef _NVBG_H
#define _NVBG_H

#include "vhcl.h"
#include <sbm/SBObject.h>
#include <string>

class Nvbg : public SmartBody::SBObject
{
public:
		Nvbg();
		virtual ~Nvbg();

		virtual void objectEvent(std::string character, std::string name, bool isAnimate, SrVec position, SrVec velocity, SrVec relativePosition, SrVec relativeVelocity);
		virtual bool execute(std::string character, std::string to, std::string messageId, std::string xml);
		virtual void notify(SmartBody::SBSubject* subject);

};


#endif