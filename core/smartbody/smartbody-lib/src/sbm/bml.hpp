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
#include "bml_sync_point.hpp"
#include "behavior_span.hpp"
#include "behavior_scheduler.hpp"
#include "behavior_scheduler_constant_speed.hpp" // temporary, for BEHAVIOR_TIMING_BY_DURATION macro

#include "mcontrol_util.h"
#include "sbm_character.hpp"

#include "xercesc_utils.hpp"

// Motion Engine & Controllers...
#include <ME/me_ct_motion.h>
#include "me_ct_examples.h"
#include <ME/me_prune_policy.hpp>

#include "bml_sync_point.hpp"

const bool LOG_BML_VISEMES	= false;
const bool LOG_AUDIO		= false;

/**
 *  Enables a argument-level compatibility mode of vrAgentBML.
 *  When enabled, vrAgentBML requires a recipient token after the agent name,
 *  just like the original vrSpeak.
 */
#define VRAGENTBML_USES_RECIPIENT (0)


namespace BML {
	//  Helper Function
	std::wstring buildBmlId( const std::wstring& behavior_id, const std::wstring& sync_id );
	bool isValidBmlId( const std::wstring& id );
	bool isValidTmId( const std::wstring& id );


	// Enumerations
	enum HeadBehaviorType { HEAD_NOD, HEAD_SHAKE, HEAD_TOSS, HEAD_ORIENT };



	// Class Definitions
	/** Base class for all BML exceptions. */
	class BmlException : public std::exception {
	public:
		BmlException( const char *const& what_string )
		:	std::exception( what_string )
		{}

		virtual const char* type()
		{	return "BML::BmlException"; }
	};

	/** Exception of errors that occur during parsing. */
	class ParsingException : public BmlException {
	public:
		ParsingException( const char *const& what_string )
		:	BmlException( what_string )
		{}

		virtual const char* type()
		{	return "BML::ParsingException"; }
	};
	/** Exception of errors that occur during scheduling. */
	class SchedulingException : public BmlException {
	public:
		SchedulingException( const char *const& what_string )
		:	BmlException( what_string )
		{}

		virtual const char* type()
		{	return "BML::SchedulingException"; }
	};
	/** Exception of errors that occur during realizing. */
	class RealizingException : public BmlException {
	public:
		RealizingException( const char *const& what_string )
		:	BmlException( what_string )
		{}

		virtual const char* type()
		{	return "BML::RealizingException"; }
	};

	class BmlRequest {
	public:
		const SbmCharacter   *actor;
		const std::string    actorId;  // in case referenced via an alias

		const std::string    requestId;
#if VRAGENTBML_USES_RECIPIENT
		const std::string    recipientId;
#endif
		const std::string    msgId;

		VecOfTriggerEvent    triggers;
		TriggerEventPtr      start_trigger;
		SyncPointPtr         bml_start;
		// SyncPointPtr        bml_end;  // bml:end SyncPoint removed until it can be better evaluated

		VecOfBehaviorRequest behaviors;
		MapOfBehaviorRequest idToBehavior;

		MapOfSyncPoint       idToSync;

		TriggerEventPtr      speech_trigger;
		SpeechRequestPtr     speech_request;

	private:
		bool                 required;

		std::string			 start_seq_name;
		std::string			 cleanup_seq_name;

		BmlRequestWeakPtr    weak_ptr;  // weak reference to the reference count struct
		BehaviorSpan         span;


	protected:
#if VRAGENTBML_USES_RECIPIENT
		BmlRequest( const SbmCharacter* agent, const std::string& actorId, const std::string& requestId, const std::string& recipientId, const std::string & msgId );
#else
		BmlRequest( const SbmCharacter* agent, const std::string& actorId, const std::string& requestId, const std::string & msgId );
#endif
		void init( BmlRequestPtr self );

		std::string buildUniqueBehaviorId( const XMLCh* tag, const XMLCh* id, size_t ordinal );

		bool hasExistingBehaviorId( const std::wstring& id );
		void importNamedSyncPoints( SequenceOfNamedSyncPoints& sync_seq, const std::wstring& id, const std::wstring& logging_label );

		BehaviorSpan getBehaviorSpan();

		/**
		 *  Schedules and realizes teh behaviors of the BmlRequest.
		 *  May throw BML::RealizationException if error occurs or request is unschedulable.
		 */
		void realize( Processor* bp, mcuCBHandle *mcu );

		void unschedule( Processor* bp, mcuCBHandle* mcu, time_sec transition_duration );

		void cleanup( Processor* bp, mcuCBHandle* mcu );

	public:
		virtual ~BmlRequest();

