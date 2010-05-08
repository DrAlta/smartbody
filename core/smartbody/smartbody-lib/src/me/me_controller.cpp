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
#include <SR/sr_euler.h>
#include <ME/me_prune_policy.hpp>


using namespace std;

#define WARN_ON_INVALID_BUFFER_INDEX (0)
#define TRACE_REMAP_CHANNELS         (0)


//============================= MeController ============================

//#if ME_CONTROLLER_ENABLE_XMLIFY
//const XMLCh* MeController::CONTROLLER_TAG = L"me:controller";
//#endif // ME_CONTROLLER_ENABLE_XMLIFY


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
 	_record_output( NULL ) // for recording poses and motions of immediate local results
{
	_instance_id = instance_count;
	instance_count ++;
	_invocation_count = -1;

	_prune_policy->ref();

	_frames = new std::list<FRAME>;
	_record_mode = RECORD_NULL;
	_record_max_frames = 0;
	_record_frame_count = 0;
	_buffer_changes_toggle = false;
	_buffer_changes_toggle_reset = true;
}

MeController::~MeController () {
	//assert( _context==NULL );  // Controller should not be deleted if still referenced by context
	stop_record();
	
	if(_frames)	delete _frames;

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
	controller_start ();
}

void MeController::stop () {

	_active = false;
	stop_record();
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
	if (this->is_record_buffer_changes())
		this->cal_buffer_changes( frame );

	if( _record_mode ) 
		cont_record( time, frame );

	if( logger )
		logger->controller_post_evaluate( time, *_context, *this, frame );
}

/*
void MeController::record_pose( const char *full_prefix ) { 
	_record_mode = RECORD_POSE; 
	_record_full_prefix = std::string( full_prefix ); 
}
*/

void MeController::record_motion( int max_num_of_frames ) { 

	stop_record();
	_record_mode = RECORD_MOTION; 
	_record_max_frames = max_num_of_frames;
	cout << "MeController::record_motion START"<<endl;
}

void MeController::record_bvh( int max_num_of_frames, double dt )	{

	stop_record();
	_record_mode = RECORD_BVH_MOTION; 
	_record_max_frames = max_num_of_frames;
	_record_dt = dt;
	cout << "MeController::record_bvh START"<<endl;
}

#if ME_CONTROLLER_ENABLE_XMLIFY
DOMElement* MeController::xmlify( DOMDocument* doc ) const {
	DOMElement* elem = doc->createElement( L"me:controller" );
	// TODO: Add name and type name attribute, call xmlify_state and xmlify_children

	// _instance_id
	{	ostringstream oss;
		oss << _instance_id;
		XMLCh* instance_id_xstr = XMLString::transcode( oss.str().c_str() );
		elem->setAttribute( L"instance-id", instance_id_xstr );
		XMLString::release( &instance_id_xstr );
	}

	//
	const char* type_name = controller_type();
	if( type_name ) {
		XMLCh* type_name_xstr = XMLString::transcode( type_name ); 
		elem->setAttribute( L"type", type_name_xstr );
		XMLString::release( &type_name_xstr );
	}

	const char* name_str = name();
	if( name_str ) {
		XMLCh* name_xstr = XMLString::transcode( name_str );
		elem->setAttribute( L"name", name_xstr );
		XMLString::release( &name_xstr );
	}

	return elem;
}
#endif // ME_CONTROLLER_ENABLE_XMLIFY


void MeController::load_bvh_joint_hmap( void )	{
	
/*
	if( _context ) {
		SkChannelArray& ctChannels = controller_channels(); // virtual ct reference
		int nchan = ctChannels.size();
		
		for( int i=0; i<nchan; ++i ) {
		
			SkJointName jname = ctChannels.name( i );
			const char* strname = jname.getstring();
			
//			bool b = _record_joint_hmap.insertstat( strname, 1 );
			bool b = _record_joint_hmap.insert( jname, 1 );
			if( b ) {
				printf( "MeController::load_bvh_joint_hmap SUCCESS: '%s'\n", strname );
			}
			else	{
				printf( "MeController::load_bvh_joint_hmap FAILURE: '%s'\n", strname );
			}
		}
	}
*/
}

