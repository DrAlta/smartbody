#ifndef _SBSCRIPT_H_
#define _SBSCRIPT_H_

#include <sbm/SBObject.h>

namespace SmartBody {

class SBScript : public SBObject
{
	public:
		SBScript();
		~SBScript();

		virtual void start();
		virtual void beforeUpdate();
		virtual void update(double time);
		virtual void afterUpdate();
		virtual void stop();

};

}

#endif