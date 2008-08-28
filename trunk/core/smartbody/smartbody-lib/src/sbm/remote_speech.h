/*
 *  remote_speech.h - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Andrew n marshall, USC
 *      Ashok Basawapatna, USC (no longer)
 */

#ifndef REMOTE_SPEECH_H
#define REMOTE_SPEECH_H


// Predeclare class
class remote_speech;


#include <string>
#include "sbm_speech.hpp"
#include "sr_hash_map.h"
#include "sr_arg_buff.h"
#include "sbm_character.hpp"
// Predeclare class
class mcuCBHandle;


class remote_speech: public SmartBody::SpeechInterface {
    public:
        /**
         *  Requests audio for a speech by agentName as specified in XML node.
         *  If node is a DOMElement, use the child nodes.
         *  If node is a text node, is the value a string.
         *  If node is an attribute, use the attribute value as a string.
         *  Returns a unique request identifier.
         */

		// Default Constructor/Destructor
		remote_speech( float timeOutInSeconds = 10 );
		virtual ~remote_speech();

		// Methods
		SmartBody::RequestId requestSpeechAudio( const char* agentName, const DOMNode* node, const char* callbackCmd ); //accepts dom document of which sound will be created from, returns Request ID
		SmartBody::RequestId requestSpeechAudio( const char* agentName, const char* text, const char* callbackCmd ); //accepts char* of above and returns request ID
		std::vector<SmartBody::VisemeData *>* getVisemes( SmartBody::RequestId requestId ); //returns visemes  for given request
		char* getSpeechPlayCommand( SmartBody::RequestId requestId, const int characterId = 0 ); //returns the command to play speech
		char* getSpeechStopCommand( SmartBody::RequestId requestId ); //''                     stop
		char* getSpeechAudioFilename( SmartBody::RequestId requestId ); // gets the fileName of speech
		float getMarkTime( SmartBody::RequestId requestId, const XMLCh* markId ); //gets time value for a given marker
		

		void requestComplete( SmartBody::RequestId requestId );

		// RemoteSpeech specific methods
		int handleRemoteSpeechResult( SbmCharacter* character, char* msgID, char* status, char* result, mcuCBHandle* mcu_p );
		int testRemoteSpeechTimeOut( const char* request_id_str, mcuCBHandle* mcu_p );
		
	private:
		std::vector<SmartBody::VisemeData *>* extractVisemes(DOMNode* node, std::vector<SmartBody::VisemeData*>* visemes);
		std::string forPlaysound;
		srHashMap<DOMNode>     uttLookUp; 
		srHashMap<std::string> soundLookUp;
		srHashMap<const char>  commandLookUp;
		srHashMap<std::string> remote_speech::charLookUp;
		unsigned int           msgNumber;
		float                  timeOut; //seconds for timeout between sbm's RvoiceRequest and Remote speech process's RmoteSpeechReply
};



// included after class definition b/c dependency
#include "mcontrol_util.h"

int remoteSpeechResult_func( srArgBuffer& args, mcuCBHandle* mcu_p);
int set_char_voice(char* char_name, char* voiceCode, mcuCBHandle* mcu_p);
int remoteSpeechTimeOut_func(srArgBuffer& args, mcuCBHandle* mcu_p);

// Test functions
// TODO: move to "test ..."
int remote_speech_test( srArgBuffer& args, mcuCBHandle* mcu_p);
int remoteSpeechReady_func( srArgBuffer& args, mcuCBHandle* mcu_p);

#endif // REMOTE_SPEECH_H