void MeController::print_tabs( int depth )	{

	for( int i=0; i<depth; i++ ) { *_record_output << "\t"; }
}

bool MeController::print_bvh_hierarchy( SkJoint* joint_p, int depth )	{
	int i;
	
	if( joint_p == NULL )	{
		cout << "MeController::print_bvh_hierarchy ERR: NULL joint_p" << endl;
		return( false );
	}
	
	print_tabs( depth );
	if( depth == 0 )	{
		*_record_output << "ROOT " << joint_p->name() << "\n";
	}
	else	{
		*_record_output << "JOINT " << joint_p->name() << "\n";
	}

	print_tabs( depth );
	*_record_output << "{\n";
	
	print_tabs( depth + 1 );
	*_record_output << "OFFSET ";

	// STUPID-POLYTRANS ignores ROOT OFFSET: added to CHANNEL motion
	if( depth == 0 )	{
		*_record_output << "0.0 0.0 0.0 \n";
	}
	else	{
		SrVec offset_v = joint_p->offset();

	// STUPID-POLYTRANS subtracts OFFSET instead of adds
#define STUPID_POLYTRANS_FLIP_OFFSET 0
#if STUPID_POLYTRANS_FLIP_OFFSET
		*_record_output << -offset_v.x << " ";
		*_record_output << -offset_v.y << " ";
		*_record_output << -offset_v.z << " ";
#else
		*_record_output << offset_v.x << " ";
		*_record_output << offset_v.y << " ";
		*_record_output << offset_v.z << " ";
#endif
		*_record_output << "\n";
	}
	
	// CHANNELS: 
	// Optimize: check 
	//   SkJointQuat::_active, and 
	//   SkJointPos:SkVecLimits::frozen()
	print_tabs( depth + 1 );
	*_record_output << "CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n";
	
	int num_child = joint_p->num_children();
	if( num_child == 0 )	{
	
		print_tabs( depth + 1 );
		*_record_output << "End Site\n";

		print_tabs( depth + 1 );
		*_record_output << "{\n";
		
		// End Site OFFSET not used
		// This is the geometric vector of the final bone segment
		print_tabs( depth + 2 );
		*_record_output << "OFFSET 0.0 0.0 0.0\n";
		
		print_tabs( depth + 1 );
		*_record_output << "}\n";
	}
	else
	for( i = 0; i < num_child; i++ )	{
		SkJoint* child_p = joint_p->child( i );
		print_bvh_hierarchy( child_p, depth + 1 );
	}

	print_tabs( depth );
	*_record_output << "}\n";
	
	return( true );
}

