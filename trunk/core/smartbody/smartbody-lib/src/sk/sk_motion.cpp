/*
 *  sk_motion.cpp - part of Motion Engine and SmartBody-lib
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
 *      Marcelo Kallmann, USC (currently at UC Merced)
 *      Andrew n marshall, USC
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 */

#include "vhcl.h"
# include <math.h>
# include <stdlib.h>
# include <string.h>
#include <iostream>

# include <SK/sk_motion.h>
# include <SK/sk_posture.h>
# include <SK/sk_skeleton.h>
#include "sbm/Event.h"
#include <algorithm>
#include <sbm/gwiz_math.h>
#include <boost/algorithm/string.hpp>

using namespace gwiz;

//============================= SkMotion ============================

SkMotion::SkMotion() :
	_postsize( 0 ),
	_name( NULL ),
	_filename( NULL ),
	_skeleton( NULL ),
	_floatbuffer( NULL ),
	_last_apply_frame( 0 )
#if 0
	,
	_time_ready( -1 ),
	_time_stroke_start( -1 ),
	_time_stroke_emphasis( -1 ),
	_time_stroke_end( -1 ),
	_time_relax( -1 )
#endif
{}

SkMotion::~SkMotion()
 {
   init ();
   delete[] _name;
   delete[] _filename;

   int f;
   for ( f = 0; f < _frames.size(); f++ )
   {
      free( _frames[f].posture );
      _frames[f].posture = NULL;
   }

   for (size_t x = 0; x < _motionEvents.size(); x++)
	   delete _motionEvents[x];
 }

void SkMotion::init()
 {
   _skeleton = 0;
   _floatbuffer = 0;
   _postsize = 0;

   while ( _frames.size()>0 )
    free ( _frames.pop().posture );

   _channels.init();

   _last_apply_frame = 0;
 }

void SkMotion::init ( const SkChannelArray& ca )
 {
   init ();
   _channels = ca;
   _postsize = _channels.floats();
 }
 
void SkMotion::compress()
 {
   _frames.compress ();
   _channels.compress ();
 }

SkMotion* SkMotion::buildMirrorMotion()
{	
	SkChannelArray& mchan_arr = this->channels();
	SkMotion *mirror_p = new SkMotion;
	mirror_p->synch_points = srSynchPoints(synch_points);
	mirror_p->init( mchan_arr );
	int num_f = this->frames();
	for (int i = 0; i < num_f; i++)
	{
		mirror_p->insert_frame( i, this->keytime( i ) );
		float *ref_p = this->posture( i );
		float *new_p = mirror_p->posture( i );
		// go through each channel, and flip the channel value when necessary
		for (int k=0;k<mchan_arr.size();k++)
		{
			SkChannel& chan = mchan_arr[k];
			std::string jointName = mchan_arr.name(k).get_string();
			int index = mchan_arr.float_position(k);
			if (chan.type == SkChannel::XPos)
			{
				new_p[index] = -ref_p[index]; // flip x-translation
			}
			else if (chan.type == SkChannel::Quat)
			{
				// flip rotation for y,z axis
				euler_t ref_e = quat_t( ref_p[ index ], ref_p[ index + 1 ], ref_p[ index + 2 ], ref_p[ index + 3 ] );
				quat_t new_q = euler_t( ref_e.x(), -ref_e.y(), -ref_e.z() );
				new_p[ index + 0 ] = (float)new_q.w();
				new_p[ index + 1 ] = (float)new_q.x();
				new_p[ index + 2 ] = (float)new_q.y();
				new_p[ index + 3 ] = (float)new_q.z();
			}
			else
			{
				for (int n=0;n<chan.size();n++)
					new_p[index+n] = ref_p[index+n];
			}
		}
		for (int k=0;k<mchan_arr.size();k++)
		{
			SkChannel& chan = mchan_arr[k];
			std::string jointName = mchan_arr.name(k).get_string();
			int index = mchan_arr.float_position(k);
			if (boost::algorithm::starts_with(jointName,"l_"))
			{
				std::string jointNameRight = jointName;
				jointNameRight[0] = 'r';  // get the mirror joint name
				int rjointIndex = mchan_arr.search( SkJointName( jointNameRight.c_str() ), chan.type );
				if (rjointIndex < 0)
					continue;
				int rindex = mchan_arr.float_position(rjointIndex);				
				// swap left and right channels
				for (int n=0;n<chan.size();n++)
				{
					std::swap(new_p[rindex+n],new_p[index+n]);
				}
			}
		}				
	}
	return mirror_p;
}

