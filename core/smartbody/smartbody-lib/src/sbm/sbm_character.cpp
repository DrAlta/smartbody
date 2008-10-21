/*
 *  sbm_character.cpp - part of SmartBody-lib
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
 *      Ed Fast, USC
 *      Ashok Basawapatna, USC (no longer)
 */

#include <stdio.h>

#include <iostream>
#include <string>

#include <SK/sk_skeleton.h>
#include <ME/me_ct_blend.hpp>
#include <ME/me_ct_time_shift_warp.hpp>
#include "mcontrol_util.h"
#include "me_utilities.hpp"
#include <ME/me_spline_1d.hpp>


const bool LOG_CONTROLLER_TREE_PRUNING = false;


using namespace std;

// Predeclare private functions defined below
static int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p );
static inline bool parse_float_or_error( float& var, const char* str, const string& var_name );




/////////////////////////////////////////////////////////////
//  Singleton Instance
mcuCBHandle* mcuCBHandle::_singleton = NULL;


/////////////////////////////////////////////////////////////
//  Method Definitions

MeCtSchedulerClass* CreateSchedulerCt( const char* character_name, const char* sched_type_name ) {
	MeCtSchedulerClass* sched_p = new MeCtSchedulerClass();
	sched_p->ref();
	sched_p->active_when_empty( true );
	string sched_name( character_name );
	sched_name += "'s ";
	sched_name += sched_type_name;
	sched_name += " schedule";
	sched_p->name( sched_name.c_str() );

	return sched_p;
}

//  Constructor
SbmCharacter::SbmCharacter( const char* character_name )
:	SbmPawn( character_name ),
	speech_impl( NULL ),
	posture_sched_p( CreateSchedulerCt( character_name, "posture" ) ),
	motion_sched_p( CreateSchedulerCt( character_name, "motion" ) ),
	gaze_sched_p( CreateSchedulerCt( character_name, "gaze" ) ),
	head_sched_p( CreateSchedulerCt( character_name, "head" ) ),
	face_ct( new MeCtFace() ),
	face_neutral( NULL )
{
	face_ct->ref();
	string face_ct_name( character_name );
	face_ct_name += "'s face_ct";
	face_ct->name( face_ct_name.c_str() );

	bonebusCharacter = NULL;

	eye_blink_closed = false;
	eye_blink_last_time = 0;
	eye_blink_repeat_time = 0;
}

//  Destructor
SbmCharacter::~SbmCharacter( void )	{
	posture_sched_p->unref();
	motion_sched_p->unref();
	gaze_sched_p->unref();
	head_sched_p->unref();
	face_ct->unref();

    if ( bonebusCharacter )
    {
       mcuCBHandle::singleton().bonebus.DeleteCharacter( bonebusCharacter );
       bonebusCharacter = NULL;
    }
}


int SbmCharacter::init( SkSkeleton* new_skeleton_p,
					    SkMotion* face_neutral,
                        const AUMotionMap* au_motion_map,
                        const VisemeMotionMap* viseme_motion_map,
                        const char* unreal_class )
{
	if( face_neutral ) {
		// Store pointers for access via init_skeleton()
		this->face_neutral      = face_neutral;
		face_neutral->ref();
		this->au_motion_map     = au_motion_map;
		this->viseme_motion_map = viseme_motion_map;
	}

	int result=SbmPawn::init( new_skeleton_p );
	if( result!=CMD_SUCCESS ) {
		return( result ); 
	}

	if( face_neutral ) {
		// Clear pointer data
		this->viseme_motion_map = NULL;
		this->au_motion_map     = NULL;
		face_neutral->unref();
		this->face_neutral      = NULL;
	}

	posture_sched_p->init();
	motion_sched_p->init();
	gaze_sched_p->init();
	head_sched_p->init();

	// Add Prioritized Schedule Controllers to the Controller Pipeline
	pipeline_p->add_controller( posture_sched_p );
	pipeline_p->add_controller( motion_sched_p );
	pipeline_p->add_controller( gaze_sched_p );
	pipeline_p->add_controller( head_sched_p );
	pipeline_p->name( std::string(name)+"'s pipeline" );

	// Face controller
	if( face_neutral ) {
		pipeline_p->add_controller( face_ct );
	}


	bonebusCharacter = mcuCBHandle::singleton().bonebus.CreateCharacter( name, unreal_class, mcuCBHandle::singleton().net_face_bones );

	return( CMD_SUCCESS ); 
}


