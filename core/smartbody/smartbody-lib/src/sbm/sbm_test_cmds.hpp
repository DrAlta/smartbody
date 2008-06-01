/*
 *  sbm_test_cmds.hpp - part of SmartBody-lib
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

#ifndef SBM_TEST_CMDS_HPP
#define SBM_TEST_CMDS_HPP

#include "sr_arg_buff.h"
#include "mcontrol_util.h"




/**
 *  Handles the "set test ..." commands.
 */
int sbm_set_test_func( srArgBuffer& args, mcuCBHandle *mcu );

/**
 *  Handles the "print test ..." commands.
 */
int sbm_print_test_func( srArgBuffer& args, mcuCBHandle *mcu );




/**
 *  Handles the "test args ..." commands.
 *  Tests srArgBuffer by counting the arguments and printing them out.
 */
int test_args_func( srArgBuffer& args, mcuCBHandle *mcu );

/**
 *  Handles the "test bml ..." commands.
 *  Try "test bml help".
 */
int test_bml_func( srArgBuffer& args, mcuCBHandle *mcu );

/**
 *  Handles the "test fml ..." commands.
 *  Try "test fml help".
 */
int test_fml_func( srArgBuffer& args, mcuCBHandle *mcu );


/**
 *  This function is used to test the face, but can be used to test 
 *  any joints with position parameterizion, to use at the SBM command
 *  line put:
 *    test bone_pos <joint name> <x pos> <y pos> <z pos>
 */
int test_bone_pos_func( srArgBuffer& args, mcuCBHandle* mcu_p );


#endif // SBM_TEST_CMDS_HPP