#ifndef _SBPARSER_H_
#define _SBPARSER_H_

#include <vhcl.h>
#include <external/parser/Params.h>

class InputTree;

namespace SmartBody {

class SBParseNode;

class SBParser
{
	public:
		SBParser();
		~SBParser();

		void initialize(const std::string& arg1, const std::string& arg2);
		SmartBody::SBParseNode* parse(const std::string& input);
		void cleanUp(SmartBody::SBParseNode* node);
		bool isInitialized();

	protected:
		void createParseTree(InputTree* inputTree, SBParseNode* node);

		bool _initialized;
		Params params;
		bool release;
		string agent_id;

};

}

#endif