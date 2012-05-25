#ifndef _SBPHONEME_H_
#define _SBPHONEME_H_

#include <vector>
#include <string>
#include <map>

namespace SmartBody {

class SBDiphone
{
public:
	SBDiphone();
	SBDiphone(const std::string& fromPhoneme, const std::string& toPhoneme);
	~SBDiphone();

	const std::string& getFromPhonemeName();
	const std::string& getToPhonemeName();

	void addKey(const std::string& viseme, float time, float weight);
	std::vector<float>& getKeys(const std::string& viseme);
	std::vector<std::string> getVisemeNames();
	int getNumVisemes();
	void clean();

private:
	std::string _fromPhoneme;
	std::string _toPhoneme;
	std::map<std::string, std::vector<float> > _visemeKeysMap;
};

}

#endif
