/*
 *  me_ct_scheduler2.cpp - part of SmartBody-lib's Motion Engine
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
 *      Ed Fast, USC
 */


#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <SK/sk_skeleton.h>

#include <ME/me_ct_scheduler2.h>
#include <ME/me_ct_blend.hpp>
#include <ME/me_ct_time_shift_warp.hpp>


using namespace std;


#define DEBUG_INSPECT_CHANNELS       (0)  // Enable the runtime debugger inspection of channel name/types
#define DEBUG_LOGGED_CHANNEL_MAPPING (0)


const float MeCtScheduler2::MAX_TRACK_DURATION = 4194303.0; // Calculated to give 1 second precision with floats



// Copied from http://www.boost.org/doc/libs/1_38_0/libs/smart_ptr/sp_techniques.html#static
// for http://www.boost.org/doc/libs/1_38_0/libs/smart_ptr/sp_techniques.html#weak_without_shared
// used by MeCtScheduler2::_self
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};




//===== MeCtScheduler2::Context ==============================

const char* MeCtScheduler2::Context::CONTEXT_TYPE = "MeCtScheduler2::Context";

MeCtScheduler2::Context::Context( MeCtScheduler2* schedule )
:	MeCtContainer::Context( schedule ),
	_local_frame( this )   // Ignore Warning: No writes, just passing a reference
{}

void MeCtScheduler2::Context::child_channels_updated( MeController* child ) {
	if( _container == NULL )
		return; // MeCtScheduler2 was deleted.

	MeCtScheduler2* schedule = static_cast<MeCtScheduler2*>(_container);

	// Does Schedule have a context yet?
	MeControllerContext* parent_context = schedule->_context;
	//if( parent_context==NULL )
	//	return;  // Ignore...

	// Verify it is a child...
	if( schedule->_child_to_track.find(child) ==schedule->_child_to_track.end() )
		return; // Not a known child.  Ignore...

	SkChannelArray& child_channels = child->controller_channels();
	SkChannelArray& sched_channels = schedule->_channels;
	int floats_before = sched_channels.floats();
	sched_channels.merge( child_channels );

	if( sched_channels.floats() == floats_before ) {
		child->remap();
	} else {
		sched_channels.rebuild_hash_table();
		if( parent_context )
			parent_context->child_channels_updated( schedule );
	}
}

void MeCtScheduler2::Context::remove_controller( MeController* child ) {
	MeCtContainer::Context::remove_controller( child );
}

//// Wrapper method to put within permission scope of MeCtScheduler2
//void MeCtScheduler2::Context::add_controller( MeController* ct ) {
//	MeControllerContext::add_controller( ct );
//	childChannelsUpdated( ct );
//}


//======= MeCtScheduler2::Track ==============================

// Constructor
MeCtScheduler2::Track::Track( MeCtUnary* blending_ct,
							  MeCtUnary* timing_ct,
							  MeController* animation_ct )
:	_blending_ct( blending_ct ),
	_timing_ct( timing_ct ),
	_animation_ct( animation_ct ),
	_root( animation_ct )
{
	// _animation_ct should never be NULL
	_animation_ct->ref();
	if( _timing_ct ) {
		_timing_ct->ref();
		_timing_ct->init( _root );
		_root = _timing_ct;
	}
	if( _blending_ct ) {
		_blending_ct->ref();
		_blending_ct->init( _root );
		_root = _blending_ct;
	}
}


//// No Copy Constructor (Copy TrackPtrs instead)
//MeCtScheduler2::Track::Track( const MeCtScheduler2::Track& other )
//:   _schedule_weak( other._schedule ),
//	_blending_ct(  other._blending_ct ),
//	_timing_ct(    other._timing_ct ),
//	_animation_ct( other._animation_ct ),
//	_root(         other._root )
//#if TRACK_UNREF_CONTROLLERS
//	, did_unref_controllers( false )
//#endif // TRACK_UNREF_CONTROLLERS
//{
//	// Assume proper initialization from other controller
//	_animation_ct->ref();
//	if( _timing_ct )
//		_timing_ct->ref();
//	if( _blending_ct )
//		_blending_ct->ref();
//}

// Destructor
MeCtScheduler2::Track::~Track() {
	// starting with shallowest, remove children and unref
	if( _blending_ct ) {
		_blending_ct->unref();
		_blending_ct = NULL;
	}
	if( _timing_ct ) {
		_timing_ct->unref();
		_timing_ct = NULL;
	}
	if( _animation_ct ) {
		_animation_ct->unref();
		_animation_ct = NULL;
	}
	_root = NULL;
}

