#ifndef _SBSCRIPT_H_
#define _SBSCRIPT_H_

#include <sbm/SBObject.h>

namespace SmartBody {

class SBScript : public SBObject
{
	public:
		SBScript();
		~SBScript();

		virtual void setEnable(bool val);
		virtual bool isEnable();

	protected:
		bool _enabled;

};

}

#endif