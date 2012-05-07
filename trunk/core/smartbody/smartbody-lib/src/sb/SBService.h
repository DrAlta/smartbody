#ifndef _SBSERVICE_H_
#define _SBSERVICE_H_

#include <sb/SBObject.h>

namespace SmartBody {

class SBSubject;

class SBService : public SBObject
{
	public:
		SBService();
		~SBService();

		virtual void setEnable(bool val);
		virtual bool isEnable();

		virtual void notify(SBSubject* subject);

	protected:
		bool _enabled;
};

}


#endif