// Copy Assignment Operator
MeCtScheduler2::Track& MeCtScheduler2::Track::operator=( const MeCtScheduler2::Track& other ) {
	// Assume proper initialization from other controller
	if( _blending_ct != other._blending_ct ) {
		if( _blending_ct ) {
			_blending_ct->unref();
		}
		_blending_ct = other._blending_ct;
		if( _blending_ct ) {
			_blending_ct->ref();
		}
	}
	if( _timing_ct != other._timing_ct ) {
		if( _timing_ct ) {
			_timing_ct->unref();
		}
		_timing_ct = other._timing_ct;
		if( _timing_ct )
			_timing_ct->ref();
	}
	if( _animation_ct != other._animation_ct ) {
		if( _animation_ct ) {
			// _animation_ct should never be NULL
			_animation_ct->unref();
		}
		_animation_ct = other._animation_ct;
		_animation_ct->ref();
	}
	_root = other._root;

    return *this;
}

MeController* MeCtScheduler2::Track::animation_parent_ct() {
	MeCtScheduler2Ptr schedule = _schedule_weak.lock();

	MeController* parent = NULL;
	if( schedule ) {
		parent = schedule.get();
		if( _blending_ct != NULL )
			parent = _blending_ct;
		if( _timing_ct != NULL )
			parent = _timing_ct;
	}

	return parent;
}

bool MeCtScheduler2::Track::evaluate( double time, MeFrameData& frame ) {
	_root->evaluate( time, frame );
	return _root->active();
}

//// No longer needed (Compare TrackPtrs instead)
//bool MeCtScheduler2::Track::operator==( const MeCtScheduler2::Track& other ) const {
//	return ( &_schedule_weak==&(other._schedule_weak) )
//		&& _blending_ct==other._blending_ct
//		&& _timing_ct==other._timing_ct
//		&& _animation_ct==other._animation_ct;
//}

void MeCtScheduler2::Track::remap() {
	if( _root ) {
		//  TODO: Remap Track's _context
		_root->remap();
	}
}

//void MeCtScheduler2::Track::unref_controllers() {
//	if( _root ) {
//		_schedule._sub_sched_context->remove_controller( _root );
//
//		MeController* ct = _animation_ct;
//		_animation_ct = NULL;
//		if( _timing_ct ) {
//			if( ct ) {
//				_timing_ct->init( NULL );  // remove child
//				ct->unref();
//			}
//			ct = _timing_ct;
//			_timing_ct = NULL;
//		}
//		if( _blending_ct ) {
//			if( ct ) {
//				_blending_ct->init( NULL ); // remove child
//				ct->unref();
//			}
//			ct = _blending_ct;
//			_blending_ct = NULL;
//		}
//		if( ct ) {
//			ct->unref();
//			ct = NULL;
//		}
//
//		_root = NULL;
//
//#if TRACK_UNREF_CONTROLLERS
//		did_unref_controllers = true;
//#endif
//	}
//}

//=================================== MeCtScheduler2 =====================================

const char* MeCtScheduler2::type_name = "MeCtScheduler2";

MeCtScheduler2::MeCtScheduler2 ()
:	_self( this, null_deleter() ), // See: http://www.boost.org/doc/libs/1_38_0/libs/smart_ptr/sp_techniques.html#weak_without_shared
	MeCtContainer( new MeCtScheduler2::Context( this ) ),   // Ignore Warning: No writes, just passing a reference
	_sub_sched_context( static_cast<MeCtScheduler2::Context*>( _sub_context) )
{
   _sub_sched_context->ref();
}

MeCtScheduler2::~MeCtScheduler2 () {
   stop (-1);
   _sub_sched_context->unref();
}

MeController* MeCtScheduler2::child( unsigned int n ) {
	if( n >= 0 && n < _tracks.size() )
		return _tracks[ n ]->_root;
	else
		return NULL;
}

void MeCtScheduler2::context_updated() {
	_sub_sched_context->context( _context );
}

