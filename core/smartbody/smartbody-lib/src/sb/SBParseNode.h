#ifndef _SBPARSETREE_H_
#define _SBPARSETREE_H_

#include <string>
#include <vector>

namespace SmartBody {

class SBParseNode
{
	public:
		SBParseNode();
		SBParseNode(const std::string& word, const std::string& term);
		~SBParseNode();

		bool isTerminal();
		void setWord(const std::string& word);
		void setTerm(const std::string& term);

		const std::string& getWord();
		const std::string& getTerm();

		void addChild(SBParseNode* node);
		int getNumChildren();
		SBParseNode* getChild(int num);

	protected:
		std::string _word;
		std::string _term;
		std::vector<SBParseNode*> _children;

};

}


#endif