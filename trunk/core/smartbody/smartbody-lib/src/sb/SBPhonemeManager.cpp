#include <sb/SBPhonemeManager.h>
#include <sb/SBPhoneme.h>
#include <algorithm>
#include <vhcl.h>

namespace SmartBody {

SBDiphoneManager::SBDiphoneManager()
{
}

std::vector<std::string> SBDiphoneManager::getCommonPhonemes()
{
	std::vector<std::string> commonPhonemes;

	commonPhonemes.push_back("Aa");   /// Viseme for aa
	commonPhonemes.push_back("Ah");   /// Viseme for aa, ae, ah
	commonPhonemes.push_back("Ao");
	commonPhonemes.push_back("Aw");   /// aw
	commonPhonemes.push_back("Ay");  /// ay
	commonPhonemes.push_back("Bmp");
	commonPhonemes.push_back("D");
	commonPhonemes.push_back("Ee");
	commonPhonemes.push_back("Eh");   /// ey, eh, uh
	commonPhonemes.push_back("Er");
	commonPhonemes.push_back("F");
	commonPhonemes.push_back("H");  /// h
	commonPhonemes.push_back("Ih");   /// y, iy, ih, ix
	commonPhonemes.push_back("J");
	commonPhonemes.push_back("Kg");
	commonPhonemes.push_back("Ih");
	commonPhonemes.push_back("L");   /// l
	commonPhonemes.push_back("Ng");
	commonPhonemes.push_back("Oh");
	commonPhonemes.push_back("Oo");
	commonPhonemes.push_back("Ow");   /// ow
	commonPhonemes.push_back("Oy");  /// oy
	commonPhonemes.push_back("R");
	commonPhonemes.push_back("Sh");   /// sh, ch, jh, zh
	commonPhonemes.push_back("Th");
	commonPhonemes.push_back("W");
	commonPhonemes.push_back("Z");
	commonPhonemes.push_back("_");	/// silence

	return commonPhonemes;
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