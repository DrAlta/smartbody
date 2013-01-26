/*
 *  me_utilities.hpp - part of SmartBody-lib
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
 *      Marcus Thiebaux, USC
 */

#ifndef ME_UTILITIES_HPP
#define ME_UTILITIES_HPP


#include <sk/sk_skeleton.h>
#include <sk/sk_motion.h>
#include <sk/sk_posture.h>

#include "sr_path_list.h"
#include "sr_hash_map.h"
#include "sbm/mcontrol_util.h"
#include "mcontrol_callbacks.h"

#include <boost/filesystem/path.hpp>


SkSkeleton* load_skeleton( const char *filename, srPathList &path_list, double scale = 1.0 );

int load_me_motion_individual( SrInput & input, const std::string & motionName, std::map<std::string, SkMotion*>& map, double scale = 1.0 );
int load_me_motions( const char* pathname, std::map<std::string, SkMotion*>& map, bool recursive, double scale = 1.0 );
int load_me_skeleton_individual( SrInput & input, const std::string & skeletonName, std::map<std::string, SkSkeleton*>& map, double scale = 1.0 );
int load_me_skeletons( const char* pathname, std::map<std::string, SkSkeleton*>& map, bool recursive, double scale = 1.0 );

// For Posture scaling, using skmScale for now
int load_me_postures( const char* pathname, std::map<std::string, SkPosture*>& map, bool recursive, double scale = 1.0 );

void print_joint( const SkJoint* joint );


#endif // ME_UTILITIES_HPP
