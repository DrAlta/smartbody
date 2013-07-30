/*
 *  action_unit.hpp - part of SmartBody-lib
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

#ifndef SBM_ActionUnit_HPP
#define SBM_ActionUnit_HPP

#include <map>

#include <sr/sr_shared_class.h>
#include <sr/sr_shared_ptr.hpp>
#include <sk/sk_motion.h>


/**
 *  Maps the a Facial Action Coding (FAC) Action Unit
 *  to some asset type.
 *
 *  Currently we use the first frame of SkMotion because
 *  of limitations in our exports (can't export direct to .skp).
 *
 *  The unit number is not stored internally.  It can be found
 *  in the ActionUnitMap as a std::pair< int, ActionUnit >.
 */
class ActionUnit
{
public:
	ActionUnit(SkMotion* unified );
	ActionUnit( SkMotion* left, SkMotion* right );
	ActionUnit(ActionUnit* source);
	~ActionUnit();

	SBAPI bool is_bilateral() const;
	void reset_type();
	void set_left();
	void set_bilateral();
	void set_right();
	SBAPI bool is_left() const;
	SBAPI bool is_right() const;
	void set( SkMotion* motion );
	void set( SkMotion* left, SkMotion* right );

	SkMotion* left;
	SkMotion* right;

	protected:
		bool m_isLeft;
		bool m_isRight;
		bool m_isBilateral;
};

#endif // SBM_ActionUnit_HPP