MeCtScheduler2::TrackPtr MeCtScheduler2::create_track( MeCtUnary* blending,
                                                       MeCtUnary* timing,
                                                       MeController* animation,
                                                       MeCtScheduler2::TrackPtr before_track )
{
	VecOfTrack::iterator pos;
	if( before_track ) {
		pos = pos_of_track( before_track );
		if( pos == _tracks.end() )
			return TrackPtr();
	} else {
		pos = _tracks.end();
	}

	TrackPtr track( new Track( blending, timing, animation ) );
	track->_schedule_weak = MeCtScheduler2WeakPtr( _self );

	pos = _tracks.insert( pos, track );

	MeController* root = track->_root;

	_child_to_track.insert( make_pair( root, track ) );
	if( animation )
		_anim_to_track.insert( make_pair( track->_animation_ct, track ) );

	_sub_sched_context->add_controller( root );
	_sub_sched_context->child_channels_updated( root );

	return track;
}

MeCtScheduler2::TrackPtr MeCtScheduler2::schedule( MeController* ct, double tin, float indt, float outdt )
{
	double dur = ct->controller_duration();
	bool dur_defined = (dur >= 0);
	double tout = tin + dur;  // Only used if duration is defined.  See below.

	MeCtTimeShiftWarp* timingCt   = new MeCtTimeShiftWarp( ct );
	MeCtBlend*         blendingCt = new MeCtBlend( timingCt );

	const char* ct_name = ct->name();

	// Configure blend curve

	//////////////////////////////////////////////////////
	//  TODO: Utilize slopes when spline is not linear
	//// slopes
	//MeSpline1D::range in_slope = 1/indt;
	//MeSpline1D::range out_slope = -1/outdt;
	//// control point lengths
	//MeSpline1D::range in_cp_length = indt/2;
	//if( in_cp_length > 1 )
	//	in_cp_length = 1;
	//MeSpline1D::range mid_cp_length = (dur-indt-outdt)/2;
	//if( mid_cp_length > 1 )
	//	mid_cp_length = 1;
	//MeSpline1D::range out_cp_length = outdt/2;
	//if( out_cp_length > 1 )
	//	out_cp_length = 1;

	MeSpline1D& bCurve = blendingCt->blend_curve();
	/*                  x           y    (control_tan, l_control_len, and r_control_len are all zero) */
	if( indt>0 ) {
		bCurve.make_smooth( tin,        0,   0, 0, 0 );
		bCurve.make_smooth( tin+indt,   1,   0, 0, 0 );
	} else {
		bCurve.make_disjoint( tin, 1, 0, 0, 0, 0, 0 );
	}
	if( dur_defined ) {
		if( outdt>0 ) {
			bCurve.make_smooth( tout-outdt, 1,   0, 0, 0 );
			bCurve.make_smooth( tout,       0,   0, 0, 0 );
		} else {
			bCurve.make_disjoint( tout, 0, 1, 0, 0, 0, 0 );
		}
	}
	// else leave blend fixed at 100% for eternity
	if( ct_name && (ct_name[0]!='\0') ) {
		string blend_name( "blending for " );
		blend_name += ct_name;
		blendingCt->name( blend_name.c_str() );
	}

	// Configure time mapping
	MeSpline1D& tMapFunc = timingCt->time_func();
	/*                      x        y            (l_tan, l_control_len, r_tan, and r_control_len are all zero) */
	tMapFunc.make_cusp(     tin,     0,            0, 0, 0, 0 );
	if( dur_defined ) {
		tMapFunc.make_cusp( tout,    dur,          0, 0, 0, 0 );
	} else {
		tMapFunc.make_cusp( MAX_TRACK_DURATION, MAX_TRACK_DURATION-tin,  0, 0, 0, 0 );
	}
	if( ct_name && (ct_name[0]!='\0') ) {
		string timing_name( "timing for " );
		timing_name += ct_name;
		timingCt->name( timing_name.c_str() );
	}


	return create_track( blendingCt, timingCt, ct );
}

bool MeCtScheduler2::remove_child( MeController *child ) {
	bool result = remove_child_impl( child );
	if( result )
		recalc_channels_requested();

	return result;
}

bool MeCtScheduler2::remove_track( MeCtScheduler2::TrackPtr track ) {
	bool result = remove_track_impl( track );
	if( result )
		recalc_channels_requested();

	return result;
}

void MeCtScheduler2::remove_tracks( vector< MeCtScheduler2::TrackPtr >& tracks ) {
	VecOfTrack::iterator it = tracks.begin();
	VecOfTrack::iterator end = tracks.end();

	bool did_remove = false;

	for( ; it != end; ++it ) {
		did_remove |= remove_track_impl( *it );
	}

	if( did_remove )
		recalc_channels_requested();
}

