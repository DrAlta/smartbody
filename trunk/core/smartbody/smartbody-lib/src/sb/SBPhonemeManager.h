#ifndef _SBPHONEMEMANAGER_H_
#define _SBPHONEMEMANAGER_H_

#include <vector>
#include <string>
#include <map>

namespace SmartBody{

class SBDiphone;

class SBDiphoneManager
{
public:
	SBDiphoneManager();
	~SBDiphoneManager();

	std::vector<std::string> getCommonPhonemes();
	
	SBDiphone* createDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name);
	std::vector<SBDiphone*>& getDiphones(const std::string& name);
	SBDiphone* getDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name);
	std::vector<std::string> getDiphoneMapNames();
	int getNumDiphoneMap();
	int getNumDiphones(const std::string& name);

protected:
	std::map<std::string, std::vector<SBDiphone*> > _diphoneMap;
};

}
#endif
