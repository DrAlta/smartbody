/*
 *  behavior_scheduler_constant_speed.cpp - part of SmartBody-lib
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
 */

#include "bml.hpp"
#include "behavior_scheduler_constant_speed.hpp"

using namespace std;
using namespace BML;


const bool LOG_ABNORMAL_SPEED = false;



/////////////////////////////////////////////////////////////////////////////
//  BehaviorSchedulerLinear
//


BehaviorSchedulerConstantSpeed::BehaviorSchedulerConstantSpeed(
#if BEHAVIOR_TIMING_BY_DURATION
    // preferred durations between sync points
    time_sec startReadyDur,
	time_sec readyStrokeDur,
    time_sec strokeRelaxDur,
    time_sec relaxEndDur,
#else
    // preferred "local times" of sync points
    time_sec startTime,
	time_sec readyTime,
    time_sec strokeTime,
    time_sec relaxTime,
	time_sec endTime,
#endif // BEHAVIOR_TIMING_BY_DURATION

    time_sec speed
) :
#if BEHAVIOR_TIMING_BY_DURATION
    // preferred durations between sync points
    startReadyDur( startReadyDur ),
	readyStrokeDur( readyStrokeDur ),
    strokeRelaxDur( strokeRelaxDur ),
    relaxEndDur( relaxEndDur ),
#else
    // preferred "local times" of sync points
    startTime( startTime ),
	readyTime( readyTime ),
    strokeTime( strokeTime ),
    relaxTime( relaxTime ),
	endTime( endTime ),
#endif // BEHAVIOR_TIMING_BY_DURATION
	speed( speed )
{}



/** Tests ordering.  If invlaid, prints warning about ignoring SyncPoint before. */
bool testSyncBefore(
		SyncPointPtr& before, const char* before_name,
		SyncPointPtr& after, const char* after_name
) {
	if( before->time <= after->time ) {
		return true;
	} else {
        clog << "WARNING: BehaviorRequest::testSyncOrder(): "<<before_name<<" NOT before "<<after_name<<"... ignoring "<<before_name<<"." << endl;
		return false;
	}
}

/** Tests ordering.  If invlaid, prints warning about ignoring SyncPoint after. */
bool testSyncAfter(
		SyncPointPtr& before, const char* before_name,
		SyncPointPtr& after, const char* after_name
) {
	if( before->time <= after->time ) {
		return true;
	} else {
        clog << "WARNING: BehaviorRequest::testSyncOrder(): "<<before_name<<" NOT before "<<after_name<<"... ignoring "<<after_name<<"." << endl;
		return false;
	}
}



