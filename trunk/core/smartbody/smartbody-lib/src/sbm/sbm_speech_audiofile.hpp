/*
 *  sbm_speech_audiofile.hpp - part of SmartBody-lib
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

#ifndef SBM_SPEECH_AUDIOFILE_HPP
#define SBM_SPEECH_AUDIOFILE_HPP

#include "sbm_speech.hpp"
#include "xercesc_utils.hpp"
#include "sr_hash_map.h"



namespace SmartBody {
	class AudioFileSpeech : public SpeechInterface {
	public:
		// Constructor / Destructor
		AudioFileSpeech();
		virtual ~AudioFileSpeech();

		//  Override SpeechInterface methods (see sbm_speech.hpp)
        virtual RequestId requestSpeechAudio( const char* agentName, const DOMNode* node, const char* callbackCmd );
        virtual RequestId requestSpeechAudio( const char* agentName, const char* text, const char* callbackCmd );
        virtual const std::vector<VisemeData *>* getVisemes( RequestId requestId );
        virtual char* getSpeechPlayCommand( RequestId requestId );
        virtual char* getSpeechStopCommand( RequestId requestId );
        virtual char* getSpeechAudioFilename( RequestId requestId );
        virtual float getMarkTime( RequestId requestId, const XMLCh* markId );
        virtual void requestComplete( RequestId requestId );
		static  srHashMap<std::string> AudioFileSpeech::audioSoundLookUp;
		static  srHashMap<std::string> AudioFileSpeech::audioVisemeLookUp;
		static  srHashMap<std::string> AudioFileSpeech::textOfUtterance;
		
	private:
		static int	audioMsgNumber;
		//std::string agentName;
		int wordScore(std::string word);
		std::string justTheText(std::string toConvert);
	};
};




// included after class definition b/c dependency
#include "mcontrol_util.h"
int init_audio_func(srArgBuffer& args, mcuCBHandle* mcu_p);

#endif // SBM_SPEECH_AUDIOFILE_HPP