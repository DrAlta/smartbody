/*
 *  sbm_speech.hpp - part of SmartBody-lib
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
 *      Ed Fast, USC
 *      Corne Versloot, Univ of Twente (while at USC)
 */

# ifndef SBM_AUDIO_HPP
# define SBM_AUDIO_HPP


# include <vector>
# include <iostream>
# include "vhcl.h"

# include "xercesc_utils.hpp"


// Forward declarations
class SbmCharacter;


namespace SmartBody {
	// typedef
	typedef int RequestId;


	/**
	*  interface for viseme data.
	*/
	class VisemeData {
	private:
		std::string _id;
		float _weight;
		float _time;
		float _duration;
		int		_numKeys;
		std::string	_curveInfo;
		bool _curveMode;

	public:
		VisemeData( const char * id, float weight, float time )
			: _id( id ), _weight( weight ), _time( time ), _duration( 0 ), _numKeys( 0 ), _curveInfo( "" ), _curveMode(false)
		{
		}
		
		VisemeData( const char * id, float weight, float time, float duration )
			: _id( id ), _weight( weight ), _time( time ), _duration( duration ), _numKeys( 0 ), _curveInfo( "" ), _curveMode(false)
		{
		}


		VisemeData( const char * id, int numKeys, const char * curveInfo )
			: _id( id ), _weight( 1.0 ), _time( 0.0 ), _duration( 0.0 ), _numKeys( numKeys ), _curveInfo( curveInfo ), _curveMode(true)
		{
			// NOTE: We are parsing twice - once here in order to extract the proper duration of the curve,
			//       and later again when processing the command. These two parses should be unified here for
			//       efficiency reasons. The parse below is a 'light' parse - although we need to tokenize the
			//       entire curve data string, we are only interested in the last control point.

			std::vector<std::string> tokens;
			vhcl::Tokenize(curveInfo, tokens, " ");
			if (tokens.size() > 0 && numKeys > 0)
			{
				// make sure the curve info is properly created
				int index = 4 * (numKeys - 1);
				if (tokens.size() <= (unsigned int) index)
				{
					LOG("Curve %s is not properly created - insufficient number of keys (%d) - expected %d.", id, tokens.size() / 4, numKeys * 4);
				}
				else
				{
					_duration = float(atof(tokens[index].c_str()));
				}
			}
		}

		virtual ~VisemeData()
		{
		}


		/** Return the viseme identifier/name.  */
		const char * id() const { return _id.c_str(); }

		/** Return the weight of the viseme. */
		float weight() const { return _weight; }

		/** Return the audio relative time to trigger this viseme. */
		float time() const { return _time; }
		
		/** Return the blend-in duration of the viseme. */
		float duration() const { return _duration; }

		/** Set the audio relative time to trigger this viseme. */
		void setTime( float time ) { _time = time; }

		/** Set the blend-in duration of the viseme. */
		void setDuration( float duration ) { _duration = duration; }

		/** Get the number of keys. */
		int getNumKeys() {return _numKeys;}

		/** Get the curve information. */
		const char* getCurveInfo() {return _curveInfo.c_str();}

		/** Get the viseme Mode. */
		bool isCurveMode() {return _curveMode;}
	};

	/**
	 *  Write VisemeData to Stream
	 */
	std::ostream& operator << ( std::ostream& strm, const VisemeData& v );

    /**
     *  Abstract interface to speech synthesizers and speech audio.
     *
     *  Speech requests evaluate in four steps:
     *  1. A request is made with one of the given requestSpeechAudio() methods
     *  2. The speech implementation sends out a SBM result message of 
     *     the form: <callbackCmd> <agentId> <requestId> SUCCESS
     *           or:  <callbackCmd> <agentId> <requestId> ERROR <msg>
     *  3. Up reciept of the callbackMsg, the client can query result data via
     *     any of the get*(RequestId) methods.
     *  4. When complete, the client should call requestComplete(RequestId).
     *
     *  Each of the get*(RequestId) methods return a pointer to an object.
     *  This object belongs to the SpeechInterface and should not be deleted by 
     *  SpeechInterface clients.  The SpeechInterface implementation can delete 
     *  the object on a call to requestComplete(..), or in its destructor.
     *
     *  If any get*(RequestId) method has been called on an invalid RequestId
     *  or prior to the availability of the data, the method should return NULL.
     */
    class SpeechInterface {
    public:
        /**
         *  Requests audio for a speech by agentName as specified in XML node.
         *  If node is a DOMElement, use the child nodes.
         *  If node is a text node, is the value a string.
         *  If node is an attribute, use the attribute value as a string.
         *  Returns a unique request identifier.
         */
		// TODO: Define error return value as a constant somewhere (or new exception type).
        virtual RequestId requestSpeechAudio( const char* agentName, const DOMNode* node, const char* callbackCmd ) = 0;

        /**
         *  Requests audio for a speech char[] text by agentName.
         *  Returns a unique request identifier.
         */
		// TODO: Define error return value as a constant somewhere (or new exception type).
		virtual RequestId requestSpeechAudio( const char* agentName, std::string text, const char* callbackCmd ) = 0;

        /**
         *  If the request has been processed, returns the time ordered vector 
         *  of VisemeData for the requestId.  Otherwise return NULL.
         *
         *  Visemes in this list are actually morph targets, and multiple
         *  visemes with different weights can be added together.  Because of
         *  this, the returned viseme list should include zero weighted
         *  VisemeData instances of when to cancel previous visemes (change
         *  of viseme, and end of words).
         */
        virtual const std::vector<VisemeData *>* getVisemes( RequestId requestId ) = 0;

        /**
         *  Returns the sbm command used to play the speech audio.
         */
        virtual char* getSpeechPlayCommand( RequestId requestId, const SbmCharacter* character = NULL ) = 0;

        /**
         *  Returns the sbm command used to stop the speech audio.
         */
        virtual char* getSpeechStopCommand( RequestId requestId, const SbmCharacter* character = NULL ) = 0;

        /**
         *  Returns the filename of the audio.
         */
        virtual char* getSpeechAudioFilename( RequestId requestId ) = 0;

        /**
         *  Returns the timing for a synthesis bookmark,
         *  or -1 if the markId is not recognized.
         */
        virtual float getMarkTime( RequestId requestId, const XMLCh* markId ) = 0;

        /**
         *  Signals that requestId processing is complete and its data can be 
         *  discarded.  May be called (...?)
         */
        virtual void requestComplete( RequestId requestId ) = 0;

    };
}  // end namespace SmartBody

# endif // SBM_AUDIO_HPP
