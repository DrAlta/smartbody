/**
* \file me_ct_breathing.cpp
* \brief The skeleton animation breathing controller
*
* Part of Motion Engine and SmartBody-lib.
* Copyright (C) 2008  University of Southern California
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
*   - Celso M. de Melo, demelo@usc.edu, USC
*
* \author University of Southern California
* \author Institute for Creative Technologies, USC
*/

# include <sbm/me_ct_breathing.h>
# include <sbm/gwiz_math.h>
# include <sr/sr_output.h>
#include <cstdio>

const char* MeCtBreathing::type_name = "Breathing";

MeCtBreathing::MeCtBreathing ()
{
	_motion = 0;
	_last_apply_frame = 0;

	_local_time_offset = 0;

	_default_breath_cycle = new LinearBreathCycle();
	_breath_layers.push_front(new BreathLayer(_default_breath_cycle, -1));

	_pending_breath_layer = NULL;
	_pending_pop = false;

	_previous_breath_is_inspiring = current_breath_layer()->cycle->is_inspiring();
	
	_pending_motion = NULL;
	_bpm = 15;
	_pending_bpm = -1;
	_expiratory_reserve_volume_threshold = 0;

	_incremental = false;
}

MeCtBreathing::~MeCtBreathing ()
{
	if ( _motion ) _motion->unref ();
	delete _default_breath_cycle;
}

void MeCtBreathing::init ( SbmPawn* pawn )
{
	MeController::init(pawn);
}

void MeCtBreathing::setMotion ( SkMotion* m ) 
{
	immediate_motion(m);

	_default_breath_cycle->min(_expiratory_reserve_volume_threshold);
	_default_breath_cycle->max(1.0f);
}

SkMotion* MeCtBreathing::getMotion () 
{
	return _motion;
}