void SbmCharacter::add_face_channel( const string& name, const int wo_index ) {
	SkJoint* joint_p = skeleton_p->add_joint( SkJoint::TypeEuler, wo_index );
	joint_p->name( SkJointName( name.c_str() ) );
	// Activate channel with lower limit != upper.
	joint_p->pos()->limits( SkJointPos::X, 0, 2 );  // Setting upper bound to 2 allows some exageration
}

int SbmCharacter::init_skeleton() {
	if( SbmPawn::init_skeleton() != CMD_SUCCESS ) {
		return CMD_FAILURE;
	}

	// Adding viseme and FAC control channels
	//
	// Because the channels at the pipeline level are based on
	// the chanels of a skeleton, we need to use joints to add
	// these channels.  We need to reimplement the pipeline to
	// use raw channels or channel arrays.
	const SkJoint* wo_joint_p = get_world_offset_joint();
	if( !wo_joint_p ) {
		cerr << "ERROR: SbmCharacter lacks world_offset joint after SbmPawn::init_skeleton." << endl;
		return CMD_FAILURE;
	}
	const int wo_index = wo_joint_p->index();  // World offest joint index
	

	if( face_neutral ) {
		face_ct->init( face_neutral );

		{	AUMotionMap::const_iterator i   = au_motion_map->begin();
			AUMotionMap::const_iterator end = au_motion_map->end();
			
			for(; i != end; ++i ) {
				const int   id = i->first;
				AUMotionPtr au( i->second );

				if( au->is_bilateral() ) {
					if( au->left ) {
						string name = "au_";
						name += id;
						name += "_left";

						add_face_channel( name, wo_index );
						// TODO: Add to au_channel_map
						//       & register with face_ct
					}
					if( au->right ) {
						string name = "au_";
						name += id;
						name += "_right";
						add_face_channel( name, wo_index );
						// TODO: Add to au_channel_map
						//       & register with face_ct
					}
				} else {
					if( au->left ) {
						string name = "au_";
						name += id;
						add_face_channel( name, wo_index );
						// TODO: Add to au_channel_map
						//       & register with face_ct
					}
				}
			}
			//face_ct->init( face_neutral );
		}
		{	VisemeMotionMap::const_iterator i   = viseme_map.begin();
			VisemeMotionMap::const_iterator end = viseme_map.end();
			for(; i != end; ++i ) {
				const string&    id     = i->first;
				const SkMotion* motion = i->second;

				if( motion ) {
					string name = "viseme_";
					name += id;
					add_face_channel( name, wo_index );
				}
			}
		}

		face_ct->finish_adding();
	}

	// Rebuild the active channels to include new joints
	skeleton_p->make_active_channels();

	return CMD_SUCCESS;
}

