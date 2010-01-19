/*
 *  behavior_scheduler_linear.hpp - part of SmartBody-lib
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

#ifndef LINEAR_BEHAVIOR_SCHEDULER_HPP
#define LINEAR_BEHAVIOR_SCHEDULER_HPP

#include "behavior_scheduler.hpp"
#include "ME/me_controller.h"


/**
 *  Transitional mode for calculating the behavior schedules by
 *  durations instead of a fictious local timeline.
 */
#define BEHAVIOR_TIMING_BY_DURATION (0)


namespace BML {
	/**
	 *  Schedules a behavior with a linearly adjustable duration.
	 *  That is, the proportions between sync points will always remain constant.
	 *  Often used by controller-based behaviors.
	 */
	class BehaviorSchedulerConstantSpeed : public BehaviorScheduler {
		public:
#if BEHAVIOR_TIMING_BY_DURATION
	        // preferred durations between sync points
		    time_sec startReadyDur;
			time_sec readyStrokeDur;
	        time_sec strokeRelaxDur;
		    time_sec relaxEndDur;
#else
	        // preferred "local times" of sync points
		    time_sec startTime;
			time_sec readyTime;
	        time_sec strokeTime;
		    time_sec relaxTime;
			time_sec endTime;
#endif // BEHAVIOR_TIMING_BY_DURATION

	        time_sec speed;  

		public:
#if BEHAVIOR_TIMING_BY_DURATION
	        // preferred durations between sync points
			BehaviorSchedulerConstantSpeed(
				time_sec startReadyDur,
				time_sec readyStrokeDur,
				time_sec strokeRelaxDur,
				time_sec relaxEndDur,
				time_sec speed );
#else
	        // preferred "local times" of sync points
			BehaviorSchedulerConstantSpeed(
				time_sec startTime,
				time_sec readyTime,
				time_sec strokeTime,
				time_sec relaxTime,
				time_sec endTime,
				time_sec speed );
#endif // BEHAVIOR_TIMING_BY_DURATION

			virtual void schedule( SyncPoints& syncs, time_sec now );
	};
	typedef boost::shared_ptr<BehaviorSchedulerConstantSpeed> BehaviorSchedulerConstantSpeedPtr;

	/**
	 *  Build a linear BehaviorScheduler using the metadata of a MeController.
	 */
	BehaviorSchedulerConstantSpeedPtr buildSchedulerForController( MeController* ct );

} // namespace BML

#endif // LINEAR_BEHAVIOR_SCHEDULER_HPP
