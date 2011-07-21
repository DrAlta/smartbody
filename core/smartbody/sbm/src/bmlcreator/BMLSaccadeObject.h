#ifndef _BMLSACCADEOBJECT_
#define _BMLSACCADEOBJECT_

#include "BMLObject.h"

class BMLSaccadeObject : public BMLObject
{
	public:
		BMLSaccadeObject();
		~BMLSaccadeObject();

		virtual void notify(DSubject* subject);
};

#endif