// Recursive portion of SbmCharacter::prune_controller_tree
void prune_schedule( MeCtScheduler2* sched,
					 double          time,
					 MeCtScheduler2* posture_sched_p,
					 //////  Higher priority controllers....
					 MeCtGaze**      &gaze_key_cts,
					 MeCtSimpleNod*  &nod_ct,
					 MeController*   &motion_ct,
					 MeCtPose*       &pose_ct
) {
	if( LOG_CONTROLLER_TREE_PRUNING ) cout << "DEBUG: sbm_character.cpp prune_schedule(..): Pruning schedule \""<<sched->name()<<"\" from time "<<time<<endl;
	MeCtScheduler2::track_iterator first = sched->begin();
	MeCtScheduler2::track_iterator it = sched->end();

	vector< MeCtScheduler2::track_iterator > tracks_to_remove;  // don't remove during iteration

	while( it != first ) {
		// Decrement track iterator (remember, we started at end)
		--it;

		// Start with the assumption the controller is in use
		bool in_use     = true;
		bool flat_blend_curve = true;  // No blend controller means the blend is always 1, thus flat

		MeController* anim_source = it->animation_ct();
		if( anim_source ) {
#if 0 // DYNAMIC_CASTS_ACTUALLY_WORK?
			// These don't seem to work, even with Runtime Type Inspection enabled
			MeCtBlend*         blend_ct = dynamic_cast<MeCtBlend*>( it->blending_ct() );
			MeCtTimeShiftWarp* timing_ct = dynamic_cast<MeCtTimeShiftWarp*>( it->timing_ct() );
#else // Trying using manual runtime typing
			MeCtUnary* unary_blend_ct = it->blending_ct();
			MeCtBlend* blend_ct = NULL;
			if( unary_blend_ct && unary_blend_ct->controller_type() == MeCtBlend::CONTROLLER_TYPE )
				blend_ct = (MeCtBlend*)unary_blend_ct;

			MeCtUnary*         unary_timing_ct = it->timing_ct();
			MeCtTimeShiftWarp* timing_ct = NULL;
			if( unary_timing_ct && unary_timing_ct->controller_type() == MeCtTimeShiftWarp::CONTROLLER_TYPE )
				timing_ct = (MeCtTimeShiftWarp*)unary_timing_ct;
#endif

			if( blend_ct ) {
				// Determine if the blend is still active,
				// or will ever be in the future

				MeSpline1D& spline = blend_ct->blend_curve();
				MeSpline1D::Knot* knot = spline.knot_last();
				MeSpline1D::domain x = knot->get_x();
				MeSpline1D::range  y = knot->get_y();
				if( LOG_CONTROLLER_TREE_PRUNING )
					cout << "\tblend_Ct \""<<blend_ct->name()<<"\": blend curve last knot: x = "<<x<<", y = "<< y <<endl;
				if( x < time ) {
					flat_blend_curve = true;
					if( y == 0 ) {
						in_use = false;
					}
				} else {
					if( y == knot->get_left_y() ) {
						MeSpline1D::Knot* prev_knot = knot;
						knot = prev_knot->get_prev();
						if( knot->get_x() < time ) {
							// span between covers now and all future activity.
							if( knot->get_right_control() != y ) {
								flat_blend_curve = false;
								if( y==0 )
									in_use = false;
							}
						} else {
							flat_blend_curve = false;  // assume more blend curve knots means more activity
						}
					} else {
						static const double TIME_EPSILON = MeCtScheduler2::MAX_TRACK_DURATION * 0.999;
						if( x < TIME_EPSILON )
							flat_blend_curve = false;  // activity at the blend curve knot
					}
				}
			}
			if( LOG_CONTROLLER_TREE_PRUNING && anim_source ) {
				cout << '\t' << anim_source->controller_type() << " \"" << anim_source->name() << "\": in_use = "<<in_use<<", flat_blend_curve = "<<flat_blend_curve<<endl;
			}

			if( in_use && flat_blend_curve ) {  // Ignore tracks with future blend activity or are already not in use
				// Determine if the animation will be occluded by
				// (previously visited) higher priority controllers
				const char* anim_ct_type = anim_source->controller_type();
				if( anim_ct_type == MeCtScheduler2::type_name ) {
					double time_offset = time;
					if( timing_ct ) {
						time_offset = timing_ct->time_func().eval( time );
					}

					MeCtScheduler2* sched_ct = (MeCtScheduler2*)anim_source;

					if( sched_ct==posture_sched_p ) {
						//ostringstream oss;
						//oss << sched_ct->print_state( "1", oss );

						//  Don't let higher priority controller occlude ALL pose controllers
						//  by pretending there wasn't a higher priority controller
						MeCtGaze**     gaze_key2_cts = new MeCtGaze*[ MeCtGaze::NUM_GAZE_KEYS ];
						for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key )
							gaze_key2_cts[key] = NULL;

						MeCtSimpleNod* nod2_ct = NULL;
						MeController*  motion2_ct = NULL;
						MeCtPose*      pose2_ct = NULL;
						prune_schedule( sched_ct, time_offset, posture_sched_p, gaze_key2_cts, nod2_ct, motion2_ct, pose2_ct );

						delete[] gaze_key2_cts;
						//if( sched_ct->count_children()==0 ) {
						//	cerr<< "ERROR!!  Invalid posture track: "<<oss.str()<<endl;
						//}

						in_use = true;
					} else {
						prune_schedule( sched_ct, time_offset, posture_sched_p, gaze_key_cts, nod_ct, motion_ct, pose_ct );
						in_use = sched_ct->count_children()>0;
					}
				} else if( anim_ct_type == MeCtSimpleNod::_type_name ) {
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << "DEBUG: testing MeCtMotion for pruning... ";
					if(    nod_ct
						|| (    (gaze_key_cts[MeCtGaze::GAZE_KEY_HEAD]!=NULL)
						     && (gaze_key_cts[MeCtGaze::GAZE_KEY_NECK]!=NULL) ) )
					{
						in_use = false;
					} else {
						nod_ct = (MeCtSimpleNod*)anim_source;
					}
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << ( in_use? "Not Pruned." : "Pruned!" ) << endl;


				} else if( anim_ct_type == MeCtGaze::type_name ) {
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << "DEBUG: testing MeCtGaze for pruning... ";
					if( motion_ct || pose_ct ) {
						in_use = false;
					} else {
						MeCtGaze* gaze_ct = (MeCtGaze*)anim_source;

						bool is_occluded = true;
						for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key ) {
							if( gaze_ct->get_blend( key ) > 0 ) {  // gaze_ct has output for this key
								if( gaze_key_cts[ key ]==NULL ) {
									is_occluded = false;
									if( gaze_ct->is_full_blend( key ) )
										// Occludes lower priority controllers
										gaze_key_cts[ key ] = gaze_ct;
								}
							}
						}

						// If still ocluded (after testing each key) then it is not in use
						in_use = !is_occluded;
					}
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << ( in_use? "Not Pruned." : "Pruned!" ) << endl;

				} else if( anim_ct_type == MeCtMotion::type_name || anim_ct_type == MeCtQuickDraw::type_name ) {
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << "DEBUG: testing "<<anim_ct_type<<" for pruning... ";
					if( motion_ct || pose_ct ) {
						in_use = false;
					} else {
						motion_ct = anim_source;
					}
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << ( in_use? "Not Pruned." : "Pruned!" ) << endl;

				} else if( anim_ct_type == MeCtPose::type_name ) {
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << "DEBUG: testing MeCtPose for pruning... ";
					if( pose_ct ) {
						in_use = false;
					} else {
						pose_ct = (MeCtPose*)anim_source;
					}
					if( LOG_CONTROLLER_TREE_PRUNING ) cout << ( in_use? "Not Pruned." : "Pruned!" ) << endl;

				} else {
					//  TODO: Throttle warnings....
					cerr << "WARNING: Cannot prune unknown controller type \"" << anim_source->controller_type() << "\"" << endl;
				}
			}
		} else {
			// No animation source
			in_use = false;
		}

		if( !in_use ) {
			// insert at front, because we are iterating end->begin
			// and we prefer the final list order matches order within schedule
			tracks_to_remove.insert( tracks_to_remove.begin(), it );
		}
	}

	if( !tracks_to_remove.empty() ) {
		sched->remove_tracks( tracks_to_remove );
	}
}

