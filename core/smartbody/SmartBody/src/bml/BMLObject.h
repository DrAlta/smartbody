#ifndef _BMLOBJECT_H
#define _BMLOBJECT_H

#include "sb/SBObject.h"
#include <string>
#include <sbm/rapidxml_utils.hpp>

class BMLObject : public SmartBody::SBObject
{
	public:
		SBAPI BMLObject();
		SBAPI ~BMLObject();

		SBAPI virtual void notify(SBSubject* subject);

		SBAPI virtual void parse(rapidxml::xml_node<>* node);
		SBAPI virtual BMLObject* copy();
		SBAPI virtual void constructBML();
		SBAPI virtual std::string getBML();

	protected:
		std::string _bml;

};

#endif