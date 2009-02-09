/*
 *  me_controller.cpp - part of Motion Engine and SmartBody-lib
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

#include <iostream>
#include <sstream>

#include <ME/me_controller.h>
#include <SR/sr_quat.h>
#include <ME/me_prune_policy.hpp>


using namespace std;

#define WARN_ON_INVALID_BUFFER_INDEX (0)
#define TRACE_REMAP_CHANNELS         (0)


//============================= MeController ============================

int MeController::instance_count = 0;

MeController::MeController () 
:	_active( false ),
	_indt( 0.0f ),
	_outdt( 0.0f ),
	_name( NULL ),
	_emphasist( -1.0f ),
	_lastEval( -1 ),  // effectively unset.. should really be a less likely value like NaN
	_prune_policy( new MeDefaultPrunePolicy() ),
	_context( NULL ),
 	_record_output( NULL ), // for recording poses and motions of immediate local results
	_record_motion( NULL ),
	_record_pose( NULL )
{
	_instance_id = instance_count;
	instance_count ++;
	_invocation_count = -1;

	_prune_policy->ref();

	_record_mode = RECORD_NULL;
	_recording = false;
	_record_num_frames = 0;
	_record_frame_count = 0;
}

MeController::~MeController () {
	//assert( _context==NULL );  // Controller should not be deleted if still referenced by context
	stop_record();

	if( _prune_policy ) {
		_prune_policy->unref();
		_prune_policy = NULL;
	}

    delete[] _name;
}

void MeController::clone_parameters( MeController* other ) {
	if( _name )
		delete[] _name;
	if( other->_name ) {
		int len = strlen(other->_name)+1;
		_name = new char[len];
		strcpy( _name, other->_name );
	}
	_indt = other->_indt;
	_outdt = other->_outdt;
	_emphasist = other->_emphasist;
}

void MeController::emphasist ( float t )
{ 
    if ( t<0 ) {
        t=-1.0f;
    }
    else
    {
        double d = controller_duration();
        if ( d>=0 && double(t)>d )
            t=float(d);
    }
    _emphasist=t;
}

void MeController::inoutdt ( float indt, float outdt )
 {
   float d = (float)controller_duration();

   if ( d>=0 && indt+outdt>d )
    { float factor = d/(indt+outdt);
      indt *= factor;
      outdt *= factor;
    }

   _indt = indt;
   _outdt = outdt;
 }

void MeController::remove_all_children() {
	size_t n = count_children();
	while( n != 0 ) {
		remove_child( child( --n ) );
	}
}

void MeController::init () {
	_active = false;
	controller_init ();

	if( _context )
		_context->child_channels_updated( this );
}

MePrunePolicy* MeController::prune_policy () {
	return _prune_policy;
}

void MeController::prune_policy( MePrunePolicy* prune_policy ) {
	if( _prune_policy != prune_policy ) {
		if( _prune_policy != NULL ) {
			_prune_policy->unref();
			_prune_policy = NULL;
		}
		_prune_policy = prune_policy;
		if( _prune_policy != NULL ) {
			_prune_policy->ref();
		}
	}
}


void MeController::start () {

	_active = true;
	_invocation_count++;
	controller_start ();
}

void MeController::stop () {

	_active = false;
	if( _recording ) {
		stop_record();
	}
	controller_stop ();
// printf( ">>> MeController::stop <<<\n" );
}

void MeController::remap() {
#if TRACE_REMAP_CHANNELS
	std::cout << "========= Entering MeController::remap() for " << controller_type() << " \"" << name() << "\" ==" << std::endl;
#endif

	SkChannelArray& localChnls = controller_channels();
	int size = localChnls.size();
	_toContextCh.size( size );

	SkChannelArray& contextChnls = _context->channels();

	for( int i=0; i<size; ++i ) {
		SkJointName name = localChnls.name( i );
		SkChannel::Type type = localChnls.type( i );
		_toContextCh[i] = contextChnls.search( name, type );

		int parent_index = _toContextCh[i];
		if( parent_index >= 0 ) {
#if WARN_ON_INVALID_BUFFER_INDEX
			// WARN for invalid parent context buffer indices
			if( _context->toBufferIndex( parent_index ) < 0 ) {
				const char* parent_ch_type = SkChannel::type_name( _context->channels().type( parent_index ) );
				const char* parent_ch_name = (const char*)(_context->channels().name( parent_index ));
				std::cerr << "WARNING: MeController::remap(): "<<controller_type()<<" \""<<this->name()<<"\": "
					<<_context->context_type()<<" channel "<<parent_index<<", \""<<parent_ch_name<<"\" ("<<parent_ch_type<<") lacks valid buffer index!!" << std::endl;
			}
#endif

#if TRACE_REMAP_CHANNELS
			// Get a reference to the channel to inspect via debugger
			const char*     local_ch_name = name.get_string();
			SkChannel::Type parent_ch_type = _context->channels().type( parent_index );
			const char*     parent_ch_name = (const char*)(_context->channels().name( parent_index ));

			// Print it out...
			std::cout << "L#"<< i << "\t-> P#"<<parent_index<<"\t\t"<<local_ch_name<<" ("<<SkChannel::type_name(type)<<")"<< std::endl;

			if( strcmp( local_ch_name, parent_ch_name ) || (type != parent_ch_type ) ) {
				std::cerr << "ERROR: MeController::remap(): " << controller_type() << " \"" << _name << "\" :"
					<< " Local \"" << local_ch_name << "\" " << SkChannel::type_name(type) << " != "
					<< " Parent \"" << parent_ch_name << "\" " << SkChannel::type_name(parent_ch_type) << std::endl;
			}

#endif
		}
	}

	controller_map_updated();

#if TRACE_REMAP_CHANNELS
	std::cout << "========= Exiting MeController::remap() for " << controller_type() << " \"" << name() << "\" ==" << std::endl;
#endif
}

void MeController::evaluate ( double time, MeFrameData& frame ) {

	MeEvaluationLogger* logger = _context->get_evaluation_logger();
	if( logger )
		logger->controller_pre_evaluate( time, *_context, *this, frame );

	// Reevaluate controller. Even for the same evaluation time as _lastEval, results may be influenced by differing buffer values
	_active = controller_evaluate ( time, frame );
	
	if( _record_mode ) {
		if( _recording == false )	{
			init_record();
		}
		cont_record( time, frame );
	}

	if( logger )
		logger->controller_post_evaluate( time, *_context, *this, frame );
}

void MeController::record_motion( const char *full_prefix, int num_frames ) { 
	if( _recording )	{
		stop_record();
	}
	_record_mode = RECORD_MOTION; 
	_record_num_frames = num_frames;
	_record_full_prefix = std::string( full_prefix ); 
}

/*
void MeController::record_pose( const char *full_prefix ) { 
	_record_mode = RECORD_POSE; 
	_record_full_prefix = std::string( full_prefix ); 
}
*/

