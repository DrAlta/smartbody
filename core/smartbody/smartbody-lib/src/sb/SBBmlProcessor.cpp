#include "SBBmlProcessor.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBBmlProcessor::SBBmlProcessor()
{
}

SBBmlProcessor::~SBBmlProcessor()
{
}

// This command is inside bml_processor.cpp, unlike most of other commands inside mcontrol_util. So unable to rewrite, instead, re-routine to bp.
void SBBmlProcessor::vrSpeak(std::string agent, std::string recip, std::string msgId, std::string msg)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::stringstream msgStr;
	msgStr << agent << " " << recip << " " << msgId << " " << msg;
	srArgBuffer vrMsg(msgStr.str().c_str());
	BML::Processor& bp = mcu.bml_processor;
	bp.vrSpeak_func(vrMsg, &mcu);
}

void SBBmlProcessor::vrAgentBML(std::string op, std::string agent, std::string msgId, std::string msg)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (op == "request" || op == "start" || op == "end")
	{
		std::stringstream msgStr;
		msgStr << agent << " " << msgId << " " << op << " " << msg;
		srArgBuffer vrMsg(msgStr.str().c_str());
		BML::Processor& bp = mcu.bml_processor;
		bp.vrAgentBML_cmd_func(vrMsg, &mcu);
	}
	else
	{
		LOG("vrAgentBML option %s not recognized!", op.c_str());
		return;	
	}
}

std::string SBBmlProcessor::build_vrX(std::ostringstream& buffer, const std::string& cmd, const std::string& char_id, const std::string& recip_id, const std::string& content, bool for_seq ) 
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::stringstream msgId;
	if (mcu.process_id != "")
		msgId << "sbm_" << mcu.process_id << "_test_bml_" << (++mcu.testBMLId);
	else
		msgId << "sbm_test_bml_" << ++mcu.testBMLId;

	buffer.str("");
	if( for_seq )
		buffer << "send " << cmd << " ";
	buffer << char_id << " "<< recip_id << " " << msgId.str() << std::endl << content;
	return msgId.str();
}

std::string SBBmlProcessor::send_vrX( const char* cmd, const std::string& char_id, const std::string& recip_id,
			const std::string& seq_id, bool echo, bool send, const std::string& bml ) 
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::ostringstream msg;
	std::string msgId = "";
	bool all_characters = ( char_id=="*" );

	if( seq_id.length()==0 ) {
		if( echo ) {
			msgId = build_vrX( msg, cmd, char_id, recip_id, bml, false );
			// don't log a vrX message
			////LOG("%s %s", cmd, msg.str().c_str());
		}

		if( send ) {
			// execute directly
			if( all_characters ) {
				for(std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
					iter != mcu.getCharacterMap().end();
					iter++)
				{
					SbmCharacter* character = (*iter).second;
					msgId = build_vrX( msg, cmd, character->getName().c_str(), recip_id, bml, false );
					mcu.vhmsg_send( cmd, msg.str().c_str() );
				}
			} else {
				msgId = build_vrX( msg, cmd, char_id, recip_id, bml, false );
				///////LOG("vvmsg cmd =  %s, msg = %s", cmd, msg.str().c_str());
				mcu.vhmsg_send( cmd, msg.str().c_str() );
			}
		}
		return msgId;
	}else {
		// Command sequence to trigger vrSpeak
		srCmdSeq *seq = new srCmdSeq(); // sequence file that holds the bml command(s)
		seq->offset( (float)( mcu.time ) );

		if( echo ) {
			msg << "echo // Running sequence \"" << seq_id << "\"...";
			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert echo header command for character \"" << char_id << "\".";
				////LOG(strstr.str().c_str());
			}
			msgId = build_vrX( msg, cmd, char_id, recip_id, bml, false );
			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert echoed command for character \"" << char_id << "\".";
				////LOG(strstr.str().c_str());
			}
		}
		if( all_characters ) {
			for(std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
				iter != mcu.getCharacterMap().end();
				iter++)
			{
				SbmCharacter* character = (*iter).second;
				msgId = build_vrX( msg, cmd, character->getName().c_str(), recip_id, bml, true );
				if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
					std::stringstream strstr;
					strstr << "WARNING: send_vrX(..): Failed to insert vrSpeak command for character \"" << char_id << "\".";
					////LOG(strstr.str().c_str());
				}
			}
		} else {
			msgId = build_vrX( msg, cmd, char_id, recip_id, bml, true );
			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert vrSpeak command for character \"" << char_id << "\".";
				////LOG(strstr.str().c_str());
			}
		}

		if( send ) {
			mcu.activeSequences.removeSequence(seq_id, true); // remove old sequence by this name
			if( !mcu.activeSequences.addSequence(seq_id, seq ))
			{
				std::stringstream strstr;
				strstr << "ERROR: send_vrX(..): Failed to insert seq into active sequences.";
				////LOG(strstr.str().c_str());
				return msgId;
			}
		} else {
			mcu.pendingSequences.removeSequence(seq_id, true);  // remove old sequence by this name
			if (mcu.pendingSequences.addSequence(seq_id, seq))
			{
				std::stringstream strstr;
				strstr << "ERROR: send_vrX(..): Failed to insert seq into pending sequences.";
				////LOG(strstr.str().c_str());
				return msgId;
			}
		}
		return msgId;
	}
}