// protected
MeCtScheduler2::VecOfTrack::iterator MeCtScheduler2::pos_of_track( TrackPtr track ) {
	return find( _tracks.begin(), _tracks.end(), track );
}

bool MeCtScheduler2::remove_child_impl( MeController *child ) {
	MapOfCtToTrack::iterator map_it = _child_to_track.find( child );
	bool result = (map_it != _child_to_track.end());
	if( result ) {
		TrackPtr track( map_it->second );
		
		VecOfTrack::iterator pos = pos_of_track( track );
		if( pos != _tracks.end() )
			_tracks.erase( pos );

		_child_to_track.erase( child );
		_anim_to_track.erase( track->_animation_ct );

		// Call this last, because it may recurse via the MeControllerContext
		_sub_sched_context->remove_controller( track->_root );
	}

	return result;
}

bool MeCtScheduler2::remove_track_impl( TrackPtr track ) {
	bool result = false;

	if( track ) {
		VecOfTrack::iterator pos = pos_of_track( track );
		if( pos != _tracks.end() ) {
			result = true;
			_tracks.erase( pos );

			if( track->_root != NULL ) {
				_child_to_track.erase( track->_root );
				_anim_to_track.erase( track->_animation_ct );

				// Call this last, because it may recurse via the MeControllerContext
				_sub_sched_context->remove_controller( track->_root );
			}

			// Clear the back reference to the scheduler
			track->_schedule_weak.reset();
		}
	}

	return result;
}

// protected
void MeCtScheduler2::recalc_channels_requested() {
	if( _tracks.empty() ) {
		_channels.init();
	} else {
		// TODO: recalculate channels requested
	}
}


void MeCtScheduler2::clear () {
	VecOfTrack::iterator it = _tracks.begin();
	VecOfTrack::iterator end = _tracks.end();

	for( ; it != end; ++it ) {
		TrackPtr track( *it );

		if( track->_root != NULL )
			remove_child( track->_root );

		track->_schedule_weak.reset();
	}

	_tracks.clear();
	_child_to_track.clear();
	_anim_to_track.clear();

	recalc_channels_requested();
}

MeCtScheduler2::TrackPtr MeCtScheduler2::track_for_child( MeController* ct )
{
	MapOfCtToTrack::iterator i = _child_to_track.find(ct);
	if( i==_child_to_track.end() )
		return TrackPtr();
	else
		return i->second;
}

MeCtScheduler2::TrackPtr MeCtScheduler2::track_for_anim_ct( MeController* ct )
{
	MapOfCtToTrack::iterator i = _anim_to_track.find(ct);
	if( i==_anim_to_track.end() )
		return TrackPtr();
	else
		return i->second;
}

MeCtScheduler2::VecOfTrack MeCtScheduler2::tracks() {
	return VecOfTrack( _tracks );
}


//----- virtuals -----

void MeCtScheduler2::controller_init ()
{
   clear();
}

void MeCtScheduler2::controller_map_updated() {
	// Clear previous data
	_local_ch_to_buffer.assign( _channels.size(), -1 );
	_channels_logged.clear();

	// Map logged channels
	if( _context ) {
		// Get context's logged channels
		const std::set<int>& context_logged_channels = _context->get_logged_channel_indices();
		set<int>::const_iterator context_logged_channels_end = context_logged_channels.end();
		if( DEBUG_LOGGED_CHANNEL_MAPPING ) {
			ostringstream oss;
			set<int>::const_iterator i = context_logged_channels.begin();
			for(; i!=context_logged_channels_end; ++i ) {
				int index = *i;
				SkJointName name = _context->channels().name(index);
				SkChannel::Type type = _context->channels().type(index);
				oss <<'['<<index<<']'<<name.get_string()
					<<'('<<SkChannel::type_name(type)<<"), ";
			}

			cout << "DEBUG: MeCtBlend::controller_map_updated(): Context's logged channels:" << endl
				<< '\t' << oss.str() << endl;
		}

		SkChannelArray& local_channels = controller_channels();
		const int num_channels = local_channels.size();
		for( int i=0; i<num_channels; ++i ) {
			int parent_index = _toContextCh[i];

			if( parent_index >= 0 ) {
// Get a reference to the channel to inspect via debugger
#if DEBUG_INSPECT_CHANNELS
				if( parent_index >= 0 ) {
					SkChannel::Type local_ch_type = local_channels.type( i );
					const char*     local_ch_name = (const char*)(local_channels.name( i ));
					SkChannel::Type parent_ch_type = _context->channels().type( parent_index );
					const char*     parent_ch_name = (const char*)(_context->channels().name( parent_index ));
					int break_here = 1;
				}
#endif
				
				_local_ch_to_buffer[i] = _context->toBufferIndex( parent_index );

				if( context_logged_channels.find(parent_index) != context_logged_channels_end ) {
					// Log this channel
					_channels_logged.insert( i );
					if( DEBUG_LOGGED_CHANNEL_MAPPING ) {
						cout << "DEBUG: MeCtBlend::controller_map_updated(): Logged parent channel "<< parent_index << " maps to " << i << std::endl;
					}
				}
			}
		}
	}

	// remap children
	VecOfTrack::iterator end = _tracks.end();
	for( VecOfTrack::iterator i=_tracks.begin(); i!=end; ++i ) {
		TrackPtr track = *i;
		// TrackPtr in _tracks should not be null
		MeController* ct = track->_root;
		if( ct )
			ct->remap();
	}
}


