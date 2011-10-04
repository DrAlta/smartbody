#ifndef _BMLOBJECT_H
#define _BMLOBJECT_H

#include "sbm/DObject.h"
#include <string>

class BMLObject : public DObject
{
	public:
		BMLObject();
		~BMLObject();

		virtual void notify(DSubject* subject);

		virtual void constructBML();
		virtual std::string getBML();

	protected:
		std::string _bml;

};

#endif