/**
 *  Prunes the controller tree by making wild assumptions about
 *  what types of controllers will overwrite the results of
 *  of other types of controllers. Fails to recognize partial
 *  body motions and partial spine gazes.
 *
 *  What a glorious hack before I leave for China...  - Anm
 */
int SbmCharacter::prune_controller_tree() {
	double time = mcuCBHandle::singleton().time;  // current time

	// Pointers to the most active controllers of each type.
	MeCtGaze**     gaze_key_cts = new MeCtGaze*[ MeCtGaze::NUM_GAZE_KEYS ];
	for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key )
		gaze_key_cts[key] = NULL;
	MeCtSimpleNod* nod_ct    = NULL;
	MeController*  motion_ct = NULL;  // also covers quickdraw
	MeCtPose*      pose_ct   = NULL;

	

	// Traverse the controller tree from highest priority down, most recent to earliest
	prune_schedule( head_sched_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct );
	prune_schedule( gaze_sched_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct );
	prune_schedule( motion_sched_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct );

	// For the posture track, ignore prior controllers, as they should never be used to mark a posture as unused
	for( int key=0; key<MeCtGaze::NUM_GAZE_KEYS; ++key )
		gaze_key_cts[key] = NULL;
	nod_ct    = NULL;
	motion_ct = NULL;  // also covers quickdraw
	pose_ct   = NULL;
	prune_schedule( posture_sched_p, time, posture_sched_p, gaze_key_cts, nod_ct,  motion_ct, pose_ct );

	delete[] gaze_key_cts;

	return CMD_SUCCESS;
}