		/**
		 *  Creates a new TriggerEvent following this BmlRequest's start_trigger.
		 *  DO NOT CALL BEFORE CONSTRUCTOR RETURNS.
		 */
		TriggerEventPtr createTrigger( const std::wstring &name );

		bool registerBehavior( const std::wstring& id, BehaviorRequestPtr behavior );
//		bool registerBehavior( const std::wstring& id, SpeechRequestPtr behavior );

		/**
		 *  Gets the TimeRange for all scheduled SequenceOfNamedSyncPoints in all BehaviorRequests.
		 */
		SyncPointPtr getSyncPoint( const std::wstring& notation );  // Lookup a SyncPoint
		
		bool isPersistent() { return getBehaviorSpan().persistent; }

	protected:
		friend class Processor;
	};

	//  Structure to keep track of a scheduled SBM command
	//  Needed because the entries of srCmdSeq are not editable,
	//  and these commands might need to be shifted slightly.
	struct SbmCommand {
		std::string command;
		time_sec    time;

		SbmCommand( std::string & command, time_sec time );

		//  Copy constructor and assignment operator
		SbmCommand( SbmCommand& other );
		SbmCommand& operator= (const SbmCommand& other );
	};

	class BehaviorRequest {
    ///////////////////////////////////////////////////////////////////
    //  Constants
	public:
		static const time_sec PERSISTENCE_THRESHOLD;

	///////////////////////////////////////////////////////////////////
    //  Data
	public:
		const std::string         unique_id;
		bool                      required;
		SequenceOfNamedSyncPoints sync_seq;
		BehaviorSchedulerPtr      scheduler;

	private:
        time_sec audioOffset;

    ///////////////////////////////////////////////////////////////////
    //  Methods
	public:
		BehaviorRequest( const std::string& unique_id, const SequenceOfNamedSyncPoints& sync_seq );
		virtual ~BehaviorRequest();

		void set_scheduler( BehaviorSchedulerPtr scheduler );

		/**
		 *  Schedules the behavior's SyncPoints, returning the earliest time (usually the start time).
		 *  May throw RealizationException.
		 */
		virtual void schedule( time_sec now );

		/**
		 *   Simplified behavior scheduling method, only offers start time (assumes default duration
		 *   params:
		 *     SbmCharacter* actor: access to character state
		 *     float startAt: time to start behavior
		 */
		virtual void realize( BmlRequestPtr request, mcuCBHandle* mcu );

		/**
		 *   Behavior scheduling method.
		 *   Reads scheduled times from SequenceOfNamedSyncPoints.
		 */
        virtual void realize_impl( BmlRequestPtr request, mcuCBHandle* mcu ) = 0;

		/**
		 *  returns true is behaviors involves controllers
		 *  Temporary transition in pruning algorithm
		 */
		virtual bool has_cts() { return false; }

		/**
		 *  Registers the give prune policy with all controllers of this behavior.
		 *  Does nothing by default.
		 **/
		virtual void registerControllerPrunePolicy( MePrunePolicy* prune_policy ) {};

		/**
		 *  Returns true if the continues to influence the actor beyond
		 *  the end of the BehaviorSpan.  Usually, such influence is part
		 *  of a maintainence tasks such as a posture or gaze/pointing
		 *  target tracking.
		 *
		 *  Default heuristic looks for far future sync points.
		 *  It's highly recommended to override with a simple boolean
		 *  value if you know it ahead of time.
		 */
		virtual bool isPersistent();

		/**
		 *  Gets the BehaviorSpan for all scheduled SyncPoints.
		 *
		 *  Default algorithm attempts to detect sync points in the
		 *  far future.
		 */
		virtual BehaviorSpan getBehaviorSpan();

		/**
		 *  Schedules a cancelation / interruption of the behavior
		 *  at specified time and duration.
		 */
		virtual void unschedule( mcuCBHandle* mcu,
		                         BmlRequestPtr request,
			                     time_sec duration ) = 0;

		/**
		 *  Abstract method point for implementing the specific clean-up algorithm.
		 *  May be called without calling unschedule, or called multiple times.
		 */
		virtual void cleanup( mcuCBHandle* mcu, BmlRequestPtr request ) {};
	};

	class MeControllerRequest : public BehaviorRequest {
	public:
		enum SchduleType { LINEAR, MANUAL };

	protected: // Data
		/** Controller holding the source of the animation (not necessarily a motion or animation). */
		MeController*            anim_ct;
		/** The schedule controller the animation was added to. */
		MeCtScheduler2*          schedule_ct;

		bool                     persistent;

