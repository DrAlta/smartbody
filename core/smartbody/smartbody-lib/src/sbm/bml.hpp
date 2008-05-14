/*
 *  bml.hpp - part of SmartBody-lib
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
 *      Corne Versloot, while at USC
 *      Ed Fast, USC
 */

#ifndef BML_HPP
#define BML_HPP

#include <string>
#include <vector>
#include <map>
#include <limits>


//#include "mcontrol_util.h"
#include "sbm_character.hpp"
#include "sbm_speech.hpp"

#include "xercesc_utils.hpp"

// Controllers...
#include <ME/me_ct_motion.h>
#include "me_ct_examples.h"



namespace BML {
	//  Common XML Identifiers
	const XMLCh ATTR_ID[]    = L"id";
	const XMLCh ATTR_TYPE[]  = L"type";
	const XMLCh ATTR_NAME[]  = L"name";
	const XMLCh ATTR_LEVEL[] = L"level";

	const XMLCh ATTR_START[]        = L"start";
	const XMLCh ATTR_READY[]        = L"ready";
	const XMLCh ATTR_STROKE_START[] = L"stroke-start";
	const XMLCh ATTR_STROKE[]       = L"stroke";
	const XMLCh ATTR_STROKE_END[]   = L"stroke-end";
	const XMLCh ATTR_RELAX[]        = L"relax";
	const XMLCh ATTR_END[]          = L"end";

	const XMLCh TM_START[]        = L"start";
	const XMLCh TM_READY[]        = L"ready";
	const XMLCh TM_STROKE_START[] = L"stroke-start";
	const XMLCh TM_STROKE[]       = L"stroke";
	const XMLCh TM_STROKE_END[]   = L"stroke-end";
	const XMLCh TM_RELAX[]        = L"relax";
	const XMLCh TM_END[]          = L"end";


    typedef double time_sec;  // An attempt to generalize the floating point format
	enum GestureType { BML_MOTION };
	enum HeadBehaviorType { HEAD_NOD, HEAD_SHAKE, HEAD_TOSS, HEAD_ORIENT };

    const time_sec TIME_UNSET = std::numeric_limits<time_sec>::infinity();

	//  Helper Function
	const XMLCh* buildBmlId( const XMLCh* behaviorId, const XMLCh* synchId );
	bool isValidBmlId( const XMLCh* id );
	bool isValidTmId( const XMLCh* id );


	//  Class Declarations
	struct SbmCommand;
	class TriggerEvent;
	class SynchPoint;
	class BehaviorRequest;
	class SpeechRequest;
	typedef std::map< const XMLCh*, SynchPoint*, xml_utils::XMLStringCmp > SynchPointMap;	


	// Typedefs
	typedef std::vector<TriggerEvent*>           VecOfTriggerEvent;
	typedef std::vector<SmartBody::VisemeData*>  VecOfVisemeData;
	typedef std::vector<SbmCommand*>             VecOfSbmCommand;
	typedef std::vector<BehaviorRequest*>        VecOfBehaviorRequest;


	// Class Definitions
	class BmlRequest {
	public:
		const SbmCharacter   *agent;

		const std::string    requestId;
		const std::string    recipientId;
		const std::string    msgId;

		VecOfTriggerEvent    triggers;
		SynchPoint*          first;
		SynchPoint*          last;
		VecOfVisemeData      visemes;
		VecOfBehaviorRequest behaviors;
		SynchPointMap        synch_points;
		SpeechRequest*       speech;
		//  TODO: Move these into SpeechRequest
		char*                audioPlay;
		char*                audioStop;

		BmlRequest( const SbmCharacter* agent, const std::string & requestId, const std::string & recipientId, const std::string & msgId, unsigned int numTriggers );

		virtual ~BmlRequest();

		void addBehavior( BehaviorRequest* behavior );

		SynchPoint* getSynchPoint( const XMLCh* name );  // Lookup a SynchPoint
	};