bool SkMotion::create_from_postures ( const SrArray<SkPosture*>& keypost, 
                                      const SrArray<float>& keytime )
 {
   if ( keypost.size()<1 || keypost.size()!=keytime.size() ) return false;

   SrArray<int> index;
   if ( !_channels.get_used_channels(keypost,&index) ) return false;

   // create the frames:
   int i, j;
   _postsize = _channels.floats();
   _frames.size ( keypost.size() );
   _frames.size ( 0 );
   for ( i=0; i<keypost.size(); i++ )
    { insert_frame ( i, keytime[i] );
      for ( j=0; j<index.size(); j++ )
       { if ( index[j]<0 ) continue;
         _frames[i].posture[index[j]] = keypost[i]->values[j];
       }
    }
   return true;
 }

bool SkMotion::insert_channel ( int pos, const char* name, SkChannel::Type type, float* values )
 {
   if ( !_channels.insert(pos,name,type) ) return false;
   
   // add position in all frames:
   int ins = SkChannel::size(type);
   int f, i, size;
   for ( f=0; f<_frames.size(); f++ )
    { size = _postsize; // we need to save size here as buffer_insert will update it
      _frames[f].posture =
       (float*) sr_buffer_insert ( _frames[f].posture, sizeof(float), size, pos, ins );
      for ( i=0; i<ins; i++ )
       _frames[f].posture[pos+i] = values[i];
    }

   _postsize+=ins;

   return true;
 }

bool SkMotion::insert_frame ( int pos, float kt )
 {
   if ( pos<0 || pos>_frames.size() ) return false;
   _frames.insert ( pos );
   _frames[pos].keytime = kt;
   _frames[pos].posture = (float*) malloc ( sizeof(float)*_postsize );
   return true;
 }

int SkMotion::connect ( SkSkeleton* s )
 {
   _skeleton = s;
   _floatbuffer = 0;
   return _channels.connect(s);
 }

void SkMotion::connect ( float* buffer )
 {
   _skeleton = 0;
   _floatbuffer = buffer;
 }

void SkMotion::apply_frame ( int f ) {
	apply_frame( f, _floatbuffer, NULL );
}
void SkMotion::apply_frame ( int f, float* buffer, SrBuffer<int>* map_p ) {
	if ( _frames.size()==0 )
		return;

	int i = _frames.size()-1;
	f = SR_BOUND(f,0,i);

	float* fp = _frames[f].posture;

	if ( buffer ) {
		if( map_p ) {
			int num;
			int csize = _channels.size();
			for ( int i=0; i<csize; i++ ) {
				// channel size
				num = _channels[i].size();

				// Find and copy parent data
				int index = map_p->get( i );
				if( index >= 0 ) {
					// channel exists in data
					float* v = buffer + map_p->get(i);  // pointer to start of this channel's floats
					for( int j=0; j<num; ++j )
						v[j] = fp[j];
				} // else skip data

				// Increment motion data pointer by channel size
				fp+=num;
			}
		} else {
			// note: memcpy does not handle overlap
			memcpy ( buffer /*dest*/, fp /*src*/, sizeof(float)*_postsize /*bytes*/ );
		}
	}
	else
	{
		// Apply to channel joints
		int inserted;
		int size = _channels.size();

		for ( i=0; i<size; i++ ) {
			if ( _channels[i].joint )
				inserted = _channels[i].set ( fp );
			else
				inserted = _channels[i].size ();
			fp += inserted;
		}
	}
}

inline float _cubic ( float t, float tmin, float tmax ) {
   t = (t-tmin)/(tmax-tmin);    // normalize t to [0,1]
   t=-(2.0f*(t*t*t)) + (3.0f*(t*t));  // cubic spline
   return t*(tmax-tmin) + tmin; // scale back
   // shape comparison with sine for graphmatica:
   // y=-(2.0*(x*x*x)) + (3.0*(x*x))
   // y=sin((x-0.5)*3)/2+0.5
}


void SkMotion::apply ( float t, SkMotion::InterpType itype, int* lastframe ) {
	apply( t, _floatbuffer, NULL, itype, lastframe );
}

#define DEBUG_T 0

