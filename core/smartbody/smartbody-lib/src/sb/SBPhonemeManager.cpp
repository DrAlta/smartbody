#include <sb/SBPhonemeManager.h>
#include <sb/SBPhoneme.h>
#include <algorithm>
#include <vhcl.h>

namespace SmartBody {

SBDiphoneManager::SBDiphoneManager()
{
}

SBDiphoneManager::~SBDiphoneManager()
{
	std::map<std::string, std::vector<SBDiphone*> >::iterator iter = _diphoneMap.begin();
	for (; iter != _diphoneMap.end(); iter++)
	{
		std::vector<SBDiphone*>& diphones = getDiphones(iter->first);
		for (size_t i = 0; i < diphones.size(); i++)
		{
			delete diphones[i];
			diphones[i] = NULL;
		}
		diphones.clear();
	}
}

SBDiphone* SBDiphoneManager::createDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name)
{
	std::string lowerCaseFromPhoneme = fromPhoneme;
	std::string lowerCaseToPhoneme = toPhoneme;
	std::transform(fromPhoneme.begin(), fromPhoneme.end(), lowerCaseFromPhoneme.begin(), ::tolower);
	std::transform(toPhoneme.begin(), toPhoneme.end(), lowerCaseToPhoneme.begin(), ::tolower);
	SBDiphone* diphone = getDiphone(lowerCaseFromPhoneme, lowerCaseToPhoneme, name);
	if (diphone)
	{
		LOG("Diphone set %s already contain diphone pair %s to %s, return existing one.", name.c_str(), lowerCaseFromPhoneme.c_str(), lowerCaseToPhoneme.c_str());
	}
	else
	{
		diphone = new SBDiphone(lowerCaseFromPhoneme, lowerCaseToPhoneme);
		_diphoneMap[name].push_back(diphone);
	}
	return diphone;
}

std::vector<SBDiphone*>& SBDiphoneManager::getDiphones(const std::string& name)
{
	std::map<std::string, std::vector<SBDiphone*> >::iterator iter = _diphoneMap.find(name);
	if (iter == _diphoneMap.end())
	{
		std::vector<SBDiphone*> newDiphones;
		_diphoneMap.insert(std::make_pair(name, newDiphones));
	}
	return _diphoneMap[name];
}

SBDiphone* SBDiphoneManager::getDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name)
{
	std::string lowerCaseFromPhoneme = fromPhoneme;
	std::string lowerCaseToPhoneme = toPhoneme;
	std::transform(fromPhoneme.begin(), fromPhoneme.end(), lowerCaseFromPhoneme.begin(), ::tolower);
	std::transform(toPhoneme.begin(), toPhoneme.end(), lowerCaseToPhoneme.begin(), ::tolower);
	std::vector<SBDiphone*>& diphones = getDiphones(name);
	for (size_t i = 0; i < diphones.size(); i++)
	{
		if (diphones[i]->getFromPhonemeName() == lowerCaseFromPhoneme && diphones[i]->getToPhonemeName() == lowerCaseToPhoneme)
		{
			return diphones[i];
		}
	}
	return NULL;
}

int SBDiphoneManager::getNumDiphoneMap()
{
	return _diphoneMap.size();
}

int SBDiphoneManager::getNumDiphones(const std::string& name)
{
	std::vector<SBDiphone*>& diphones = getDiphones(name);
	return diphones.size();
}

}