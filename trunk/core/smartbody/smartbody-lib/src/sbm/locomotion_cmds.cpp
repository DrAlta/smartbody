/*
 *  locomotion_cmds.cpp - part of SmartBody-lib
 *  Copyright (C) 2009  University of Southern California
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
 *      Jingqiao Fu, USC
 */

#include <iostream>
#include <string>

#include "gwiz_math.h"
#include "locomotion_cmds.hpp"
#include "me_ct_locomotion.hpp"

#include <sk/sk_channel_array.h>
#include <me/me_ct_raw_writer.hpp>
#include <sbm/me_ct_navigation_circle.hpp>

using namespace std;

int locomotion_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	// should this function be removed?

	SkMotion *walking1_p= mcu_p->motion_map.lookup("Step_WalkForward");
	SkMotion *walking2_p= mcu_p->motion_map.lookup("Step_StrafeRight");
	SkMotion *standing_p= mcu_p->motion_map.lookup("HandsAtSide_Motex_Softened");

	if(walking1_p && walking2_p && standing_p)
	{
		MeCtLocomotion ct_locomotion;
		MeCtLocomotionAnalysis analysis;
		analysis.set_ct(&ct_locomotion);
		analysis.init(standing_p, mcu_p->me_paths);
		analysis.add_locomotion(walking1_p);
		analysis.add_locomotion(walking2_p);
		return CMD_SUCCESS;
	}
	else
	{
		return CMD_FAILURE;
	}
}