void BehaviorSchedulerConstantSpeed::schedule( BehaviorSyncPoints& sync_seq, time_sec now ) {
	// local references to standard sync points
	SyncPointPtr start        = sync_seq.sync_start()->sync();
	SyncPointPtr ready        = sync_seq.sync_ready()->sync();
	SyncPointPtr stroke_start = sync_seq.sync_stroke_start()->sync();
	SyncPointPtr stroke       = sync_seq.sync_stroke()->sync();
	SyncPointPtr stroke_end   = sync_seq.sync_stroke_end()->sync();
	SyncPointPtr relax        = sync_seq.sync_relax()->sync();
	SyncPointPtr end          = sync_seq.sync_end()->sync();

	/*  The following implements a search for the two most important SyncPoints, and then scales the time to meet both.
     *  Importance is ranked in this order: stroke, ready, relax, start, end
     *  If only one SyncPoint is found, the controller maintains its natural duration.
     *  If no sync points are found, the behavior starts immediately.
     */
    bool hasStroke = isTimeSet( stroke->time );
    bool hasReady  = isTimeSet( ready->time );
    bool hasRelax  = isTimeSet( relax->time );
    bool hasStart  = isTimeSet( start->time );
    bool hasEnd    = isTimeSet( end->time );

	time_sec start_at = TIME_UNSET;


    if( hasStroke ) {  // Handle stroke first (most important)
        if(    hasReady
			&& testSyncBefore( ready, "ready", stroke, "stroke" )
		) 
		{
			// Stroke and compatible ready
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = stroke->time - ready->time;  // realtime duration
			time_sec bhvrDur  = readyStrokeDur;           // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - startReadyDur/speed;
#else
            if( readyTime >= strokeTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): readyTime NOT before strokeTime... ignoring ready." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = stroke->time - ready->time;  // realtime diff
				time_sec ctlrDiff = strokeTime - readyTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasRelax
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( stroke, "stroke", relax, "relax" )
		)
		{
			// Stroke and compatible relax
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur   = relax->time - stroke->time;  // realtime duration
			time_sec bhvrDur = strokeRelaxDur;              // behavior duration

			speed = bhvrDur / rtDur;
			start_at = stroke->time - (startReadyDur+readyStrokeDur)/speed;
#else
            if( strokeTime >= relaxTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): strokeTime NOT before relaxTime... ignoring relax." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = relax->time - stroke->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - strokeTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasStart
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncBefore( start, "start", stroke, "stroke" )
		)
		{
			// Stroke and compatible start
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur   = stroke->time - start->time;  // realtime duration
			time_sec bhvrDur = strokeRelaxDur;              // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= strokeTime ) {
                //clog << "WARNING: MeControllerRequest::calcAudioRelativeStart(): startTime NOT before strokeTime... ignoring start." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = stroke->time - start->time;  // realtime diff
				time_sec ctlrDiff = strokeTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif  // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasEnd
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( stroke, "stroke", end, "end" )
		)
		{
			// Stroke and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - stroke->time;      // realtime diff
			time_sec bhvrDur = strokeRelaxDur + relaxEndDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = stroke->time - (startReadyDur+readyStrokeDur)/speed;
#else
            if( strokeTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): strokeTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and stroke.
				time_sec rtDiff = end->time - stroke->time;  // realtime diff
				time_sec ctlrDiff = endTime - strokeTime;
				speed = ctlrDiff / rtDiff;

				start_at = stroke->time - (strokeTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only stroke
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = stroke->time - ( startReadyDur + readyStrokeDur );
#else
			start_at = stroke->time - strokeTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
    } else if( hasReady ) {  // Next comes ready
		// Skipping stroke. Would have been caught at previous level.
        if(    hasRelax
			&& testSyncAfter( ready, "ready", relax, "relax" )
		)
		{
			// Ready and compatible relax
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = relax->time - ready->time;        // realtime diff
			time_sec bhvrDur = readyStrokeDur + strokeRelaxDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - (startReadyDur)/speed;
#else
            if( readyTime >= relaxTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): readyTime NOT before relaxTime... ignoring relax." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = relax->time - ready->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - readyTime;
				speed = ctlrDiff / rtDiff;

				start_at = ready->time-(readyTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasStart
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncBefore( start, "start", ready, "ready" )
		)
		{
			// Ready and compatible start
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = relax->time - ready->time;        // realtime diff
			time_sec bhvrDur = readyStrokeDur + strokeRelaxDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= readyTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): startTime NOT before readyTime... ignoring start." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = ready->time - start->time;  // realtime diff
				time_sec ctlrDiff = readyTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = ready->time-(readyTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasEnd
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( ready, "ready", end, "end" )
		)
		{
			// Ready and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - ready->time;        // realtime diff
			time_sec bhvrDur = readyStrokeDur + strokeRelaxDur + relaxEndDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - (startReadyDur)/speed;
#else
            if( readyTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): readyTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and ready.
				time_sec rtDiff = end->time - ready->time;  // realtime diff
				time_sec ctlrDiff = endTime - readyTime;
				speed = ctlrDiff / rtDiff;

				start_at = ready->time-(readyTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only ready
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = ready->time - startReadyDur;
#else
			start_at = ready->time - readyTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
    } else if( hasRelax ) {  // Next comes relax
		// Skipping stroke and ready. Would have been caught at previous level.
        if(    hasStart
			&& testSyncBefore( start, "start", relax, "relax" )
		)
		{
			// Relax and compatible start
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = relax->time - start->time;                         // realtime diff
			time_sec bhvrDur = startReadyDur + readyStrokeDur + strokeRelaxDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= relaxTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): startTime NOT before relaxTime... ignoring start." << endl;
			} else {
				// adjust speed to start and relax.
				time_sec rtDiff = relax->time - start->time;  // realtime diff
				time_sec ctlrDiff = relaxTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = relax->time-(relaxTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
        }
		if(    hasEnd
			&& !isTimeSet( start_at )  // second sync point or previous failed
			&& testSyncAfter( relax, "relax", end, "end" )
		)
		{
			// Relax and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - relax->time; // realtime diff
			time_sec bhvrDur = relaxEndDur;           // behavior duration

			speed = bhvrDur / rtDur;
			start_at = ready->time - (startReadyDur)/speed;
#else
            if( relaxTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): relaxTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and relax.
				time_sec rtDiff = end->time - relax->time;  // realtime diff
				time_sec ctlrDiff = endTime - relaxTime;
				speed = ctlrDiff / rtDiff;

				start_at = relax->time-(relaxTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only relax
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = relax->time - ( startReadyDur + readyStrokeDur + strokeRelaxDur );
#else
			start_at = relax->time - relaxTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
    } else if( hasStart ) {  // Next comes start
		// Skipping stroke, ready and relax.  Would have been caught at previous level.
        if(    hasEnd
			&& testSyncAfter( start, "start", end, "end" )
		)
		{
			// Start and compatible end
#if BEHAVIOR_TIMING_BY_DURATION
			// adjust speed to start and stroke.
			time_sec rtDur = end->time - start->time;                                         // realtime diff
			time_sec bhvrDur = startReadyDur + readyStrokeDur + strokeRelaxDur + relaxEndDur; // behavior duration

			speed = bhvrDur / rtDur;
			start_at = start->time;
#else
            if( startTime >= endTime ) {
                //clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): startTime NOT before endTime... ignoring end." << endl;
			} else {
				// adjust speed to start and start.
				time_sec rtDiff = end->time - start->time;  // realtime diff
				time_sec ctlrDiff = endTime - startTime;
				speed = ctlrDiff / rtDiff;

				start_at = start->time - (startTime/speed);
			}
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
		if( !isTimeSet( start_at ) ) {
			// Only start
			speed = 1;
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = start->time;
#else
			start_at = start->time - startTime;  // Strange....
#endif // BEHAVIOR_TIMING_BY_DURATION
		}
	} else {
		speed = 1;  // End or nothing
		if( hasEnd ) {  // Last comes end
			// Only end
#if BEHAVIOR_TIMING_BY_DURATION
			start_at = end->time - (startReadyDur + readyStrokeDur + strokeRelaxDur + relaxEndDur);
#else
			start_at = end->time - endTime;
#endif // BEHAVIOR_TIMING_BY_DURATION
		} else {
			// No sync_points... align to audio
			//clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): no valid time refences." << endl;
			start_at = now;
		}
	}

    // Offset all times by startAt-startTime and scale to previously determined speed
	if( !start->is_set() )
		start->time  = start_at;
#if BEHAVIOR_TIMING_BY_DURATION
	if( !ready->isSet() )
		ready->time  = start->time  + startReadyDur/speed;
	if( !stroke->isSet() )
		stroke->time = ready->time  + readyStrokeDur/speed;
	if( !relax->isSet() )
		relax->time  = stroke->time + strokeRelaxDur/speed;
	if( !end->isSet() )
		end->time    = relax->time  + relaxEndDur/speed;
#else
	if( !ready->is_set() )
		ready->time  = start_at + (readyTime-startTime)/speed;
	if( !stroke->is_set() )
		stroke->time = start_at + (strokeTime-startTime)/speed;
	if( !relax->is_set() )
		relax->time  = start_at + (relaxTime-startTime)/speed;
	if( !end->is_set() )
		end->time    = start_at + (endTime-startTime)/speed;
#endif // BEHAVIOR_TIMING_BY_DURATION

	// TODO: validate times are set and in order

	if( LOG_ABNORMAL_SPEED ) {
		if( speed > 2 )
			clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): speed " << speed << " is unusually fast.  Try removing end sync constraint." << endl;
		else if( speed < 0.3 )
			clog << "WARNING: BehaviorRequest::calcAudioRelativeStart(): speed " << speed << " is unusually slow.  Try removing end sync constraint." << endl;
	}
}




// Part of a transitional move to BehaviorSchedulers
BehaviorSchedulerConstantSpeedPtr BML::buildSchedulerForController( MeController* ct ) {
	bool is_persistent = ct->controller_duration() < 0;

#if BEHAVIOR_TIMING_BY_DURATION
	BehaviorSchedulerConstantSpeedPtr scheduler( 
		new BehaviorSchedulerConstantSpeed(
			/* startReadyDur  */ time_sec( ct->indt() ), 
			/* readyStrokeDur */ time_sec( ct->emphasist() - ct->indt() ), 
			/* strokeRelaxDur */ time_sec( is_persistent? numeric_limits<time_sec>::max() : ct->controller_duration() - ct->emphasist() - ct->outdt() ),
			/* relaxEndDur    */ time_sec( ct->outdt() ),
			/* speed      */ 1 ) );

#else

	BehaviorSchedulerConstantSpeedPtr scheduler( 
		new BehaviorSchedulerConstantSpeed(
			/* startTime  */ 0, 
			/* readyTime  */ time_sec( ct->indt() ), 
			/* strokeTime */ time_sec( ct->emphasist() ),
			/* relaxTime  */ time_sec( is_persistent? numeric_limits<time_sec>::max() : ct->controller_duration() - ct->outdt() ), 
			/* endTime    */ time_sec( is_persistent? numeric_limits<time_sec>::max() : ct->controller_duration() ),
			/* speed      */ 1 ) );
#endif  // BEHAVIOR_TIMING_BY_DURATION

	return scheduler;
}