void MeCtBreathing::push_breath_layer(BreathCycle* cycle, int cyclesRemaining, bool smoothTransition)
{
	if(smoothTransition)
		_pending_breath_layer = new BreathLayer(cycle, cyclesRemaining);
	else
		immediate_push_breath_layer(new BreathLayer(cycle, cyclesRemaining));
}
void MeCtBreathing::pop_breath_layer()
{
	_pending_pop = true;
}
void MeCtBreathing::clear_breath_layers()
{
	_breath_layers.clear();
}
BreathLayer* MeCtBreathing::current_breath_layer()
{
	if(_breath_layers.empty())
		return NULL;
	else
		return _breath_layers.front();
}
void MeCtBreathing::breaths_per_minute(float bpm, bool smooth_transition) 
{ 
	if(smooth_transition)
		_pending_bpm = bpm; 
	else
		immediate_breaths_per_minute(bpm);
}
void MeCtBreathing::print_state( int tabCount ) 
{
	fprintf( stdout, "MeCtBreathing" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

	fprintf( stdout, ", motion" );
	if( _motion ) 
	{
		// motion name
		str = _motion->name();
		if( str )
			fprintf( stdout, "=\"%s\"", str );

		// motion filename
		str = _motion->filename();
		if( str )
			fprintf( stdout, " file=\"%s\"", str );
	} 
	else
		fprintf( stdout, "=NULL" );
	// bpm
	fprintf( stdout, ", bpm=%d", _bpm );
	fprintf( stdout, ", topBreathCycle" );
	if( current_breath_layer() != NULL ) 
	{
		// top layer's breath cycle name
		fprintf( stdout, " name=\"%s\"", current_breath_layer()->cycle->type() );
		// top layer's cycles remaining
		fprintf( stdout, " cyclesRemaining=%d", current_breath_layer()->cycles_remaining );
	}
	else
		fprintf( stdout, "=NULL" );

	fprintf( stdout, "\n" );
}

void MeCtBreathing::immediate_push_breath_layer(BreathLayer* layer)
{
	_breath_layers.push_front(layer);
}
void MeCtBreathing::immediate_pop_breath_layer()
{
	if(current_breath_layer() == NULL)
		return;

	BreathLayer* layer = current_breath_layer();
	if(layer->cycle != _default_breath_cycle)
		delete layer->cycle;
	delete layer;
	_breath_layers.pop_front();
}
void MeCtBreathing::immediate_breaths_per_minute(float bpm)
{
	_bpm = bpm;
	_pending_bpm = -1;
}
void MeCtBreathing::immediate_motion(SkMotion* motion)
{
	if ( _motion ) {
		if( motion == _motion ) {
			// Minimal init()
			_last_apply_frame = 0;
			MeController::init (NULL);
			return;
		}
		// else new motion
		_motion->unref();
	}

	_motion = motion;
	_last_apply_frame = 0;

	_motion->ref();
	_motion->move_keytimes ( 0 ); // make sure motion starts at 0

	MeController::init (NULL);

	if( _context ) {
		// Notify _context of channel change.
		_context->child_channels_updated( this );
	}

	_expiratory_reserve_volume_threshold = (float) _motion->time_ready() / (float) _motion->time_stroke_emphasis();

	// ?????
	//_motion->set_incremental_frames(_motion->time_ready());
}
void MeCtBreathing::controller_map_updated() 
{
	// Map motion channel index to context float buffer index
	if (!_motion)
		return;
	SkChannelArray& mChannels = _motion->channels();
	const int size = mChannels.size();

	_mChan_to_buff.size( size );

	if( _context ) {
		SkChannelArray& cChannels = _context->channels();

		for( int i=0; i<size; ++i ) {
			int chanIndex = cChannels.search( mChannels.name(i), mChannels.type(i) );
			_mChan_to_buff[ i ] = _context->toBufferIndex( chanIndex );
		}
	} else {
		_mChan_to_buff.setall( -1 );
	}
}

void MeCtBreathing::motion( SkMotion* motion )
{
	if (!_motion) 
		immediate_motion(motion);
	else
		_pending_motion = motion;
}

bool MeCtBreathing::controller_evaluate ( double t, MeFrameData& frame ) 
{
	if (!_motion)
		return true;

	float frameTime;
	bool isFrameTimeSet = false;
	while(!isFrameTimeSet)
	{
		if(current_breath_layer() == NULL)
			return true; //No breathing, but the controller never finishes

		float desiredBreathsPerSecond = _bpm / 60.0f;
		float currentBreathsPerSecond = 1.0f / current_breath_layer()->cycle->duration();
		float speed = desiredBreathsPerSecond / currentBreathsPerSecond;

		double localTime = t - _local_time_offset;

		double cycleTime = fmod(localTime*speed, double(current_breath_layer()->cycle->duration()));
		frameTime = (float) _motion->time_stroke_emphasis() * current_breath_layer()->cycle->value(localTime, cycleTime);
		
		isFrameTimeSet = true;
		if(!_previous_breath_is_inspiring && current_breath_layer()->cycle->is_inspiring()) //New cycle is starting
		{
			if(_pending_bpm != -1)
			{
				_bpm = _pending_bpm;
				_pending_bpm = -1;
				_local_time_offset = t;
				isFrameTimeSet = false;
			}
			if(_pending_motion != NULL)
			{
				immediate_motion(_pending_motion);
				_pending_motion = NULL;
				isFrameTimeSet = false;
			}
			
			if(current_breath_layer()->cycles_remaining != -1)
			{
				current_breath_layer()->cycles_remaining--;
				if(current_breath_layer()->cycles_remaining == 0)
					pop_breath_layer();
				isFrameTimeSet = false;
			}
			if(_pending_pop)
			{
				immediate_pop_breath_layer();
				_pending_pop = false;
				_local_time_offset = t;
				isFrameTimeSet = false;
			}
			if(_pending_breath_layer != NULL)
			{
				immediate_push_breath_layer(_pending_breath_layer);
				_local_time_offset = t;
				_pending_breath_layer = NULL;
				isFrameTimeSet = false;
			}

			if(current_breath_layer() != NULL)
				_previous_breath_is_inspiring = current_breath_layer()->cycle->is_inspiring();
		}
	}
	
	_previous_breath_is_inspiring = current_breath_layer()->cycle->is_inspiring();
	if(_incremental)
	{
		// ??????
		//_motion->apply_incremental( 
		//	frameTime, &(frame.buffer()[0]), &_mChan_to_buff, SkMotion::Linear, &_last_apply_frame );
	}
	else
	{
		_motion->apply( 
			frameTime, &(frame.buffer()[0]), &_mChan_to_buff, SkMotion::Linear, &_last_apply_frame );
	}

	return true;
}

SkChannelArray& MeCtBreathing::controller_channels ()
{
	if (_motion)
		return _motion->channels();
	else
		return _channels;
}

const char* MeCtBreathing::controller_type () const
{
	return type_name;
}