	class TriggerEvent {
	public:
		BmlRequest* request;
		SynchPoint*    start;
		SynchPoint*    end;

		//std::vector<const XMLCh*> tids;  // Time IDs;

		TriggerEvent( BmlRequest *request, TriggerEvent *prev );

		virtual ~TriggerEvent();

		// TODO: move this method to BmlRequest
		SynchPoint* addSynchPoint( const XMLCh* name );  // adds SynchPoint before end of trigger

		// TODO: use BmlRequest variant
		SynchPoint* getSynchPoint( const XMLCh* name );  // Lookup a SynchPoint
	};

	class SynchPoint {
	public:
		const XMLCh *const  name;
		const TriggerEvent* trigger;
		SynchPoint*         prev;
		SynchPoint*         next;
		time_sec            time;  // TIME_UNSET implies it has not been set
		SynchPoint*		    parent;
		float			    offset;	

		SynchPoint( const XMLCh* name, const TriggerEvent *trigger, SynchPoint *after );
		SynchPoint( const XMLCh* name, const TriggerEvent *trigger, SynchPoint *after, SynchPoint *par, float off );

		virtual ~SynchPoint();
	};

	// Ordered list of synch points, with references to standard synch_points.
	class SynchPoints {
	public:
		SynchPoint* start;
		SynchPoint* ready;
		SynchPoint* strokeStart;
		SynchPoint* stroke;
		SynchPoint* strokeEnd;
		SynchPoint* relax;
		SynchPoint* end;

		SynchPoints();

		//  TODO: Replace trigger with BmlRequest
		void parseSynchPoints( DOMElement* elem, TriggerEvent* trigger );

		//  TODO: list for ordering and non-standard controllers.
	};

	//  Structure to keep track of a scheduled SBM command
	//  Needed because the entries of srCmdSeq are not editable,
	//  and these commands might need to be shifted slightly.
	struct SbmCommand {
		std::string command;
		float       time;

		SbmCommand( std::string & command, float time );

		//  Copy constructor and assignment operator
		SbmCommand( SbmCommand& other );
		SbmCommand& operator= (const SbmCommand& other );
	};

	class BehaviorRequest {
    ///////////////////////////////////////////////////////////////////
    //  Data
	protected:
		//const GestureType type;
		//const void*       data;

		//  SynchPoints in BML request time
		const SynchPoint* start;
		const SynchPoint* ready;
		const SynchPoint* stroke;
		const SynchPoint* relax;
		const SynchPoint* end;

        // controller local time references
        time_sec startTime;
        time_sec readyTime;
        time_sec strokeTime;
        time_sec relaxTime;
        time_sec endTime;
        time_sec speed;  

	private:
        time_sec audioOffset;

    ///////////////////////////////////////////////////////////////////
    //  Methods
	public:
		BehaviorRequest( //const GestureType type, const void* data,
			            const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end,
						time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime,
						float speed );
		virtual ~BehaviorRequest();

		virtual time_sec getAudioRelativeStart();
		virtual time_sec calcAudioRelativeStart();

		/**
		 *   Simplified behavior scheduling method, only offers start time (assumes default duration
		 *   params:
		 *     SbmCharacter* actor: access to character state
		 *     MeCtSchedulerClass* scheduler: new schedule for speech act being built
		 *     float startAt: time to start behavior
		 */
		virtual void schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
			                   VecOfVisemeData& visemes,
		                       VecOfSbmCommand& commands,
                               time_sec startAt );