void SbmCharacter::remove_from_scene() {
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	
	string vrProcEnd_msg = "vrProcEnd sbm ";
	vrProcEnd_msg += name;
	mcu.vhmsg_send( vrProcEnd_msg.c_str() );

	mcu.character_map.remove( name );

	SbmPawn::remove_from_scene();
}

int SbmCharacter::set_speech_impl( SmartBody::SpeechInterface *speech_impl ) {
	this->speech_impl = speech_impl;

	return CMD_SUCCESS;
}

//returns speech implementation if set or NULL if not
SmartBody::SpeechInterface* SbmCharacter::get_speech_impl() const {
	return speech_impl;
}

int SbmCharacter::set_voice_code( std::string& voice_code ) //allows you to set the voice-- made different from the init because of non Rhetoric might not have voice codes
{
	//TODO: LOOK AND SEE IF THIS VOICE EXISTS AND IF IT DOESN'T PRINT ERROR MESSAGE AND RETURN FAILURE
	this->voice_code = voice_code; //sets voice 
	return (CMD_SUCCESS);
}

const std::string& SbmCharacter::get_voice_code() const
{
	return voice_code; //if voice isn't NULL-- no error message; just returns the string
}


void SbmCharacter::set_viseme( char* viseme,
							   float weight,
							   float rampin_duration )
{
    //fprintf( stdout, "Recieved: SbmCharacter(\"%s\")::set_viseme( \"%s\", %f, %f )\n", name, viseme, weight, rampin_duration );

	// SendWinsockSetViseme( viseme, weight );  // Old code...  

	// I think this is the corrected code (untested as of 20060719)
	//NetworkSetViseme( net_handle, viseme, weight, rampin_duration );
	if ( bonebusCharacter )
	{
		bonebusCharacter->SetViseme( viseme, weight, rampin_duration );
	}
}


void SbmCharacter::eye_blink_update( const double frame_time )
{
   if ( bonebusCharacter == NULL )
   {
      return;
   }

   // automatic blinking routine
   static const double blink_down_time = 0.04;       // how long the eyes should stay closed. ~1 frame
   static const double min_blink_repeat_time = 4.0;  // how long to wait until the next blink
   static const double max_blink_repeat_time = 8.0;  // will pick a random number between these min/max
   
   if ( !eye_blink_closed )
   {
      if ( frame_time - eye_blink_last_time > eye_blink_repeat_time )
      {
         // close the eyes
         bonebusCharacter->SetViseme( "blink", 0.9f, 0.001f );

         eye_blink_last_time = frame_time;
         eye_blink_closed = true;
      }
   }
   else
   {
      if ( frame_time - eye_blink_last_time > blink_down_time )
      {
         // open the eyes
         bonebusCharacter->SetViseme( "blink", 0, 0.001f );

         eye_blink_last_time = frame_time;
         eye_blink_closed = false;

         // compute when to close them again
         double fraction = (double)rand() / (double)RAND_MAX;
         eye_blink_repeat_time = ( fraction * ( max_blink_repeat_time - min_blink_repeat_time ) ) + min_blink_repeat_time;
      }
   }
}

