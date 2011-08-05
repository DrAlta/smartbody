#ifndef _NVBG_H
#define _NVBG_H

#include "vhcl.h"
#include <sbm/DObject.h>
#include <string>

struct Nvbg 
{
		virtual ~Nvbg();

		virtual bool execute(std::string character, std::string to, std::string messageId, std::string xml);
};


#endif