void SkMotion::apply ( float t,  
                       float* buffer, SrBuffer<int>* map_p,
                       SkMotion::InterpType itype, int* lastframe )
{
	int fsize=_frames.size();
	if ( fsize<=0 )
		return;
	if ( t<_frames[0].keytime )	{
#if DEBUG_T
		LOG("SkMotion::apply NOTICE: t=%.16f < f[0]:%.16f \n", t, _frames[0].keytime );
#endif
		return;
	}

	if ( itype==CubicSpline )
		t = _cubic ( t, _frames[0].keytime, _frames.top().keytime );

#if DEBUG_T
	if ( t<_frames[0].keytime )	{
		LOG("SkMotion::apply ERR: cubic t=%.16f < f[0]:%.16f \n", t, _frames[0].keytime );
	}
#endif

	// optimize keytime search for sequenced calls to apply
	int fini=0;
	if ( lastframe )
		_last_apply_frame = *lastframe;
	if ( _last_apply_frame>0 && _last_apply_frame<fsize ) {
		if ( t>_frames[_last_apply_frame].keytime )
			fini=_last_apply_frame+1;
    }

	int f;
	for ( f=fini; f<fsize; f++ ) {
		if ( t<_frames[f].keytime )
			break;
	}

	if ( f==_frames.size() ) {
		// Apply last frame
		apply_frame( f-1, buffer, map_p );
		return;
	}

	f--;
	_last_apply_frame = f;
	if ( lastframe )
		*lastframe = _last_apply_frame;

	float* fp1 = _frames[f].posture;
	float* fp2 = _frames[f+1].posture;

	// convert t to [0,1] according to the keytimes:
	t = (t-_frames[f].keytime) / (_frames[f+1].keytime-_frames[f].keytime);

#if DEBUG_T
	if ( t<0.0 )	{
		LOG("SkMotion::apply ERR: mapped t=%.16f < 0.0 \n", t );
	}
#endif

	//sr_out<<"t: "<<t<<" frames: "<<f<<srspc<<(f+1)<<"\n";
	int num;
	int csize = _channels.size();

	if ( buffer ) {
		// Apply to float* buffer
		if ( map_p ) {
			for ( int i=0; i<csize; i++ ) {
				// Channel size
				num = _channels[i].size();

				// Find mapped buffer location
				int index = map_p->get( i );
				if( index >= 0 ) {
					// channel exists in data
//					float* v = buffer + map_p->get(i);  // pointer to start of this channel's floats
					float* v = buffer + index;  // pointer to start of this channel's floats
					num = _channels[i].interp ( v, fp1, fp2, t );
				}

				// Increment frame data pointers
				fp1+=num; fp2+=num;
			}
		} else {
			//  Assume float data is in motion's channel order
			float* v = buffer;  // point to the start of all floats
			for ( int i=0; i<csize; i++ ) {
				num = _channels[i].interp ( v, fp1, fp2, t );
				v+=num; fp1+=num; fp2+=num;
			}
		}
	}
	else
	{
		// Apply to channel joints
		float values[4]; // 4 is the max num of values per channel
		for ( int i=0; i<csize; i++ ) {
			if ( _channels[i].joint ) {
				_channels[i].interp ( values, fp1, fp2, t );
				num = _channels[i].set ( values );
			} else {
				num = _channels[i].size();
			}

			fp1+=num; fp2+=num;
		}
	}
}

// static 
const char* SkMotion::interp_type_name ( InterpType type ) {
	switch ( type ) {
		case CubicSpline :
			return "CubicSpline";
		default :
			return "Linear";
    }
}

// static 
SkMotion::InterpType SkMotion::interp_type_name ( const char* type ) {
   if ( sr_compare(type,"CubicSpline")==0 )
	   return CubicSpline;
   return Linear;
 }

void SkMotion::operator = ( const SkMotion& m )
 {
   disconnect();
   init();
   name(m.name());

   int i;
   _postsize = m._postsize;
   _skeleton = m._skeleton;
   _floatbuffer = m._floatbuffer;
   _channels = m._channels;

   int fsize = m._frames.size();
   _frames.size ( fsize );
   _frames.size ( 0 );
   for ( i=0; i<fsize; i++ )
    { insert_frame ( i, m._frames[i].keytime );
      sr_buffer_copy ( _frames[i].posture, sizeof(float), _postsize, m._frames[i].posture, _postsize );
    }
 }

void SkMotion::move_keytimes ( float startkt )
 {
   if ( _frames.size()==0 ) return;
   
   float diff = _frames[0].keytime-startkt;
   if ( diff==0 ) return;

   int i;
   for ( i=0; i<_frames.size(); i++ )
    { _frames[i].keytime -= diff;
    }
 }

static float _correct ( float a1, float a2 )
 {
   if ( SR_DIST(a1,a2)<=srpi ) return a2;

   if ( a2>a1 )
    return _correct ( a1, a2-sr2pi );
   else
    return _correct ( a1, a2+sr2pi );
 }

void SkMotion::correct_euler_angles ()
 {
   int frsize = _frames.size();
   
   if ( frsize<=1 ) return;

   SkChannel::Type type;
   int i, j, p=0;
   int chsize = _channels.size();

   for ( i=0; i<chsize; i++ )
    { type = _channels.type(i);
      if ( type<SkChannel::XRot || type>SkChannel::ZRot )
       { p += SkChannel::size ( type );
       }
      else
       { for ( j=1; j<frsize; j++ )
          _frames[j].posture[p] = _correct ( _frames[j-1].posture[p], _frames[j].posture[p] );
         p++;
       }
    }
 }