///////////////////////////////////////////////////////////////////////////

void SbmCharacter::inspect_skeleton( SkJoint* joint_p, int depth )	{
	int i, j, n;
	
	if( joint_p )	{
		const char *name = joint_p->name();
		for( j=0; j<depth; j++ ) { printf( " " ); }
		printf( "%s\n", name );
		n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton( joint_p->child( i ), depth + 1 );
		}
	}
}

void SbmCharacter::inspect_skeleton_local_transform( SkJoint* joint_p, int depth )	{
	
	if( joint_p )	{
		const char *name = joint_p->name();
		matrix_t M;
		int i, j;

		SrMat sr_M = joint_p->lmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		euler_t rot = M.euler( GWIZ_M_TR );

		for( j=0; j<depth; j++ ) { printf( " " ); }
		printf( "%s : pos{ %.3f %.3f %.3f } : phr{ %.2f %.2f %.2f }\n", 
			name,
			pos.x(), pos.y(), pos.z(),
			rot.p(), rot.h(), rot.r()
		);

		int n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton_local_transform( joint_p->child( i ), depth + 1 );
		}
	}
}

void SbmCharacter::inspect_skeleton_world_transform( SkJoint* joint_p, int depth )	{
	
	if( joint_p )	{
		const char *name = joint_p->name();
		matrix_t M;
		int i, j;

		joint_p->update_gmat_up();
		SrMat sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		euler_t rot = M.euler( GWIZ_M_TR );

		for( j=0; j<depth; j++ ) { printf( " " ); }
		printf( "%s : pos{ %.3f %.3f %.3f } : phr{ %.2f %.2f %.2f }\n", 
			name,
			pos.x(), pos.y(), pos.z(),
			rot.p(), rot.h(), rot.r()
		);
		
		int n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton_world_transform( joint_p->child( i ), depth + 1 );
		}
	}
}