bool MeController::print_bvh_motion( SkJoint* joint_p, int depth, FRAME& frame_data )	{ 
	// NOTE: depth only used to hack STUPID-POLYTRANS ROOT bug
	int i;

	if( joint_p == NULL )	{
		cout << "MeController::print_bvh_motion ERR: NULL joint_p" << endl;
		return( false );
	}
	
	SkJointPos* sk_jp_p = joint_p->pos();
	
//*_record_output << " " << joint_p->name() << " { ";
	
	// STUPID-POLYTRANS ignores ROOT OFFSET: add to CHANNEL motion
	std::ostringstream * frame_data_os = new std::ostringstream;
	if( depth == 0 )	{
		SrVec offset_v = joint_p->offset();
		*frame_data_os << " " << sk_jp_p->value( 0 ) + offset_v.x;
		*frame_data_os << " " << sk_jp_p->value( 1 ) + offset_v.y;
		*frame_data_os << " " << sk_jp_p->value( 2 ) + offset_v.z;
		frame_data += frame_data_os->str();
	}
	else	{
		*frame_data_os << " " << sk_jp_p->value( 0 );
		*frame_data_os << " " << sk_jp_p->value( 1 );
		*frame_data_os << " " << sk_jp_p->value( 2 );
		frame_data += frame_data_os->str();
	}
	delete frame_data_os;
	
//	SkJointQuat* sk_jq_p = joint_p->quat();
//	SrQuat sr_q = sk_jq_p->value();

	SrMat sr_m = joint_p->lmat();
	float ex, ey, ez;
	sr_euler_angles_yxz( sr_m, ex, ey, ez ); // see sk_joint_euler.h for order interpretation...
//	sr_euler_angles_zxy( sr_m, ex, ey, ez );

//*_record_output << " ," << joint_p->name();

//inline double RAD( double d ) { return( d * 0.017453292519943295 ); }
//inline double DEG( double r ) { return( r * 57.295779513082323 ); }
	std::ostringstream * frame_data_os1 = new std::ostringstream;
	*frame_data_os1 << " " << ez * 57.295779513082323;
	*frame_data_os1 << " " << ex * 57.295779513082323;
	*frame_data_os1 << " " << ey * 57.295779513082323;
	frame_data += frame_data_os1->str();
	delete frame_data_os1;

//*_record_output << " }";


	int num_child = joint_p->num_children();
	for( i = 0; i < num_child; i++ )	{
		SkJoint* child_p = joint_p->child( i );
		print_bvh_motion( child_p, depth + 1, frame_data );
	}

	return( true );
}

void MeController::record_stop()	{
	stop_record();
	cout << "MeController::record_stop"<<endl;
}

void MeController::record_clear()	{
	_frames->clear();
}

void MeController::record_write( const char *full_prefix ) {
	
	_record_full_prefix = std::string(full_prefix);
	string filename;
	ostringstream record_id_oss;

	_invocation_count++;
	if( _name == NULL )	{
		name( "noname" );
	}
	record_id_oss << _instance_id << "." << _invocation_count << "_R";
	string recordname = string( controller_type() ) + "_" + string( _name ) + "_" + record_id_oss.str();
	
	if( _record_mode == RECORD_BVH_MOTION )	{
		filename = _record_full_prefix + recordname + ".bvh";
		_record_output = new SrOutput( filename.c_str(), "w" );

		SkSkeleton* skeleton_p = NULL;
		if( _context->channels().size() > 0 )	{
			skeleton_p = _context->channels().skeleton();
		}
		if( skeleton_p == NULL )	{
			cout << "MeController::record_write NOTICE: SkSkeleton not available" << endl;
			_record_mode = RECORD_NULL;
		}
		
		*_record_output << "HIERARCHY\n";
		print_bvh_hierarchy( skeleton_p->root(), 0 );
		*_record_output << "MOTION\n";
		*_record_output << "Frames: " << _frames->size() << srnl;	
		*_record_output << "Frame Time: " << _record_dt << srnl;	
//		load_bvh_joint_hmap();
		cout << "MeController::write_record BVH: " << filename << endl;
	}
	else
	if( _record_mode == RECORD_MOTION )	{
		filename = _record_full_prefix + recordname + ".skm";
		_record_output = new SrOutput( filename.c_str(), "w" );

		*_record_output << "# SKM Motion Definition - M. Kallmann 2004\n";
		*_record_output << "# Maya exporter v0.6\n";
		*_record_output << "# Recorded output from MeController\n\n";
		*_record_output << "SkMotion\n\n";
		*_record_output << "name \"" << recordname.c_str() << "\"\n\n";

		SkChannelArray& channels = controller_channels();
		*_record_output << channels << srnl;
		*_record_output << "frames " << _frames->size() << srnl;	

		cout << "MeController::write_record SKM: " << filename << endl;
	}
	else	{
		cout << "MeController::init_record NOTICE: POSE not implemented" << endl;
		_record_mode = RECORD_NULL;
		_frames->clear();
		//filename = _record_full_prefix + recordname + ".skp";
	}
	std::list<FRAME>::iterator iter = _frames->begin();
	std::list<FRAME>::iterator end  = _frames->end();
	for(;iter!=end; iter++)
		*_record_output<<(*iter).c_str()<<srnl;
	record_clear();
	if( _record_output )	{
		delete _record_output;
		_record_output = NULL;
	}
}

