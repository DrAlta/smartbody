#ifndef _SBPHONEMEMANAGER_H_
#define _SBPHONEMEMANAGER_H_

#include <sb/SBTypes.h>
#include <vector>
#include <string>
#include <map>
#include "SBService.h"

namespace SmartBody{

class SBDiphone;

class SBPhonemeManager : public SBService
{
	public:
		SBAPI SBPhonemeManager();
		SBAPI ~SBPhonemeManager();

		SBAPI virtual void setEnable(bool val);
		SBAPI void setPhonemesRealtime(const std::string& character, const std::string& phoneme);
		SBAPI void clearPhonemesRealtime(const std::string& character, const std::string& phoneme);
		SBAPI std::vector<std::string> getPhonemesRealtime(const std::string& character, const std::string& phoneme);
		
		SBAPI std::vector<std::string> getCommonPhonemes();
	
		SBAPI SBDiphone* createDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name);
		SBAPI std::vector<SBDiphone*>& getDiphones(const std::string& name);
		SBAPI SBDiphone* getDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name);
		SBAPI SBDiphone* getMappedDiphone(const std::string& fromPhoneme, const std::string& toPhoneme, const std::string& name);
		SBAPI std::vector<std::string> getDiphoneMapNames();
		SBAPI int getNumDiphoneMap();
		SBAPI int getNumDiphones(const std::string& name);
		SBAPI void normalizeCurves(const std::string& name);
		SBAPI void deleteDiphoneSet(const std::string& name);

		SBAPI void addPhonemeMapping(const std::string& from, const std::string& to);
		SBAPI std::string getPhonemeMapping(const std::string& from);

		SBAPI void loadDictionary(const std::string& language, const std::string& file);
		SBAPI void addDictionaryWord(const std::string& language, const std::string& word, std::vector<std::string>& phonemes);
		SBAPI std::vector<std::string>& getDictionaryWord(const std::string& language, const std::string& word);
		SBAPI int getNumDictionaryWords(const std::string& language);

		SBAPI std::string getDictionaryFile(const std::string& language);


	protected:
		std::map<std::string, std::vector<SBDiphone*> > _diphoneMap;
		std::map<std::string, std::string> _phonemeToCommonPhonemeMap;
		std::map<std::string, std::map<std::string, std::vector<std::string> > > _wordToPhonemeMaps;
		std::vector<std::string> _emptyPhonemes;
		std::map<std::string, std::string> _dictionaryFileMap;

		std::map<std::string, std::vector<std::string> > _realtimePhonemes;
};

}
#endif
