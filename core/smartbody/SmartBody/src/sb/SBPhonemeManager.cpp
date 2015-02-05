#include <sb/SBPhonemeManager.h>
#include <sb/SBPhoneme.h>
#include <algorithm>
#include <vector>
#include <set>
#include <vhcl.h>
#include <fstream>
#include <cctype>

namespace SmartBody {

SBDiphoneManager::SBDiphoneManager()
{
	addPhonemeMapping("SIL", "_");
	addPhonemeMapping("PAU", "_");
	addPhonemeMapping("AA", "Aa");
	addPhonemeMapping("AE", "Ah");
	addPhonemeMapping("AH", "Ah");
	addPhonemeMapping("AO", "Ao");
	addPhonemeMapping("AX", "Ah");
	addPhonemeMapping("@", "Ah");
	addPhonemeMapping("AY", "Ay");
	addPhonemeMapping("B", "BMP");
	addPhonemeMapping("CH", "Sh");
	addPhonemeMapping("D", "D");
	addPhonemeMapping("DH", "Th");
	addPhonemeMapping("DX", "D");
	addPhonemeMapping("EH", "Eh");
	addPhonemeMapping("ER", "Er");
	addPhonemeMapping("EY", "Eh");
	addPhonemeMapping("F", "F");
	addPhonemeMapping("G", "Kg");
	addPhonemeMapping("HH", "Ih");
	addPhonemeMapping("IH", "Ih");
	addPhonemeMapping("IY", "Ih");
	addPhonemeMapping("JH", "Sh");
	addPhonemeMapping("K", "Kg");
	addPhonemeMapping("L", "L");
	addPhonemeMapping("M", "BMP");
	addPhonemeMapping("N", "D");
	addPhonemeMapping("NG", "Kg");
	addPhonemeMapping("OW", "Ow");
	addPhonemeMapping("OY", "Oy");
	addPhonemeMapping("P", "BMP");
	addPhonemeMapping("R", "R");
	addPhonemeMapping("S", "Z");
	addPhonemeMapping("T", "D");
	addPhonemeMapping("SH", "Sh");
	addPhonemeMapping("T", "D");
	addPhonemeMapping("TH", "Th");
	addPhonemeMapping("UH", "Eh");
	addPhonemeMapping("UW", "W");
	addPhonemeMapping("V", "F");
	addPhonemeMapping("W", "W");
	addPhonemeMapping("Y", "Ih");
	addPhonemeMapping("Z", "Z");
	addPhonemeMapping("ZH", "Sh");
}

void SBDiphoneManager::addPhonemeMapping(const std::string& from, const std::string& to)
{
	std::string upperCaseFrom = from;
	std::string upperCaseTo = to;
	std::transform(upperCaseFrom.begin(), upperCaseFrom.end(), upperCaseFrom.begin(), toupper);
	std::transform(upperCaseTo.begin(), upperCaseTo.end(), upperCaseTo.begin(), toupper);

	std::map<std::string, std::string>::iterator iter = _phonemeToCommonPhonemeMap.find(upperCaseFrom);
	if (iter != _phonemeToCommonPhonemeMap.end())
	{
		LOG("Phoneme mapping from %s->%s already found, replacing with %s->%s", (*iter).first.c_str(), (*iter).second.c_str(), upperCaseFrom.c_str(), upperCaseTo.c_str());
	}
	_phonemeToCommonPhonemeMap[from] = upperCaseTo;
}

std::string SBDiphoneManager::getPhonemeMapping(const std::string& from)
{
	std::string uppercaseFrom = from;
	std::transform(uppercaseFrom.begin(), uppercaseFrom.end(), uppercaseFrom.begin(), toupper);
	std::map<std::string, std::string>::iterator iter = _phonemeToCommonPhonemeMap.find(uppercaseFrom);
	if (iter != _phonemeToCommonPhonemeMap.end())
	{
		return  (*iter).second;
	}
	else
	{
		return "";
	}
	
}



std::vector<std::string> SBDiphoneManager::getCommonPhonemes()
{
	std::set<std::string> commonPhonemeSet;

	for (std::map<std::string, std::string>::iterator iter = _phonemeToCommonPhonemeMap.begin();
		 iter != _phonemeToCommonPhonemeMap.end();
		 iter++)
	{
		commonPhonemeSet.insert((*iter).second);
	}

	std::vector<std::string> commonPhonemesVector;

	for (std::set<std::string>::iterator iter = commonPhonemeSet.begin();
		 iter != commonPhonemeSet.end();
		 iter++)
	{
		commonPhonemesVector.push_back(*iter);
	}
	return commonPhonemesVector;
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

void SBDiphoneManager::deleteDiphoneSet(const std::string& name)
{
	if (_diphoneMap.find(name) != _diphoneMap.end())
	{
		_diphoneMap.erase(name);
	}
}

SBDiphone* SBDiphoneManager::createDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name)
{
	std::string upperCaseFromPhoneme = fromPhoneme;
	std::string upperCaseToPhoneme = toPhoneme;
	std::transform(upperCaseFromPhoneme.begin(), upperCaseFromPhoneme.end(), upperCaseFromPhoneme.begin(), ::toupper);
	std::transform(upperCaseToPhoneme.begin(), upperCaseToPhoneme.end(), upperCaseToPhoneme.begin(), ::toupper);
	SBDiphone* diphone = getDiphone(upperCaseFromPhoneme, upperCaseToPhoneme, name);
	if (diphone)
	{
		;//LOG("Diphone set %s already contain diphone pair %s to %s, return existing one.", name.c_str(), lowerCaseFromPhoneme.c_str(), lowerCaseToPhoneme.c_str());
	}
	else
	{
		diphone = new SBDiphone(upperCaseFromPhoneme, upperCaseToPhoneme);
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

SBDiphone* SBDiphoneManager::getMappedDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name)
{
	std::string mappedFrom = getPhonemeMapping(fromPhoneme);
	std::string mappedTo = getPhonemeMapping(toPhoneme);
	return getDiphone(fromPhoneme, toPhoneme, name);
}

SBDiphone* SBDiphoneManager::getDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name)
{
	std::string upperCaseFromPhoneme = fromPhoneme;
	std::string upperCaseToPhoneme = toPhoneme;
	std::transform(upperCaseFromPhoneme.begin(), upperCaseFromPhoneme.end(), upperCaseFromPhoneme.begin(), ::toupper);
	std::transform(upperCaseToPhoneme.begin(), upperCaseToPhoneme.end(), upperCaseToPhoneme.begin(), ::toupper);

	std::vector<SBDiphone*>& diphones = getDiphones(name);
	for (size_t i = 0; i < diphones.size(); i++)
	{
		if (diphones[i]->getFromPhonemeName() == upperCaseFromPhoneme && 
			diphones[i]->getToPhonemeName() == upperCaseToPhoneme)
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

std::vector<std::string> SBDiphoneManager::getDiphoneMapNames()
{
	std::vector<std::string> diphoneMaps;
	for (std::map<std::string, std::vector<SBDiphone*> >::iterator iter = _diphoneMap.begin();
		 iter != _diphoneMap.end();
		 iter++)
	{
		diphoneMaps.push_back((*iter).first);
		
	}

	return diphoneMaps;

}

void SBDiphoneManager::normalizeCurves(const std::string& name)
{
	std::map<std::string, std::vector<SBDiphone*> >::iterator iter = _diphoneMap.find(name);
	if (iter != _diphoneMap.end())
	{
		// iterate through the phone bigrams
		for (std::map<std::string, std::vector<SBDiphone*> >::iterator iter2 = _diphoneMap.begin();
			 iter2 != _diphoneMap.end();
			 iter2++)
		{
			// get the phone bigram
			std::vector<SBDiphone*>& diphones = getDiphones(iter->first);
			for (std::vector<SBDiphone*>::iterator diphoneIter = diphones.begin();
				 diphoneIter != diphones.end();
				 diphoneIter++)
			{
				SBDiphone* diphone = (*diphoneIter);
				// get the keys and find the largest value in the list
				float maxVal = -1.0f;
				std::vector<std::string> visemes = diphone->getVisemeNames();
				for (std::vector<std::string>::iterator visemeIter = visemes.begin();
					 visemeIter != visemes.end();
					 visemeIter++)
				{
					const std::string& viseme = (*visemeIter);
					std::vector<float>& keys = diphone->getKeys(viseme);
					bool isTime = true;
					for (std::vector<float>::iterator keyIter = keys.begin();
						 keyIter != keys.end();
						 keyIter++)
					{
						float& val = (*keyIter);
						if (isTime)
						{
							isTime = false;
						}
						else
						{
							if (val > maxVal)
								maxVal = val;
							isTime = true;
						}
					}
				}

				if (maxVal <= -0.0f )
				{
					continue;
				}

				// normalize the values according the the scale
				LOG("Normalize scale is %f", maxVal);
				float scale = 1.0f / ((1.0f + maxVal) / 2.0f);
				for (std::vector<std::string>::iterator visemeIter = visemes.begin();
					 visemeIter != visemes.end();
					 visemeIter++)
				{
					const std::string& viseme = (*visemeIter);
					std::vector<float>& keys = diphone->getKeys(viseme);
					bool isTime = true;
					for (std::vector<float>::iterator keyIter = keys.begin();
						 keyIter != keys.end();
						 keyIter++)
					{
						float& val = (*keyIter);
						if (isTime)
						{
							isTime = false;
						}
						else
						{
							val *= scale;
							isTime = true;
						}
					}
				}

			}
		}
	}
}

void SBDiphoneManager::loadDictionary(const std::string& language, const std::string& file)
{
	std::ifstream filestream( file.c_str() );
	if (!filestream.good())
	{
		LOG("Could not open dictionary file %s", file.c_str());
		return;
	}
	
	int numWords = 0;
	char line[8192];
	while(!filestream.eof() && 
		   filestream.good())
	{
		filestream.getline(line, 4096, '\n');
        if (line[strlen(line) - 1] == '\r')
			line[strlen(line) - 1] = '\0';

		if (strlen(line) == 0) // ignore blank lines
			continue;

		std::string strLine(line);
		if (line[0] == ';')
			continue;
		std::vector<std::string> tokens;
		vhcl::Tokenize(strLine, tokens, " ");
		if (tokens.size() > 0)
		{
			std::vector<std::string> phonemes;
			for (size_t t = 1; t < tokens.size(); t++)
			{
				// for vowel sounds, remove the emphasis
				if (tokens[t].size() == 3)
				{
					phonemes.push_back(tokens[t].substr(0,2));
				}
				else
				{
					phonemes.push_back(tokens[t]);
				}
			}
				
			addDictionaryWord("English", tokens[0], phonemes);
			numWords++;
		}

	}
	filestream.close();
	LOG("Dictionary for language %s loaded with %d words.", language.c_str(), numWords);
	_dictionaryFileMap[language] = file;
}

void SBDiphoneManager::addDictionaryWord(const std::string& language, const std::string& word, std::vector<std::string>& phonemes)
{
	std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator iter = _wordToPhonemeMaps.find(language);
	if (iter == _wordToPhonemeMaps.end())
	{
		// create a new map
		std::map<std::string, std::vector<std::string> > languageMap;
		_wordToPhonemeMaps[language] = languageMap;
		iter = _wordToPhonemeMaps.find(language);
	}

	std::map<std::string, std::vector<std::string> >::iterator phonemeLookupIter = (*iter).second.find(word);
	if (phonemeLookupIter != (*iter).second.end())
	{
		// if the word already exists, warn and replace it
		LOG("Dictionary word %s in language %s already found, replacing...", word.c_str(), language.c_str());
	}
	(*iter).second[word] = phonemes;
}

std::vector<std::string>& SBDiphoneManager::getDictionaryWord(const std::string& language, const std::string& word)
{
	std::string uppercaseWord = vhcl::ToUpper(word);
	
	std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator iter = _wordToPhonemeMaps.find(language);
	if (iter == _wordToPhonemeMaps.end())
	{
		LOG("Dictionary word %s cannot be found: no lanugage %s present.", uppercaseWord.c_str(), language.c_str());
		return _emptyPhonemes;
	}

	std::map<std::string, std::vector<std::string> >& wordPhonemeMap = (*iter).second;

	std::map<std::string, std::vector<std::string> >::iterator phonemeLookupIter = wordPhonemeMap.find(uppercaseWord);
	if (phonemeLookupIter == wordPhonemeMap.end())
	{
		LOG("Dictionary word %s in language %s cannot be found.", uppercaseWord.c_str(), language.c_str());
		return _emptyPhonemes;
	}

	return (*phonemeLookupIter).second;
}

std::string SBDiphoneManager::getDictionaryFile(const std::string& language)
{
	std::map<std::string, std::string>::iterator iter = _dictionaryFileMap.find(language);
	if (iter == _dictionaryFileMap.end())
		return "";
	return (*iter).second;

}

int SBDiphoneManager::getNumDictionaryWords(const std::string& language)
{
	std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator iter = _wordToPhonemeMaps.find(language);
	if (iter != _wordToPhonemeMaps.end())
	{
		return (*iter).second.size();
	}
	else
	{
		return 0;
	}
}


}