void MeController::cont_record( double time, MeFrameData& frame )	{
	
	if( time < 0.0001 ) time = 0.0;

	if( _record_mode == RECORD_BVH_MOTION )	{

		SkSkeleton* skeleton_p = NULL;
		if( _context->channels().size() > 0 )	{
			skeleton_p = _context->channels().skeleton();
		}
		if( skeleton_p == NULL )	{
			cout << "MeController::cont_record NOTICE: SkSkeleton not available" << endl;
			_record_mode = RECORD_NULL;
			return;
		}
		FRAME frame_data;
		frame_data.clear();
		// NOTE: depth only used to hack STUPID-POLYTRANS ROOT bug
		print_bvh_motion( skeleton_p->root(), 0, frame_data );		
		if((_record_frame_count>=_record_max_frames)&&(_record_max_frames!=0))
			_frames->pop_front();
		_frames->push_back(frame_data);
	}
	else
	if( _record_mode == RECORD_MOTION )	{
		FRAME frame_data;
		frame_data.clear();
		ostringstream frame_data_os;
		frame_data_os << "kt " << time << " fr ";

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
				frame_data_os << axis.x << " ";
				frame_data_os << axis.y << " ";
				frame_data_os << axis.z << " ";
			}
			else
			for( j=0; j<channel_size; j++ )	{
				frame_data_os << buff[ index + j ] << " ";
			}
		}
		frame_data += frame_data_os.str();
		if((_record_frame_count>=_record_max_frames)&&(_record_max_frames!=0))
			_frames->pop_front();
		_frames->push_back(frame_data);
	}
	_record_frame_count++;
}

void MeController::stop_record( void )	{
	_record_mode = RECORD_NULL;
	_record_max_frames = 0;
	_record_frame_count = 0;
	if( _record_output )	{
		delete _record_output;
		_record_output = NULL;
	}
	_frames->clear();
//	cout << "MeController::stop_record" << endl;
}

void MeController::record_buffer_changes(bool val)
{
	_buffer_changes_toggle = val;
	_buffer_changes_toggle_reset = true;
}

void MeController::cal_buffer_changes( MeFrameData& frame)
{
	if (_buffer_changes_toggle_reset)
	{
			// reset the buffer changes
		_buffer_changes = frame.buffer();
		for (int i = 0; i < _buffer_changes.size(); i++)
			_buffer_changes[i] = 0;
		_buffer_changes_toggle_reset = false;
	}

	SrBuffer<float>& buff = frame.buffer();
	SkChannelArray& channelsInUse = this->controller_channels();
	int channels_size = channelsInUse.size();
	for( int i = 0 ; i < channels_size; i++ )
	{
		SkChannel& channel = channelsInUse.get(i);
		int channelSize = channel.size();
		int channelIndex = _toContextCh[ i ];
		int bufferIndex = frame.toBufferIndex(channelIndex);
		if (1)// frame.isChannelUpdated(channelIndex) )
		{
			for (int j = 0; j < channelSize; j++)
			{
				float val = frame.buffer()[bufferIndex + j];
				_buffer_changes[bufferIndex + j] = val;
			}
		}
	}

}


SrBuffer<float>& MeController::get_buffer_changes()
{
	return _buffer_changes;
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

#if ME_CONTROLLER_ENABLE_XMLIFY
void MeController::xmlify_state( DOMElement* elem ) const {
	// Do nothing by default
}

void MeController::xmlify_children( DOMElement* elem ) const {
	// TODO: append xmlify(..) result from each child
}
#endif // ME_CONTROLLER_ENABLE_XMLIFY


void MeController::print_state( int tab_count ) {
	using namespace std;

	const char* name = this->name();

	cout << controller_type();
	if( name!=NULL && name[0]!='\0' )
		cout << " \"" << name << "\"";
	cout << " @0x" << this << endl;

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
