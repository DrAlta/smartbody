#ifndef _SBSCRIPT_H_
#define _SBSCRIPT_H_

#include <sb/SBTypes.h>
#include <sb/SBObject.h>

namespace SmartBody {

class SBScript : public SBObject
{
	public:
		SBAPI SBScript();
		SBAPI ~SBScript();

		SBAPI virtual void setEnable(bool val);
		SBAPI virtual bool isEnable();

	protected:
		bool _enabled;

};

}

#endif