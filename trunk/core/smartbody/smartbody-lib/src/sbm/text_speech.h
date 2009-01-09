#ifndef TEXT_SPEECH_H
#define TEXT_SPEECH_H


// Predeclare class
class text_speech;


#include <string>
#include "sbm_speech.hpp"
#include "sr_hash_map.h"
#include "sr_arg_buff.h"
#include "sbm_character.hpp"
// Predeclare class
class mcuCBHandle;
class srCmdSeq;


class text_speech: public SmartBody::SpeechInterface {
    public:
		// Default Constructor/Destructor
		text_speech();
		virtual ~text_speech();

		// Methods
		SmartBody::RequestId requestSpeechAudio( const char* agentName, const DOMNode* node, const char* callbackCmd ); //accepts dom document of which sound will be created from, returns Request ID
		SmartBody::RequestId requestSpeechAudio( const char* agentName, const char* text, const char* callbackCmd ); //accepts char* of above and returns request ID
		std::vector<SmartBody::VisemeData *>* getVisemes( SmartBody::RequestId requestId ); //returns visemes  for given request
		char* getSpeechPlayCommand( SmartBody::RequestId requestId, const SbmCharacter* character = NULL ); //returns the command to play speech
		char* getSpeechStopCommand( SmartBody::RequestId requestId, const SbmCharacter* character = NULL ); //''                     stop
		char* getSpeechAudioFilename( SmartBody::RequestId requestId ); // gets the fileName of speech
		float getMarkTime( SmartBody::RequestId requestId, const XMLCh* markId ); //gets time value for a given marker
		void requestComplete( SmartBody::RequestId requestId );

		void startSchedule( SmartBody::RequestId requestId );
		static int text_speech_func( srArgBuffer& args, mcuCBHandle *mcu_p );
	private:
		std::vector<SmartBody::VisemeData *>* extractVisemes(DOMNode* node, std::vector<SmartBody::VisemeData*>* visemes);
		std::string forPlaysound;
		srHashMap<DOMNode>     uttLookUp;
		srHashMap<srCmdSeq>    scheduleLookUp;
		srHashMap<std::string> text_speech::charLookUp;
		unsigned int           msgNumber;
};



// included after class definition b/c dependency
#include "mcontrol_util.h"

#endif