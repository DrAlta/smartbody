#ifndef _SBPARSER_H_
#define _SBPARSER_H_

#include <vhcl.h>
#include <external/parser/Params.h>
#include <external/parser/InputTree.h>
#include <sbm/SBParseNode.h>

namespace SmartBody {

class SBParser
{
	public:
		SBParser();
		~SBParser();

		void initialize(const std::string& arg1, const std::string& arg2);
		SmartBody::SBParseNode* parse(const std::string& input);
		void cleanUp(SmartBody::SBParseNode* node);

	protected:
		void createParseTree(InputTree* inputTree, SBParseNode* node);

		bool _initialized;
		Params params;
		bool release;
		string agent_id;

};

}

#endif