int test_locomotion_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p  )	{
	string arg = args.read_token();
	if( arg.empty() || arg=="help" ) {
		cout << "Syntax:" << endl
		     << "\ttest locomotion [char <agent-id>]"<<endl
			 << "\t                [dx <dx>] [dy <dy>] [dz <dz>]" <<endl
			 << "\t                [deg_per_sec <angle> | rad_per_sec <angle>]" <<endl
			 << "\t                [global_deg_per_sec <angle> | global_rad_per_sec <angle>]" <<endl
			 << "\t                [local_deg_per_sec <angle> | local_rad_per_sec <angle>]" <<endl
			 << "\t                [id <int>]" <<endl
			 << "\t                [del <int>]" <<endl
			 << "\t                [enable | disable]" <<endl
		     << "\ttest locomotion [char <agent-id>] zero"<<endl
			 << "Translational velocity is in global coordinates" <<endl;
		return CMD_SUCCESS;
	}
	
	if(arg=="initialize")
	{
		mcu_p->use_locomotion = true;
		cout << "Locomotion engine has been initialized." << endl;

		return CMD_SUCCESS;
	}

	SbmCharacter* actor = NULL;
	if( arg=="character" || arg=="char" ) {
		string name = args.read_token();
		actor = mcu_p->character_map.lookup( name );
		if( actor == NULL ) {
			cerr << "ERROR: Could not find character \""<<name<<"\"." <<endl;
			return CMD_FAILURE;
		}

		arg = args.read_token();
	} else {
		if( mcu_p->test_character_default.empty() ) {
			cerr << "ERROR: No character specified, and no default set." <<endl;
			return CMD_FAILURE;
		}
		actor = mcu_p->character_map.lookup( mcu_p->test_character_default );
		if( actor == NULL ) {
			cerr << "ERROR: Could not find default character \""<<mcu_p->test_character_default<<"\"." <<endl;
			return CMD_FAILURE;
		}
	}

	if(!actor->is_locomotion_controller_initialized()) 
	{
		cerr << "ERROR: Locomotion controller not initialized." << endl;
		return CMD_FAILURE;
	}

	if(arg=="enable")
	{
		actor->get_locomotion_ct()->enabled = true;
		cout << "Locomotion engine has been enabled." << endl;

		return CMD_SUCCESS;
	}
	else if(arg=="disable")
	{
		actor->get_locomotion_ct()->enabled = false;
		cout << "Locomotion engine has been disabled." << endl;

		return CMD_SUCCESS;
	}

	if(!actor->is_locomotion_controller_enabled()) 
	{
		cerr << "ERROR: Locomotion controller not enabled." << endl;
		return CMD_FAILURE;
	}

	MeCtNavigationCircle* nav_circle = new MeCtNavigationCircle();
	nav_circle->ref();
	nav_circle->init( 0, 0, 0, 0, 0, 0, 0, 0, 0);
	actor->posture_sched_p->create_track( NULL, NULL, nav_circle);
	nav_circle->unref();
	
	if( arg=="stop" ) {
		SkChannelArray channels;
		channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::XPos );
		channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::YPos );
		channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::ZPos );
		channels.add( SkJointName( SbmCharacter::LOCOMOTION_GLOBAL_ROTATION ), SkChannel::YPos );
		channels.add( SkJointName( SbmCharacter::LOCOMOTION_LOCAL_ROTATION ), SkChannel::YPos );
		channels.add( SkJointName( SbmCharacter::LOCOMOTION_ID ), SkChannel::YPos );

		float data[] = { 0, 0, 0, 0, 0, 0 };

		float dx = 0.0f, dy = 0.0f, dz = 0.0f, g_angular = 0.0f, l_angular = 0.0f;
		int id = -1;

		nav_circle->init( dx, dy, dz, g_angular, l_angular, id, 0, 0, 0 );

		return CMD_SUCCESS;
	}

	else if( arg == "ik" )
	{
		int enable = args.read_int();
		if(enable == 1) 
		{
			actor->locomotion_ik_enable(true);
		}
		else if(enable == 0)
		{
			actor->locomotion_ik_enable(false);
		}

		return CMD_SUCCESS;
	}

	else if( arg=="reset" ) {

		actor->locomotion_reset();//temp command process
		
		return CMD_SUCCESS;
	} 	
	else if( arg=="turnspd" ) {

		actor->locomotion_set_turning_speed(args.read_float());//temp command process
		
		return CMD_SUCCESS;
	} 

	else if( arg=="turnmode" ) {

		actor->locomotion_set_turning_mode(args.read_int());//temp command process
		
		return CMD_SUCCESS;
	} 
	
	else if (arg == "tx" || arg == "tz" || arg == "spd")
	{
		SkChannelArray channels;
		float tx = 0.0f, ty = 0.0f, tz = 0.0f, spd = 0.0f;
		int flagx = 0, flagz = 0;
		int tx_index=-1, ty_index=-1, tz_index=-1, spd_index = -1;
		actor->get_locomotion_ct()->get_navigator()->clear_destination_list();
		while( !arg.empty() ) 
		{
			if(arg == "tx" || arg == "tz")
			{
				while( !arg.empty() ) 
				{
					if( arg == "tx" ) 
					{
						if( tx_index == -1 ) 
						{
							tx_index = channels.size();
						}
						tx = args.read_float();
						flagx = 1;
					}

					else if( arg == "tz" ) 
					{
						if( tz_index == -1 ) 
						{
							tz_index = channels.size();
						}
						tz = args.read_float();
						flagz = 1;
					} 

					else break;

					arg = args.read_token();

					if(flagx == 1 && flagz == 1) break;

				}
				if(flagx == 1 && flagz == 1)
				{
					SrVec dest(tx, 0.0f, tz);
					actor->get_locomotion_ct()->get_navigator()->add_destination(&dest);
					actor->get_locomotion_ct()->get_navigator()->has_destination = true;
					flagx = 0;
					flagz = 0;
				}
			}

			else if( arg == "spd" ) 
			{
				if( spd_index == -1 ) {
					spd_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::ZPos );
				}
				spd = args.read_float();
				actor->get_locomotion_ct()->get_navigator()->add_speed(spd);
				arg = args.read_token();
			} 
			else {
				cerr << "ERROR: Unexpected token \""<<arg << "\"." <<endl;
				return CMD_FAILURE;
			}
			
		}

		nav_circle->init( 0.0f, 0.0f, 0.0f, 0, 0, -1, 0, 0, 0 );
		return CMD_SUCCESS;
	}
	
	else if( arg=="dx" || arg=="dy" || arg == "dz" || arg=="deg_per_sec" || arg=="dps" || arg=="rad_per_sec" || arg=="rps" || arg=="grps" || arg=="lrps" ) {
		SkChannelArray channels;
		actor->get_locomotion_ct()->get_navigator()->has_destination = false;
		int dx_index=-1, dy_index=-1, dz_index=-1, g_angular_index=-1, l_angular_index=-1, id_index = -1;
		//float data[6] = {0,0,0,0,0,0};
		float dx = 0.0f, dy = 0.0f, dz = 0.0f, g_angular = 0.0f, l_angular = 0.0f;
		int id = -1;

		while( !arg.empty() ) {
			if( arg == "dx" ) {
				if( dx_index == -1 ) {
					dx_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::XPos );
				}
				dx = args.read_float();
				//data[ dx_index ] = dx;
			} else if( arg == "dy" ) {
				if( dy_index == -1 ) {
					dy_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::YPos );
				}
				dy = args.read_float();
				//data[ dy_index ] = dy;
			} else if( arg == "dz" ) {
				if( dz_index == -1 ) {
					dz_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::ZPos );
				}
				dz = args.read_float();
				//data[ dz_index ] = dz;
			} else if( arg == "deg_per_sec" || arg=="dps" ) {
				if( g_angular_index == -1 ) {
					g_angular_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_GLOBAL_ROTATION ), SkChannel::YPos );
				}
				g_angular = (float)RAD(args.read_float());
				//data[ g_angular_index ] = g_angular;
				if( l_angular_index == -1 ) {
					l_angular_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_LOCAL_ROTATION ), SkChannel::YPos );
				}
				l_angular = g_angular;
				//data[ l_angular_index ] = l_angular;
			} else if( arg == "rad_per_sec" || arg=="rps" ) {
				if( g_angular_index == -1 ) {
					g_angular_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_GLOBAL_ROTATION ), SkChannel::YPos );
				}
				g_angular = args.read_float();
				//data[ g_angular_index ] = g_angular;
				if( l_angular_index == -1 ) {
					l_angular_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_LOCAL_ROTATION ), SkChannel::YPos );
				}
				l_angular = g_angular;
				//data[ l_angular_index ] = l_angular;
			} else if(arg=="grps" ) {
				if( g_angular_index == -1 ) {
					g_angular_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_GLOBAL_ROTATION ), SkChannel::YPos );
				}
				g_angular = args.read_float();
				//data[ g_angular_index ] = g_angular;
			} else if(arg=="lrps" ) {
				if( l_angular_index == -1 ) {
					l_angular_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_LOCAL_ROTATION ), SkChannel::YPos );
				}
				l_angular = args.read_float();
				//data[ l_angular_index ] = l_angular;
			} else if(arg=="id" ) {
				if( id_index == -1 ) {
					id_index = channels.size();
					//channels.add( SkJointName( SbmCharacter::LOCOMOTION_ID ), SkChannel::YPos );
				}
				id = args.read_int();
				//data[ id_index ] = (float)id;
			} else {
				cerr << "ERROR: Unexpected token \""<<arg << "\"." <<endl;
				return CMD_FAILURE;
			}
			if(id_index == -1)
			{
				id_index = channels.size();
				//channels.add( SkJointName( SbmCharacter::LOCOMOTION_ID ), SkChannel::YPos );
				//data[ id_index ] = -1.0f;
				id = -1;
			}
			arg = args.read_token();
		}

		nav_circle->init( dx, dy, dz, g_angular, l_angular, id, 0, 0, 0 );

		return CMD_SUCCESS;
	}


	else if(arg == "del")
	{
		int id = args.read_int();
		nav_circle->init( 0, 0, 0, 0, 0, id, 0, 0, 0 );
		return CMD_SUCCESS;
	}

	else if(arg=="cx" || arg=="cy" || arg == "cz")
	{
		return CMD_SUCCESS;
	}

	
	/*else if( arg=="circle" ) 
	{
		arg = args.read_token();
		if( arg.empty() || arg=="help" ) {
			cout << "Syntax: test locomotion circle <velocity> <radius>" << endl;
			return CMD_SUCCESS;
		}
		float forward_velocity, radius;

		if( !( istringstream( arg ) >> forward_velocity ) ) {
			cerr << "ERROR: Invalid velocity \"" << arg << "\"." << endl;
			return CMD_FAILURE;
		}
		arg = args.read_token();
		if( !( istringstream( arg ) >> radius ) ) {
			cerr << "ERROR: Invalid radius \"" << arg << "\"." << endl;
			return CMD_FAILURE;
		}

		//MeCtNavigationCircle* nav_circle = new MeCtNavigationCircle();
		//nav_circle->ref();

		//nav_circle->initByRadius( forward_velocity, radius );
		//actor->posture_sched_p->create_track( NULL, NULL, nav_circle );

		//actor->init_locomotion_analyzer("common.sk", mcu_p);

		//nav_circle->unref();

		return CMD_SUCCESS;
	} */


	else {
		return CMD_NOT_FOUND;
	}
}
