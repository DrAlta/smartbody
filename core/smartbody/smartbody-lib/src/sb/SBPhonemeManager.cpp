#include <sb/SBPhonemeManager.h>
#include <sb/SBPhoneme.h>
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
	SBDiphone* diphone = getDiphone(fromPhoneme, toPhoneme, name);
	if (diphone)
	{
		LOG("Diphone set %s already contain diphone pair %s to %s, return existing one.", name.c_str(), fromPhoneme.c_str(), toPhoneme.c_str());
	}
	else
	{
		diphone = new SBDiphone(fromPhoneme, toPhoneme);
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
	std::vector<SBDiphone*>& diphones = getDiphones(name);
	for (size_t i = 0; i < diphones.size(); i++)
	{
		if (diphones[i]->getFromPhonemeName() == fromPhoneme && diphones[i]->getToPhonemeName() == toPhoneme)
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