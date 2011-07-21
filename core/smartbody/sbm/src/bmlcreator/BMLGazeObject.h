#ifndef _BMLGAZEOBJECT_
#define _BMLGAZEOBJECT_

#include "BMLObject.h"

class BMLGazeObject : public BMLObject
{
	public:
		BMLGazeObject();
		~BMLGazeObject();

		virtual void notify(DSubject* subject);

};
#endif