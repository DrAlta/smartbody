#include "nvbg.h"

Nvbg::~Nvbg()
{
}

bool Nvbg::execute(std::string character, std::string to, std::string messageId, std::string xml)
{
	LOG("Executing NVBG for %s %s %s %s", character.c_str(), to.c_str(), messageId.c_str(), xml.c_str());
	return true;
}

Nvbg::Nvbg()
{

}