////  Predicate for uniniting past Tracks
//class TrackFinalizer : public std::unary_function<MeCtScheduler2::Track,bool> {
//    const double time; // time
//public:
//    TrackFinalizer( const double time )
//        : time( time )
//    {}
//
//    bool operator() ( MeCtScheduler2::Track& tr ) const {
//        if( tr._type==MeCtScheduler2::Once && 
//            tr._tout>=0 && 
//            time>tr._tout+tr._ext )
//        {
//            tr.controller()->stop();
//            tr.uninit();
//            return true;
//        } else {
//            return false;
//        }
//    }
//};



bool MeCtScheduler2::controller_evaluate( double time, MeFrameData& frame ) {
	vector< TrackPtr > to_remove( 0 );  // zero size because it is rarely used (don't allocate for array until needed)

	VecOfTrack::iterator end = _tracks.end();
	for( VecOfTrack::iterator i = _tracks.begin(); i != end; ++i ) {
		TrackPtr track = *i;
		bool result = track->evaluate( time, frame );
		
		if( _automatically_remove_tracks && !result ) {
			// TODO: unify prune policy of grouping controllers like blend/timing with DefaultContainerPrunePolicy
			MeController* anim_ct = track->animation_ct();
			MePrunePolicy* prune_policy = NULL;
			if( anim_ct != NULL )
				prune_policy = anim_ct->prune_policy();
			if(    prune_policy == NULL
			    || prune_policy->shouldPrune( anim_ct, track->animation_parent_ct() ) )
			{
				to_remove.push_back( track );
			}
		}
	}

	if( !to_remove.empty() )
		remove_tracks( to_remove );
	
	return _active_when_empty || !_tracks.empty();



	///////////////////////////////////////////////////////////////////
	//// Old Scheduler Code	
	//if ( !_channels.size() ) return false; // no tracks, not active

	//MeController* ct;
	//Track* tr;
	//unsigned int i;

	///*  // Original Code for SrArray _tracks
	//// remove 'Once' controllers that have finished:
	//// note: when a controller with undetermined duration ends its tout is automatically set
	//i=0;
	//while ( i<_tracks.size() ) {
	//tr = &_tracks[i];
	//if( tr->_type==Once && tr->_tout>=0 && t>tr->_tout+tr->_ext ) { // remove this track
	//tr->_controller->stop();
	//tr->uninit();
	//_tracks.remove(i); // after this call tr pointer becomes unusable
	//} else {
	//i++;
	//}
	//}
	//*/
	//std::vector<Track>::iterator last = std::remove_if( _tracks.begin(), _tracks.end(),
	//	TrackFinalizer(t) );
	//_tracks.erase( last, _tracks.end() );


	//unsigned int trsize = _tracks.size();
	//if ( trsize==0 ) return false; // no tracks, not active anymore

	//// Mark/check controllers to be played:
	//int toplay = 0;
	//for ( i=0; i<trsize; i++ ) {
	//	tr = &_tracks[i];
	//	tr->_toplay = false; // mark as not to play
	//	if( t<tr->_tin+tr->_waitdt )
	//		continue; // not yet started
	//	if( tr->_tout>0 && t>tr->_tout+tr->_ext ) { // finished
	//		ct = tr->controller();
	//		if( ct->active() )
	//			ct->stop();
	//		continue;
	//	}

	//	tr->_toplay = true; // mark as to be played
	//	toplay++; // count number of motions to play
	//}

	//if ( toplay==0 ) return true; // no tracks were toplay at this time

	//// Evaluate the tracks and blend the buffers marked to be played:
	//float* trbuff;
	//float* buff = &buffer()[0];
	//float tloc;

	//int k;     // current active track
	//int c;     // cur channel in _channels
	//unsigned int csize; // cur channel size
	//int cf=0;  // cur channel float in _channels
	//int channels_size = _channels.size();
	//int tracks_size = _tracks.size();

	//bool easein, easeout;

	//for ( k=0; k<tracks_size; k++ )
	//{ tr = &_tracks[k];
	//if ( !tr->_toplay ) continue; // was not toplay

	//cf = 0;
	//buff = &buffer()[0];
	//ct = tr->controller();

	//double ctDuration = tr->controller()->controller_duration();
	//double tLocal = (t - tr->_tin) * tr->_speed;
	//if( tr->_toclamp ) {
	//	if( tLocal < 0 )
	//		tLocal = 0;  // first frame
	//	if( ctDuration >=0 &&
	//		tLocal > ctDuration )
	//		tLocal = ctDuration;  // final frame
	//}
	//double tEnd = tr->_tin;

	////  Anm: tout is not necessaily controller bounds..
	////       Allow controller to decide whether to evaluate.
	////       See new test in MeController::evaluate()
	////if ( tr->tout>0 && t>tr->tout ) {  // in extension period: no evaluation
	////} else {
	//if ( !ct->active() )
	//	ct->start();
	//// TODO!  Replace with temp FrameData/buffer
	//ct->evaluate ( float(tLocal), frame ); // evaluate with local time
	//if( !ct->active() && tr->_tout<0 )
	//	tr->_tout = t; // tout is now determined
	////}

	//easein = t<tr->_tin+double(tr->_waitdt+tr->_indt)? true:false; // check if in ease-in phase
	//easeout = tr->_tout>0 && t>tr->_tout+double(tr->_ext-tr->_outdt)? true:false; // check if in ease-out phase

	//for ( c=0; c<channels_size; c++ )
	//{ csize = _channels[c].size();
	//int ctChannel = ct->map()[cf];
	//if ( ctChannel>=0 ) // channel used by this track
	//{
	//	trbuff = &ct->buffer()[ctChannel];

	//	if ( easein )
	//	{ tloc =  float(t-(tr->_tin+tr->_waitdt))/tr->_indt;
	//	_channels[c].interp ( buff, buff, trbuff, spline3(tloc) );
	//	}
	//	else if ( easeout )
	//	{ tloc =  float(tr->_tout+tr->_ext-t)/tr->_outdt;
	//	_channels[c].interp ( buff, buff, trbuff, spline3(tloc) );
	//	}
	//	else // just copy values
	//	{ for ( i=0; i<csize; i++ ) buff[i]=trbuff[i];
	//	}
	//}
	//cf += csize;
	//buff  += csize;
	//}
	//}

	//return true; // returns the activation state
}