	public: ///// Methods
		MeControllerRequest( const std::string& unique_id,
		                     MeController *anim_ct,
							 MeCtSchedulerClass* schedule_ct,
			                 const SequenceOfNamedSyncPoints& sync_seq,
							 MeControllerRequest::SchduleType sched_type = LINEAR );
		virtual ~MeControllerRequest();

		/**
		 *  returns true is behaviors involves controllers
		 *  Temporary transition in pruning algorithm
		 */
		virtual bool has_cts()
		{	return true; }

		/**
		 *  Returns true if the continues to influence the actor beyond
		 *  the end of the BehaviorSpan.  Usually, such influence is part
		 *  of a maintainence tasks such as a posture or gaze/pointing
		 *  target tracking.
		 *
		 *  Default heuristic looks for far future sync points.
		 *  It's highly recommended to override with a simple boolean
		 *  value if you know it ahead of time.
		 */
		bool is_persistent()
		{	return persistent; }

		void set_persistent( bool new_value )
		{	persistent = new_value; }

		/**
		 *  Registers the give prune policy with all controllers of this behavior.
		 **/
		virtual void register_controller_prune_policy( MePrunePolicy* prune_policy );

		virtual void realize_impl( BmlRequestPtr request, mcuCBHandle* mcu );

		/**
		 *  Implemtents BehaviorRequest::unschedule(..),
		 *  ramping down the blend curve of the MeController.
		 */
		virtual void unschedule( mcuCBHandle* mcu, BmlRequestPtr request,
			                     time_sec duration );

		/**
		 *  Implemtents BehaviorRequest::cleanup(..),
		 *  removing the MeController from its parent.
		 */
		virtual void cleanup( mcuCBHandle* mcu, BmlRequestPtr request );
	};

	class MotionRequest : public MeControllerRequest {
	public:
		MotionRequest( const std::string& unique_id, MeCtMotion* motion_ct, MeCtSchedulerClass* schedule_ct,
			           const SequenceOfNamedSyncPoints& sync_seq );
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
		NodRequest( const std::string& unique_id, NodType type, float repeats, float frequency, float extent, const SbmCharacter* actor,
			        const SequenceOfNamedSyncPoints& sync_seq );
	};

	class TiltRequest : public MeControllerRequest {
    private:
        time_sec duration;
        time_sec transitionDuration;

	public: ///// Methods
		TiltRequest( const std::string& unique_id, MeCtSimpleTilt* tilt, time_sec transitionDuration, const SbmCharacter* actor,
			         const SequenceOfNamedSyncPoints& sync_seq );
	};

	class PostureRequest : public MeControllerRequest {
	public:
        time_sec duration;
        time_sec transitionDuration;

		PostureRequest( const std::string& unique_id, MeController* pose, time_sec transitionDuration, const SbmCharacter* actor,
			            const SequenceOfNamedSyncPoints& sync_seq );
	};

	class SequenceRequest : public BehaviorRequest {
	protected:
		SequenceRequest( const std::string& unique_id, const SequenceOfNamedSyncPoints& sync_seq,
						 time_sec startTime, time_sec readyTime, time_sec strokeTime, time_sec relaxTime, time_sec endTime );

	public:
		/**
		 *  Implemtents BehaviorRequest::unschedule(..),
		 *  cancelling remaining sequence.
		 */
		virtual void unschedule( mcuCBHandle* mcu, BmlRequestPtr request,
			                     time_sec duration );

		/**
		 *  Implemtents BehaviorRequest::cleanup(..),
		 *  removing the sequence.
		 */
		virtual void cleanup( mcuCBHandle* mcu, BmlRequestPtr request );

	protected:
		/**
		 *  Builds and activates a sequence from commands list.
		 */
		bool realize_sequence( VecOfSbmCommand& commands, mcuCBHandle* mcu );

		/**
		 *  Aborts the prior sequence.
		 */
		bool unschedule_sequence( mcuCBHandle* mcu );
	};

	class VisemeRequest : public SequenceRequest {
	protected:
        const char* viseme;
        float       weight;
		time_sec    duration;
		float		rampup;
		float		rampdown;

	public:
		VisemeRequest( const std::string& unique_id, const char *viseme, float weight, time_sec duration,
			           const SequenceOfNamedSyncPoints& sync_seq );

		VisemeRequest( const std::string& unique_id, const char *viseme, float weight, time_sec duration,
			           const SequenceOfNamedSyncPoints& sync_seq, float rampup, float rampdown);

        void setVisemeName( const char* viseme );

		void realize_impl( BmlRequestPtr request, mcuCBHandle* mcu );
	};
} // namespace BML

#endif  // BML_HPP