#ifndef _SBSERVICE_H_
#define _SBSERVICE_H_

#include <sbm/SBObject.h>

namespace SmartBody {

class SBService : public SBObject
{
	public:
		SBService();
		~SBService();

		virtual void setEnable(bool val);
		virtual bool isEnable();

	protected:
		bool _enabled;
};

}


#endif