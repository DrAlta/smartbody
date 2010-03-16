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
		private:
			// Ordered list of SynchPoint names to time in seconds after start
			//   include "start" => 0?
			std::vector<std::pair<std::wstring,float>> sync_point_times;

			// map of sync_point id to the index of the data
			std::map<std::wstring,unsigned int> sync_id2index;

		public:
			BehaviorSchedulerFixed( const std::vector<std::pair<std::wstring,float>>& sync_point_pairs );

			virtual void schedule( SyncPoints& syncs, time_sec now );

		protected:
			/** Compares SyncPoints orderings to provided fixed orderings.
			 *  Throws SchedulingException if order does not match.
			 */
			void validate_ordering( SyncPoints& syncs );
	};
	typedef boost::shared_ptr<BehaviorSchedulerFixed> BehaviorSchedulerFixedPtr;

} // namespace BML

#endif // BEHAVIOR_SCHEDULER_FIXED_HPP
