/*
 *  behavior_scheduler_fixed.hpp - part of SmartBody-lib
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

#ifndef BEHAVIOR_SCHEDULER_FIXED_HPP
#define BEHAVIOR_SCHEDULER_FIXED_HPP

#include "behavior_scheduler.hpp"
#include "ME/me_controller.h"




namespace BML {
	/**
	 *  Schedules a behavior with a linearly adjustable duration.
	 *  That is, the proportions between sync points will always remain constant.
	 *  Often used by controller-based behaviors.
	 */
	class BehaviorSchedulerFixed : public BehaviorScheduler {
		public:
			BehaviorSchedulerFixed( /* TODO */ );
			virtual void schedule( SyncPoints& syncs, time_sec now );
	};
	typedef boost::shared_ptr<BehaviorSchedulerFixed> BehaviorSchedulerFixedPtr;

} // namespace BML

#endif // BEHAVIOR_SCHEDULER_FIXED_HPP