void SkMotion::change_channel_values ( int f1, int f2, int channel, float mfactor, const float* offset )
 {
   int fsize = _frames.size();
   if ( fsize==0 ) return;
   f2 = SR_BOUND(f2,0,(fsize-1));
   f1 = SR_BOUND(f1,0,f2);
   SkChannel& ch = _channels[channel];
   int fp = _channels.float_position ( channel );
   int f;
   for ( f=f1; f<=f2; f++ )
    { ch.change_values ( _frames[f].posture+fp, mfactor, offset );
    }
 }

void SkMotion::registerAnimation()
{
	_registerOffset[0] = 0;
	_registerOffset[1] = 0;
	_registerOffset[2] = 0;
	_registerOrientation.set(1, 0, 0, 0);

	if (_frames.size() == 0)
		return;

	SkJointName jname("base");
	// get the offset from the first frame
	SkChannelArray& channels = this->channels();
	int xIndex = channels.search(jname, SkChannel::XPos);
	int zIndex = channels.search(jname, SkChannel::ZPos);
	
	if (xIndex == -1 && zIndex == -1)
		return;

	// subtract that offset from all the other animation clips
	for (int f = 0; f < _frames.size(); f++)
	{	
		if (f == 0)
		{
			if (xIndex >= 0)
				_registerOffset[0] = _frames[f].posture[xIndex];
			if (zIndex >= 0)
				_registerOffset[2] = _frames[f].posture[zIndex];

			if (_registerOffset[0] != 0.0 || _registerOffset[2] != 0.0)
				LOG("Animation %s registered with offset (%f, %f)", this->name(), _registerOffset[0], _registerOffset[2]);
		}
		if (xIndex >= 0)
			_frames[f].posture[xIndex] -= _registerOffset[0];
		if (zIndex >= 0)
			_frames[f].posture[zIndex] -= _registerOffset[2];
	}
}

/*
// static:
struct JT { SkJoint* j; char t[7]; float v[7]; };

static void equalize ( SrArray<JT>& used, SkMotion* m )
 {
   SkJoint* joint;
   SkChannel::Type type;

   int c=0;
   int i, j;
   for ( i=0; i<used.size(); i++ )
    { joint = used[i].j;
      if ( !joint ) continue;
      for ( j=0; j<7; j++ )
       { if ( !used[i].t[j] ) continue;
         type = (SkChannel::Type)j;
         if ( m->channel_joint(c)!=joint || m->channel_type(c)!=type ) // not there?
           m->add_channel ( c, joint->name(), type, used[i].v[j] ); // add it.
         c++;
       }
    }
 }
    *! Makes all motions in the array to have the same number of channels.
        Note: the motions are connected to sk and then disconnected. *
    static void equalize_channels ( SkSkeleton* sk, SrArray<SkMotion*> motions );

void SkMotion::equalize_channels ( SkSkeleton* sk, SrArray<SkMotion*> motions )
 {
   if ( !sk || motions.size()<=1 ) return;

   int i, j, type;
   SkJoint* joint;
   const SrArray<SkJoint*>& joints = sk->joints();
   SrArray<JT> used;

   // initialize "used dofs" array:
   used.size ( joints.size() );
   for ( i=0; i<used.size(); i++ )
    { used[i].j=0;
      for ( j=0; j<7; j++ ) { used[i].t[j]=0; used[i].v[j]=5; } // 5 value is to debug
    }
   
   // mark used dofs of all motions:
   for ( i=0; i<motions.size(); i++ )
    { motions[i]->connect(sk);

      for ( j=0; j<motions[i]->channels(); j++ )
       { joint = motions[i]->channel_joint(j);
         if ( !joint ) continue;
         type = (int)motions[i]->channel_type(j);
         used[ joint->index() ].j = joint;
         used[ joint->index() ].t[type] = 1;
         used[ joint->index() ].v[type] = motions[i]->posture(0)[j];
       }
    }

   for ( i=0; i<motions.size(); i++ )
    { equalize ( used, motions[i] );
      motions[i]->disconnect();
    }
 }

    Makes all motions in the array to have the same number of frames.
    static void equalize_frames ( SrArray<SkMotion*> motions );
void SkMotion::equalize_frames ( SrArray<SkMotion*> motions )
 {
// resampling
 }
*/

//============================ End of File ===========================

bool ascendingTime(MotionEvent*& a, MotionEvent*& b)
{
	return (a->getTime() < b->getTime());
}

void SkMotion::addMotionEvent(MotionEvent* motionEvent)
{
	_motionEvents.push_back(motionEvent);
	// make sure that the motion events are ordered by time
	std::sort(_motionEvents.begin(), _motionEvents.end(), ascendingTime);
}

std::vector<MotionEvent*>& SkMotion::getMotionEvents()
{
	return _motionEvents;
}