// HACK to initiate reholster on all QuickDraw controllers
int SbmCharacter::reholster_quickdraw( mcuCBHandle *mcu_p ) {
	const double now = mcu_p->time;
	double max_blend_dur = -1;

	MeCtScheduler2::track_iterator it = motion_sched_p->begin();
	MeCtScheduler2::track_iterator end = motion_sched_p->end();

	while( it != end ) {
		MeController* anim_ct = it->animation_ct();
		if( anim_ct ) {
			string anim_ct_type( anim_ct->controller_type() );
			if( anim_ct_type==MeCtQuickDraw::type_name ) {
				MeCtQuickDraw* qdraw_ct = (MeCtQuickDraw*)anim_ct;

				// Initiate reholster
				qdraw_ct->set_reholster();

				// Attempt to schedule blend out
				MeCtUnary* blending_ct = it->blending_ct();
				if(    blending_ct
					&& strcmp(blending_ct->controller_type(), MeCtBlend::CONTROLLER_TYPE ) )
				{
					// TODO: account for time scaling of motion_duration
					double blend_out_start = now + qdraw_ct->get_motion_duration();
					float  blend_out_dur   = qdraw_ct->outdt();
					double blend_out_end   = blend_out_start + blend_out_dur;
					float  blend_spline_tanget = -1/blend_out_dur;

					MeCtBlend* blend_ct = (MeCtBlend*)blending_ct;
					MeSpline1D& spline = blend_ct->blend_curve();
					// TODO: Don't assume we're starting at 1, may already be less than and already blending out.
					spline.make_cusp( blend_out_start,1,  0,1, blend_spline_tanget,1 );
					MeSpline1D::Knot* knot = spline.make_cusp( blend_out_end,  0,  blend_spline_tanget,1, 0,1 );

					// TODO: delete following knots

					if( blend_out_dur > max_blend_dur )
						max_blend_dur = blend_out_end;
				}
			}
		}

		++it;
	}

////  Won't compile, and I'm tired:
////  Error	1	error C2296: '<<' : illegal, left operand has type 'std::ostringstream (__cdecl *)(void)'
//	if( max_blend_dur >= 0 ) {
//		// schedule prune
//		max_blend_dur += 1;  // small buffer period
//
//		ostringstream out();
//		out << "char " << name << " prune";
//		mcu_p->execute_later( out.str().c_str(), max_blend_dur );
//	}

	return CMD_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////

int SbmCharacter::character_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string char_name = args.read_token();
	if( char_name.length()==0 ) {
		cerr << "ERROR: Expected character name." << endl;
		return CMD_FAILURE;
	}

	string char_cmd  = args.read_token();
	if( char_cmd.length()==0 ) {
		cerr << "ERROR: Expected character command." << endl;
		return CMD_FAILURE;
	}

	bool all_characters = false;
	SbmCharacter* character = NULL;
	if( char_name=="*" ) {
		all_characters = true;
	} else {
		character = mcu_p->character_map.lookup( char_name.c_str() );
	}

	if( char_cmd=="init" ) {
		// Original style creation:
		char* skel_file = args.read_token();
		char* unreal_class = args.read_token();
		return(	
			mcu_character_init( char_name.c_str(), skel_file, unreal_class, mcu_p )
		);
	} 
	else 
	if( char_cmd=="ctrl" ) {
		return mcu_character_ctrl_cmd( char_name.c_str(), args, mcu_p );
	} 
	else 
	if( char_cmd=="inspect" ) {
		if( character ) {
			if( character->skeleton_p ) {
				SkJoint* joint_p = character->skeleton_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME );
				if( joint_p )	{
					character->inspect_skeleton( joint_p );
//					inspect_skeleton_local_transform( joint_p );
//					inspect_skeleton_world_transform( joint_p );
				}
			}
		}
		return CMD_SUCCESS;
	}
	else
	if( char_cmd=="prune" ) {
		int result = CMD_SUCCESS;
		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() ) {
				if( character->prune_controller_tree() != CMD_SUCCESS ) {
					cerr << "ERROR: Failed to prune controller tree of character \""<<character->name<<"\"."<<endl;
					result = CMD_FAILURE;
				}
			}
		} else if( character ) {
			int result = character->prune_controller_tree();
			if( result != CMD_SUCCESS ) {
				cerr << "ERROR: Failed to prune controller tree of character \""<<char_name<<"\"."<<endl;
			}
		} else {
			cerr<<"ERROR: Unknown character \""<<char_name<<"\" for prune command."<<endl;
			result = CMD_FAILURE;
		}
		return result;
	}
	else 
	if( char_cmd=="viseme" ) {
        char * viseme = args.read_token();
        float  weight = args.read_float();
		float  rampin_duration = args.read_float();
		 
		if( all_characters ) {
			mcu_p->character_map.reset();
			while( character = mcu_p->character_map.next() ) {
				character->set_viseme( viseme, weight, rampin_duration );
			}
			return CMD_SUCCESS;
		} else {
			if ( !character ) {
				cerr << "ERROR: SbmCharacter::character_cmd_func(..): Unknown character \"" << char_name << "\"." << endl;
				return CMD_FAILURE;  // this should really be an ignore/out-of-domain result
			} else {
				character->set_viseme( viseme, weight, rampin_duration );
				return CMD_SUCCESS;
			}
		}
	} else if( char_cmd=="bone" ) {
		return mcu_character_bone_cmd( char_name.c_str(), args, mcu_p );
	} else if( char_cmd=="bonep" ) {
		return mcu_character_bone_position_cmd( char_name.c_str(), args, mcu_p );
	} else if( char_cmd=="remove" ) {
		return SbmCharacter::remove_from_scene( char_name.c_str() );
	} else if( char_cmd=="reholster" ) {
		return character->reholster_quickdraw( mcu_p );
	} else {
		return CMD_NOT_FOUND;
	}
}