SkChannelArray& MeCtScheduler2::controller_channels ()
{
   return _channels;
}

double MeCtScheduler2::controller_duration () {
	double total_dur=0;

	VecOfTrack::iterator end = _tracks.end();
	for( VecOfTrack::iterator it = _tracks.begin(); it != end; ++it ) {
		TrackPtr track = (*it);

		double ct_dur = track->_root->controller_duration();
		if ( ct_dur<0 ) {
			total_dur = -1;

			break;
		} else {
			total_dur = max( ct_dur, total_dur );
		}
	}
	return total_dur;
}

const char* MeCtScheduler2::controller_type () const
 {
   return type_name;
 }

void MeCtScheduler2::print_state( int tab_count ) {
	++tab_count;  // we indent the track info also
	string indent( tab_count, '\t' );

	cout << "MeCtScheduler2";

	const char* str_name = name();
	if( str_name[0] )
		cout << " \"" << str_name << "\"";
	cout << ":";

	//  Print tracks in reverse order (highest priority first)
	if( _tracks.size()>0 ) {
		cout << endl;
		
		int count = 0;
		VecOfTrack::iterator end = _tracks.end();
		for( VecOfTrack::iterator it = _tracks.begin(); it != end; ++it )
		{
			TrackPtr track = (*it);

			cout << indent << "Track #" << (++count) << ": ";

			MeController* ct = track->_root;
			if( ct )
				ct->print_state( tab_count );
			else
				cout <<  "NULL _root!!!" << endl;
		}
	} else {
		cout << " No Tracks" << endl;
	}
}

//======================================= EOF =====================================