std::string SBBmlProcessor::execBML(std::string character, std::string bml)
{
	std::ostringstream entireBml;
	entireBml	<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				<< "<act>\n"
				<< "\t<bml>\n"
				<< "\t\t" << bml
				<< "\t</bml>\n"
				<< "</act>";	
	//return send_vrX( "vrSpeak", character, "ALL", "", true, true, entireBml.str() );
	return send_vrX( "vrSpeak", character, "ALL", "", false, true, entireBml.str() );
}

std::string SBBmlProcessor::execBMLFile(std::string character, std::string filename)
{
	//return send_vrX( "vrSpeak", character, "ALL", "", true, true, filename );
	return send_vrX( "vrSpeak", character, "ALL", "", false, true, filename );
}

std::string SBBmlProcessor::execXML(std::string character, std::string xml)
{
	std::ostringstream entireXML;
	entireXML	<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				<< xml;
	//return send_vrX( "vrSpeak", character, "ALL", "", true, true, entireXML.str() );
	return send_vrX( "vrSpeak", character, "ALL", "", false, true, entireXML.str() );
}

void SBBmlProcessor::execBMLAt(double time, std::string character, std::string bml)
{
	SBScene* scene = SBScene::getScene();

	std::stringstream strstr;
	strstr << "bml char " << character << " " << bml;
	scene->commandAt((float) time, strstr.str());
}

void SBBmlProcessor::execBMLFileAt(double time, std::string character, std::string filename)
{
	SBScene* scene = SBScene::getScene();

	std::stringstream strstr;
	strstr << "bml char " << character << " file " << filename;
	scene->commandAt((float) time, strstr.str());
}

void SBBmlProcessor::execXMLAt(double time, std::string character, std::string xml)
{
	SBScene* scene = SBScene::getScene();

	std::stringstream strstr;
	strstr << "bml char " << character << " " << xml;
	scene->commandAt((float) time, strstr.str());
}

void SBBmlProcessor::interruptCharacter(const std::string& character, double seconds)
{
	SBCharacter* sbCharacter = SBScene::getScene()->getCharacter(character);
	if (!sbCharacter)
	{
		LOG("No character named '%s' found. Interrupt not done.", character.c_str());
		return;
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.interrupt(sbCharacter, seconds, &mcu);
	
}

void SBBmlProcessor::interruptBML(const std::string& character, const std::string& id, double seconds)
{
	SBCharacter* sbCharacter = SBScene::getScene()->getCharacter(character);
	if (!sbCharacter)
	{
		LOG("No character named '%s' found. Interrupt not done.", character.c_str());
		return;
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.interrupt(sbCharacter, id, seconds, &mcu);
}





} // namespace