int SbmCharacter::remove_from_scene( const char* char_name ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if( strcmp( char_name, "*" )==0 ) {
		SbmCharacter * char_p;
		mcu.character_map.reset();
		while( char_p = mcu.character_map.pull() ) {
			char_p->remove_from_scene();
			delete char_p;
		}
		return CMD_SUCCESS;
	} else {
		SbmCharacter* char_p = mcu.character_map.lookup( char_name );

		if ( char_p ) {
			char_p->remove_from_scene();
			delete char_p;

			return CMD_SUCCESS;
		} else {
			printf( "ERROR: Unknown character \"%s\".\n", char_name );
			return CMD_FAILURE;
		}
	}
}

int SbmCharacter::set_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		cerr << "ERROR: SbmCharacter::set_cmd_func(..): Missing character id." << endl;
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->character_map.lookup( character_id.c_str() );
	if( character==NULL ) {
		cerr << "ERROR: SbmCharacter::set_cmd_func(..): Unknown character \""<<character_id<<"\" to set." << endl;
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		cerr << "ERROR: SbmCharacter::set_cmd_func(..): Missing attribute to set." << endl;
		return CMD_FAILURE;
	}

	//  voice_code and voice-code are backward compatible patches
	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		return set_voice_cmd_func( character, args, mcu_p );
	} else {
		return SbmPawn::set_attribute( character, attribute, args, mcu_p );
	}
}

int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args, mcuCBHandle *mcu_p ) {
	//  Command: set character voice <speech_impl> <character id> voice <implementation-id> <voice code>
	//  Where <implementation-id> is "remote" or "audiofile"
	//  Sets character's voice code
	const char* impl_id = args.read_token();

	if( strlen( impl_id )==0 ) {
		character->set_speech_impl( NULL );
		character->set_voice_code( string("") );
		
		// Give feedback if unsetting
		cout << "Unset " << character->name << "'s voice." << endl;
	} else if( _strcmpi( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			cerr << "ERROR: Expected remote voice id." << endl;
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_rvoice() );
		character->set_voice_code( string( voice_id ) );
	} else if( _strcmpi( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			cerr << "ERROR: Expected audiofile voice path." << endl;
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else {
		cerr << "ERROR: Unknown speech implementation \"" << impl_id << "\"." << endl;
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

int SbmCharacter::print_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p ) {
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		cerr << "ERROR: SbmCharacter::print_cmd_func(..): Missing character id." << endl;
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->character_map.lookup( character_id.c_str() );
	if( character==NULL ) {
		cerr << "ERROR: SbmCharacter::print_cmd_func(..): Unknown character \""<<character<<"\"." << endl;
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		cerr << "ERROR: SbmCharacter::print_cmd_func(..): Missing attribute to print." << endl;
		return CMD_FAILURE;
	}

	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		//  Command: print character <character id> voice_code
		//  Print out the character's voice_id
		cout << "character " << character_id << "'s voice_code: " << character->get_voice_code() << endl;
		return CMD_SUCCESS;
	} else if( attribute=="schedule" ) {
		//  Command: print character <character id> schedule
		//  Print out the current state of the character's schedule
		cout << "Character " << character_id << "'s schedule:" << endl;
		cout << "POSTURE Schedule:" << endl;
		character->posture_sched_p->print_state( 0 );
		cout << "MOTION Schedule:" << endl;
		character->motion_sched_p->print_state( 0 );
		cout << "GAZE Schedule:" << endl;
		character->gaze_sched_p->print_state( 0 );
		cout << "HEAD Schedule:" << endl;
		character->head_sched_p->print_state( 0 );
		// Print Face?

		return CMD_SUCCESS;
	} else {
		return SbmPawn::print_attribute( character, attribute, args, mcu_p );
	}
}


///////////////////////////////////////////////////////////////////////////
//  Private sbm_character functions

// Because I don't like c style error checking, I'm avoiding srArgBuffer::read_float
bool parse_float_or_error( float& var, const char* str, const string& var_name ) {
	if( istringstream( str ) >> var )
		return true; // no error
	// else
	cerr << "ERROR: Invalid value for " << var_name << ": " << str << endl;
	return false;
}
