/*
 *  sbm_eyeblink.h - part of SmartBody-lib
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
 *      Ed Fast, USC
 */

#ifndef __SBM_EYEBLINK_H
#define __SBM_EYEBLINK_H


#include "mcontrol_util.h"



void SBM_eyeblink_update( BoneBusCharacter * character, double frame_time );

// EDF - NOTE!  This is a global setting, not per character. Will have to be modified to support multiple chars.
void SBM_enable_auto_eyeblink();
void SBM_disable_auto_eyeblink( mcuCBHandle * mcu );



#endif // __SBM_EYEBLINK_H
