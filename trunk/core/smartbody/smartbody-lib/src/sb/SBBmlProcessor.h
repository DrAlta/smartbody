#ifndef _SBBMLPROCESSOR_H
#define _SBBMLPROCESSOR_H

#include <string>
#include <ostream>

namespace SmartBody {

class SBBmlProcessor
{
	public:
		SBBmlProcessor();
		~SBBmlProcessor();

		void vrSpeak(std::string agent, std::string recip, std::string msgId, std::string msg);
		void vrAgentBML(std::string op, std::string agent, std::string msgId, std::string msg);
		
		std::string execBML(std::string character, std::string bml);
		std::string execBMLFile(std::string character, std::string filename);
		std::string execXML(std::string character, std::string xml);

		void interruptCharacter(const std::string& character, double seconds);
		void interruptBML(const std::string& character, const std::string& id, double seconds);

	protected:
		std::string build_vrX(std::ostringstream& buffer, const std::string& cmd, const std::string& char_id, const std::string& recip_id, const std::string& content, bool for_seq );
		std::string send_vrX( const char* cmd, const std::string& char_id, const std::string& recip_id,
			const std::string& seq_id, bool echo, bool send, const std::string& bml );
};

}

#endif