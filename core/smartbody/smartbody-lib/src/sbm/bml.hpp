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


#include "bml_types.hpp"


//#include "mcontrol_util.h"
#include "sbm_character.hpp"
#include "sbm_speech.hpp"

#include "xercesc_utils.hpp"

// Controllers...
#include <ME/me_ct_motion.h>
#include "me_ct_examples.h"


// Transitionary Build Options
#define SYNC_LINKED_LIST (0)



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


	//  Helper Function
	const XMLCh* buildBmlId( const XMLCh* behaviorId, const XMLCh* synchId );
	bool isValidBmlId( const XMLCh* id );
	bool isValidTmId( const XMLCh* id );


	// Enumerations
//	enum GestureType { BML_MOTION };
	enum HeadBehaviorType { HEAD_NOD, HEAD_SHAKE, HEAD_TOSS, HEAD_ORIENT };



	// Class Definitions
	class BmlRequest {
	public:
		const SbmCharacter   *agent;

		const std::string    requestId;
		const std::string    recipientId;
		const std::string    msgId;

		VecOfTriggerEvent    triggers;
		TriggerEventPtr      start_trigger;
		SynchPointPtr        bml_start;
		// SynchPointPtr        bml_end;  // bml:end SynchPoint removed until it can be better evaluated
		VecOfVisemeData      visemes;
		VecOfBehaviorRequest behaviors;
		MapOfSynchPoint      synch_points;

		TriggerEventPtr      speech_trigger;
		SpeechRequestPtr     speech_request;

		//  TODO: Move these into SpeechRequest
		char*                audioPlay;
		char*                audioStop;

	private:
		BmlRequestWeakPtr    weak_ptr;  // weak reference to the reference count struct


	protected:
		BmlRequest( const SbmCharacter* agent, const std::string & requestId, const std::string & recipientId, const std::string & msgId );
		void init( BmlRequestPtr self );

	public:
		virtual ~BmlRequest();

		/**
		 *  Creates a new TriggerEvent following this BmlRequest's start_trigger.
		 *  DO NOT CALL BEFORE CONSTRUCTOR RETURNS.
		 */
		TriggerEventPtr createTrigger( const std::string &name );

		void addBehavior( BehaviorRequest* behavior );

		SynchPointPtr getSynchPoint( const XMLCh* name );  // Lookup a SynchPoint


		friend class Processor;
	};

	class TriggerEvent {
	public:
		std::string         name;     // for logging / debugging
		BmlRequestWeakPtr   request;

	private:
		TriggerEventWeakPtr weak_ptr;  // weak reference to the reference count struct

	protected:
		TriggerEvent( const std::string& name, BmlRequestPtr request );
		void init( TriggerEventPtr self );

	public:
		SynchPointPtr addSynchPoint( const XMLCh* name );  // adds SynchPoint before end of trigger
#if SYNC_LINKED_LIST
		SynchPointPtr addSynchPoint( const XMLCh* name, SynchPointPtr prev );
		SynchPointPtr addSynchPoint( const XMLCh* name, SynchPointPtr prev, SynchPointPtr par, float off );
#else
		SynchPointPtr addSynchPoint( const XMLCh* name, SynchPointPtr par, float off );
#endif // SYNC_LINKED_LIST


		friend class BmlRequest;
	};

	class SynchPoint {
	public:
		const XMLCh *const        name;
		const TriggerEventWeakPtr trigger;
#if SYNC_LINKED_LIST
		SynchPointPtr             prev;
		SynchPointPtr             next;
#endif  // SYNC_LINKED_LIST
		time_sec                  time;  // TIME_UNSET implies it has not been set
		SynchPointPtr             parent;
		float			          offset;	

	private:
		SynchPointWeakPtr         weak_ptr;  // weak reference to the reference count struct

	protected:
		SynchPoint( const XMLCh* name, const TriggerEventPtr trigger );
		SynchPoint( const XMLCh* name, const TriggerEventPtr trigger, SynchPointPtr par, float off );
#if SYNC_LINKED_LIST
		void init( SynchPointPtr self, SynchPointPtr prev );
#else
		void init( SynchPointPtr self );
#endif  // SYNC_LINKED_LIST

	public:
		virtual ~SynchPoint();


		friend class BmlRequest;
		friend class TriggerEvent;
	};

	// Ordered list of synch points, with references to standard synch_points.
	class SynchPoints {
	public:
		SynchPointPtr start;
		SynchPointPtr ready;
		SynchPointPtr strokeStart;
		SynchPointPtr stroke;
		SynchPointPtr strokeEnd;
		SynchPointPtr relax;
		SynchPointPtr end;

		//SynchPoints();  // Unnecessary with shared_ptr

		void parseStandardSynchPoints( DOMElement* elem, BmlRequestPtr request );
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
		const SynchPointPtr start;
		const SynchPointPtr ready;
		const SynchPointPtr stroke;
		const SynchPointPtr relax;
		const SynchPointPtr end;

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
		BehaviorRequest( const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end,
						 time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime,
						 float speed );
		virtual ~BehaviorRequest();

		virtual time_sec getAudioRelativeStart();
		virtual time_sec calcAudioRelativeStart();

		/**
		 *   Simplified behavior scheduling method, only offers start time (assumes default duration
		 *   params:
		 *     SbmCharacter* actor: access to character state
		 *     MeCtSchedulerClass* scheduler: new schedule for BML request
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
		 *     MeCtSchedulerClass* scheduler: new schedule for BML request
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
			                 const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end );
		virtual ~MeControllerRequest();

        virtual void schedule( const mcuCBHandle* mcu, const SbmCharacter* actor, MeCtSchedulerClass* scheduler,
		                       VecOfVisemeData& visemes,
							   VecOfSbmCommand& commands,
                               time_sec startAt, time_sec readyAt, time_sec strokeAt, time_sec relaxAt, time_sec endAt );
	};

	class MotionRequest : public MeControllerRequest {
	public:
		MotionRequest( MeCtMotion* motion,
			           const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end );
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
			        const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end );
	};

	class TiltRequest : public MeControllerRequest {
    private:
        time_sec duration;
        time_sec transitionDuration;

	public: ///// Methods
		TiltRequest( MeCtSimpleTilt* tilt, time_sec transitionDuration,
			         const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end );
	};

	class PostureRequest : public MeControllerRequest {
	public:
		//MeCtPose* pose;
        time_sec duration;
        time_sec transitionDuration;

		PostureRequest( MeController* pose, time_sec transitionDuration,
			            const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end );
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
			           const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end);

		VisemeRequest( const char *viseme, float weight, time_sec duration,
			           const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end , float rampup, float rampdown);

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
			          const SynchPointPtr start, const SynchPointPtr ready, const SynchPointPtr stroke, const SynchPointPtr relax, const SynchPointPtr end);
	
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
		TriggerEventPtr   trigger;
		SynchPointPtr     start;
		SynchPointPtr     ready;
		SynchPointPtr     relax;
		SynchPointPtr     end;

#if !SYNC_LINKED_LIST
		VecOfSynchPoint   tms;  // <tm> Time Markers, or syntheis markup equiv
#endif // !SYNC_LINKED_LIST



		///////////////////////////////////////////////////////////////
		// Methods
		SpeechRequest( DOMElement*, const XMLCh* id, BmlRequestPtr request );
		virtual ~SpeechRequest();

		const DOMElement* getXML() { return xml; }


		//
		///**
		// *  Adds wordbreak mark after STROKE or last mark, and before RELAX
		// */
		//SynchPointPtr addMark( const XMLCh* id );


		/**
		 *  Retrieves previously added mark.
		 *  Takes both 'behavior:mark' and just 'mark'.
		 */
		SynchPointPtr getMark( const XMLCh* id );
	};
}

#endif  // BML_HPP