		/**
		 *   Behavior scheduling method, only offers start time
		 *   params:
		 *     SbmCharacter* actor: access to character state
		 *     MeCtSchedulerClass* scheduler: new schedule for speech act being built
		 *     float startAt/readyAt/strokeAt/relaxAt/endAt: behavior transition times
		 */
        virtual void schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
			                   VecOfVisemeData& visemes,
		                       VecOfSbmCommand& commands,
                               time_sec startAt, time_sec readyAt, time_sec strokeAt, time_sec relaxAt, time_sec endAt ) = 0;
	};

	class MeControllerRequest : public BehaviorRequest {
	public: ///// Constants
		enum TrackType { UTTERANCE, POSTURE } ;
	protected: // Data
		TrackType     trackType;
		MeController *controller;

	public: ///// Methods
		MeControllerRequest( TrackType trackType, MeController *controller, 
			                 const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end );
		virtual ~MeControllerRequest();

        virtual void schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
		                       VecOfVisemeData& visemes,
							   VecOfSbmCommand& commands,
                               time_sec startAt, time_sec readyAt, time_sec strokeAt, time_sec relaxAt, time_sec endAt );
	};

	class MotionRequest : public MeControllerRequest {
	public:
		MotionRequest( MeCtMotion* motion,
			           const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end );
	};

	class NodRequest : public MeControllerRequest {
	public: ///// Constants
		enum NodType { HORIZONTAL = HEAD_SHAKE,
			           VERTICAL   = HEAD_NOD } ;

    private:
        const NodType type;
        const float repeats;
        const float frequency;
        const float extent;    // % of full extension

	public: ///// Methods
		NodRequest( NodType type, float repeats, float frequency, float extent, const SbmCharacter* actor,
			        const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end );
	};

	class TiltRequest : public MeControllerRequest {
    private:
        time_sec duration;
        time_sec transitionDuration;

	public: ///// Methods
		TiltRequest( MeCtSimpleTilt* tilt, time_sec transitionDuration,
			         const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end );
	};

	class PostureRequest : public MeControllerRequest {
	public:
		//MeCtPose* pose;
        time_sec duration;
        time_sec transitionDuration;

		PostureRequest( MeController* pose, time_sec transitionDuration,
			            const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end );
	};

	class VisemeRequest : public BehaviorRequest {
	protected:
        const char* viseme;
        float       weight;
		time_sec    duration;
		float		rampup;
		float		rampdown;

	public:
		VisemeRequest( const char *viseme, float weight, time_sec duration,
			           const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end);

		VisemeRequest( const char *viseme, float weight, time_sec duration,
			           const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end , float rampup, float rampdown);

        void setVisemeName( const char* viseme );

		void schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
		               VecOfVisemeData& visemes,
		               VecOfSbmCommand& commands,
			           double startAt, double readyAt, double strokeAt, double relaxAt, double endAt );
	};

	class EventRequest : public BehaviorRequest {
	protected:
		const std::string message;
	
	public:
		EventRequest( const char* message,
			          const SynchPoint* start, const SynchPoint* ready, const SynchPoint* stroke, const SynchPoint* relax, const SynchPoint* end);
	
		void schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
		               VecOfVisemeData& visemes,
		               VecOfSbmCommand& commands,
			           double startAt, double readyAt, double strokeAt, double relaxAt, double endAt );
	};

	class SpeechRequest {  // not a standard behavior
		///////////////////////////////////////////////////////////////
		// Data
		const DOMElement* xml;
		const XMLCh*      id;

	public:
		// Overrides (but equivalent to) BehaviorRequest fields  (Needed early reference)
		SynchPoint*  start;
		SynchPoint*  ready;
		SynchPoint*  relax;
		SynchPoint*  end;

		TriggerEvent*  trigger;



		///////////////////////////////////////////////////////////////
		// Methods
		SpeechRequest( DOMElement*, const XMLCh* id, TriggerEvent* trigger );
		virtual ~SpeechRequest();

		const DOMElement* getXML() { return xml; }


		//
		///**
		// *  Adds wordbreak mark after STROKE or last mark, and before RELAX
		// */
		//SynchPoint* addMark( const XMLCh* id );


		/**
		 *  Retrieves previously added mark.
		 *  Takes both 'behavior:mark' and just 'mark'.
		 */
		SynchPoint* getMark( const XMLCh* id );
	};
}

#endif  // BML_HPP