bool MeController::init_record( void )	{
	string filename;
	ostringstream record_id_oss;

	if( _name == NULL )	{
		name( "noname" );
	}
	record_id_oss << _instance_id << "." << _invocation_count << "_R";
	string recordname = string( controller_type() ) + "_" + string( _name ) + "_" + record_id_oss.str();
	if( _record_mode == RECORD_MOTION )	{
		filename = _record_full_prefix + recordname + ".skm";
		_record_motion = new SkMotion;
		_record_motion->name( recordname.c_str() );	
	}
	else	{
		filename = _record_full_prefix + recordname + ".skp";
		_record_pose = new SkPosture;
		_record_pose->name( recordname.c_str() );
	}

	_record_output = new SrOutput( filename.c_str(), "w" );
	*_record_output << "# SKM Motion Definition - M. Kallmann 2004\n";
	*_record_output << "# Maya exporter v0.6\n";
	*_record_output << "# Recorded output from MeController\n\n";
	*_record_output << "SkMotion\n\n";
	*_record_output << "name \"" << recordname.c_str() << "\"\n\n";

	SkChannelArray& channels = controller_channels();
	*_record_output << channels << srnl;
	*_record_output << "frames " << _record_num_frames << srnl;
	
	_recording = true;
	cout << "MeController::init_record: " << filename << endl;
	return( true );
}

void MeController::cont_record( double time, MeFrameData& frame )	{
	
	if( _record_frame_count >= _record_num_frames )	{
		stop_record();
		return;
	}
	if ( time < 0.0001 ) time = 0.0;

	ostringstream key_time_oss;
	key_time_oss << "kt " << time << " fr ";
	*_record_output << key_time_oss.str().c_str();

	SkChannelArray& channels = controller_channels();
	int num_channels = channels.size();
	SrBuffer<float>& buff = frame.buffer();

	int i, j;
	for( i=0; i<num_channels; i++ )	{
		
		int index = frame.toBufferIndex( _toContextCh[ i ] );
		int channel_size = channels[ i ].size();
		
		// SKM format does not actually store a quat, it stores an 'axisangle'
		if( channels[ i ].type == SkChannel::Quat )	{
			SrQuat q ( buff[ index + 0 ], buff[ index + 1 ], buff[ index + 2 ], buff[ index + 3 ] );
			SrVec axis = q.axis();
			float ang = q.angle();
			axis.len ( ang );		
			*_record_output << axis.x << " ";
			*_record_output << axis.y << " ";
			*_record_output << axis.z << " ";
		}
		else
		for( j=0; j<channel_size; j++ )	{
			*_record_output << buff[ index + j ] << " ";
		}
	}
	*_record_output << srnl;
	
	_record_frame_count++;
}

void MeController::stop_record( void )	{
	_recording = false;
	_record_mode = RECORD_NULL;
	_record_num_frames = 0;
	_record_frame_count = 0;
	if( _record_output )	{
		delete _record_output;
		_record_output = NULL;
	}
	if( _record_motion )	{
		delete _record_motion;
		_record_motion = NULL;
	}
	if( _record_pose )	{
		delete _record_pose;
		_record_pose = NULL;
	}
}

void MeController::output ( SrOutput& o )
 {
   SrString n;
   n.make_valid_string ( name() );
   o << n << " inout " << _indt << srspc << _outdt << srnl;
   if ( _emphasist>=0 )
     o << "emphasist " << _emphasist << srnl;
 }

void MeController::input ( SrInput& i )
 {
   i.get_token();
   name ( i.last_token() );
   i.get_token(); // inout
   i >> _indt;
   i >> _outdt;
   i.get_token();
   if ( i.last_token()=="emphasist" )
    { i >> _emphasist; }
   else
    { _emphasist=-1.0; i.unget_token(); }
 }

void MeController::print_state( int tab_count ) {
	using namespace std;

	cout << controller_type() << "@0x" << this << endl;

	print_children( tab_count );
}

void MeController::print_children( int tab_count ) {
	using namespace std;

	int count = count_children();
	if( count>0 ) {
		std::string indent( ++tab_count, '\t' );
		for( int i=0; i<count; ++i ) {
			MeController* ct = child( i );
			if( ct ) {
				cout << endl << indent;
				ct->print_state( tab_count );
			} else {
				cout << endl << indent << "Child " << i << " is NULL" << endl;
			}
		}
	}
}

//============================ End of File ============================
