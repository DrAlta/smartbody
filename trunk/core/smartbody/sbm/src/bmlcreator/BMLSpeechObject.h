#ifndef _BMLSPEECHOBJECT_
#define _BMLSPEECHOBJECT_

#include "BMLObject.h"

class BMLSpeechObject : public BMLObject
{
	public:
		BMLSpeechObject();
		~BMLSpeechObject();

		virtual std::string getBML();

		virtual void notify(DSubject* subject);

	protected:
		StringAttribute* typeAttr;
		StringAttribute* refAttr;
		StringAttribute* contentAttr;
};
#endif