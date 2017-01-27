/*************************************************************
Copyright(C) 2017 University of Southern California

This file is part of Smartbody.

Smartbody is free software : you can redistribute it and / or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Smartbody is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Smartbody.If not, see <http://www.gnu.org/licenses/>.

**************************************************************/

#include "vhcl_log.h"

#include <controllers/me_ct_motion.h>
#include <sb/SBEvent.h>
#include <sb/SBScene.h>
#include <sb/sbm_pawn.hpp>
#include <sb/SBScene.h>
#include <sb/SBRetargetManager.h>
#include <sb/SBRetarget.h>
#include <sb/SBMotion.h>
#include <sb/SBSkeleton.h>
#include "controllers/me_ct_generic_hand.h"

//=================================== MeCtMotion =====================================

std::string MeCtMotion::type_name = "Motion";

MeCtMotion::MeCtMotion ()
 {
   _motion = 0;
   _play_mode = SkMotion::Linear;
//   _duration = 0;
   _twarp = _maxtwarp = _mintwarp = 1.0f;
   _loop = false;
   _last_apply_frame = 0;
   _offset = 0;
   _isAdditive = false;
   _useOffset = false;
   _holdTime = 0.0;
   _holdDuration = 0.0;
   _prestrokeHoldTime = 0.0;
   _prestrokeHoldDuration = 0.0;
   _isGesture = true;
 }

MeCtMotion::~MeCtMotion ()
 {
 }

void MeCtMotion::init(SbmPawn* pawn, SkMotion* m_p, double time_offset, double time_scale)	{

	if ( _motion ) {
		if( m_p == _motion ) {
			// Minimal init()
			_last_apply_frame = 0;
			MeController::init (pawn);
			return;
		}
		// else new motion
	}

	_motion = m_p;
	_last_apply_frame = 0;

	_character = dynamic_cast<SmartBody::SBCharacter*>(pawn);

	_motion->move_keytimes ( 0 ); // make sure motion starts at 0
//	_duration = _motion->duration() / _twarp;

	MeController::init (pawn);
	//LOG("before if (_context)");
	if( _context ) {
		// Notify _context of channel change.
		_context->child_channels_updated( this );
	}
	//LOG("after if (_context)");

	synch_points.copy_points( m_p->synch_points, time_offset, time_scale );
#if 1
	double indt = synch_points.get_interval_to( srSynchPoints::READY );
	if( indt < 0.0 ) indt = 0.0;
	double outdt = synch_points.get_interval_from( srSynchPoints::RELAX );
	if( outdt < 0.0 ) outdt = 0.0;
	inoutdt( (float)indt, (float)outdt );
	double emph = synch_points.get_time( srSynchPoints::STROKE );
	emphasist( (float)emph );
	twarp( 1.0f / (float)time_scale );
	_offset = time_offset;
#endif

	motionTime = synch_points.get_time( srSynchPoints::START);
	controlRestart();
	_lastCycle = -1;
	loadMotionEvents();

	// intialize hand controller 
	//_character->generic_hand_ct->init(_motion);
}

void MeCtMotion::init (SbmPawn* pawn, SkMotion* m_p ) {
	init(pawn, m_p, 0.0, 1.0 );
}

void MeCtMotion::init(SbmPawn* pawn, SkMotion* m_p, std::vector<std::string>& joints)
{
	_joints = joints;
	init(pawn, m_p, 0.0, 1.0);
}

void MeCtMotion::offset ( double amount )
{
	_offset = amount;
}

void MeCtMotion::twarp ( float tw ) {
	// put in/out back to their original values:
	float in = indt()*_twarp;
	float out = outdt()*_twarp;
	float emph = emphasist()*_twarp;

	// check limits:
//	if ( tw<_mintwarp ) tw = _mintwarp;
//	if ( tw>_maxtwarp ) tw = _maxtwarp;
	_twarp = tw;

	// update duration:
//	_duration = _motion->duration() / _twarp;

	// update in/out:
	inoutdt ( in/_twarp, out/_twarp );

	// update emphasist:
	if ( emph>=0 )
		emphasist ( emph/_twarp );
	else
		emphasist ( -1.0f ); // set -1 again to ensure that all <0 values are -1
}

double MeCtMotion::phase_duration ()
{ 
	return synch_points.get_duration(); 
}

void MeCtMotion::output ( SrOutput& out )
 {
   MeController::output ( out );

   // name
   if ( getName() != _motion->getName())
    { 
		SrString n;
		n.make_valid_string ( _motion->getName().c_str() );
      out << "motion " << n << srnl;
    }

   // play mode
   if ( _play_mode!=SkMotion::Linear )
    { out << "mode " << SkMotion::interp_type_name(_play_mode) << srnl;
    }

   // time warp
   if ( _twarp!=1.0f || _mintwarp!=1.0f || _maxtwarp!=1.0f )
    { out << "twarp " << _twarp;
      out << " minmax " << _mintwarp << srspc << _maxtwarp << srnl;
    }

   // loop
   if ( _loop==true )
    { out << "loop true\n";
    }

   // end
   out << "end\n";
 }

bool MeCtMotion::input ( SrInput& inp, const SrHashTable<SkMotion*>& motions ) {
   MeController::input ( inp );

   // init with defaults:
   SrString mname ( getName().c_str() );
   _play_mode = SkMotion::Linear;
   _twarp = _mintwarp = _maxtwarp = 1.0f;
   _loop = false;

   // read:
   while ( !inp.finished() )
    { inp.get_token();
      SrString& s = inp.last_token();
      if ( s=="motion" )
       { inp.get_token();
         mname = inp.last_token();
       }
      else if ( s=="mode" )
       { inp.get_token();
         _play_mode = SkMotion::interp_type_name(inp.last_token());
       }
      else if ( s=="twarp" )
       { inp >> _twarp;
         inp.get_token();
         inp >> _mintwarp >> _maxtwarp;
       }
      else if ( s=="loop" )
       { inp.get_token();
         _loop = inp.last_token()=="true"? true:false;
       }
      else if ( s=="end" )
       { break;
       }
    }
   
	SkMotion* m = motions.lookup ( mname );
	if ( m ) {
		init ( NULL, m );
		return true;
	} else {
		return false;
	}
}

//----- virtuals -----
void MeCtMotion::controller_map_updated() {
	// Map motion channel index to context float buffer index
	SkChannelArray& mChannels = _motion->channels();
	int size = mChannels.size();
	_mChan_to_buff.size( size );

	if( _context ) {
		SkChannelArray& cChannels = _context->channels();

#if 0	// Inspect for debugger
		int world_offset_ch_x = cChannels.search( SkJointName("world_offset"), SkChannel::XPos );
		int world_offset_ch_y = cChannels.search( SkJointName("world_offset"), SkChannel::YPos );
		int world_offset_ch_z = cChannels.search( SkJointName("world_offset"), SkChannel::ZPos );
		int world_offset_ch_q = cChannels.search( SkJointName("world_offset"), SkChannel::Quat );
//		LOG( "world_offset_ch_x: %d\n", world_offset_ch_x );
#endif
		if (_joints.empty())
		{
			for( int i=0; i<size; ++i ) 
			{
				int chanIndex = cChannels.search( mChannels.mappedName(i), mChannels.type(i) );
				_mChan_to_buff[ i ] = _context->toBufferIndex( chanIndex );
			}
		}
		else
		{
			for (size_t i = 0; i < _joints.size(); ++i)
			{
				int motionChannelId = mChannels.search(_joints[i], SkChannel::Quat);
				if (motionChannelId >= 0)
				{
					int chanIndex = cChannels.search(mChannels.name(motionChannelId), mChannels.type(motionChannelId));
					if (chanIndex >= 0)
						_mChan_to_buff[motionChannelId] = _context->toBufferIndex(chanIndex);
				}
			}
		}

	} else {
		_mChan_to_buff.setall( -1 );
	}
}


void MeCtMotion::controller_start ()
{

}

void MeCtMotion::controller_stop ()
{

	updateDt((float)stop_time());
	motionTime += dt;	
	checkMotionEvents(motionTime);
}

bool MeCtMotion::controller_evaluate ( double t, MeFrameData& frame ) {

	bool continuing = true;
//	double dur = _duration;
	double dur = phase_duration();
//	if( dur < 0.0 )	{
//		LOG( "no-dur: %s", name() );
//	}

#if 1
	updateDt((float)t);
	//motionTime += dt;
	motionTime = t;
#else
	motionTime = t;
#endif
	double curMotionTime = motionTime;
	if ( _loop ) {
		double x = motionTime/dur;
		int cycleNum = int(x);
		//LOG("x = %f, cycle Num = %d",x , cycleNum);
		if ( x > 1.0 )
		{
			curMotionTime = dur *( x - cycleNum );
			if (cycleNum != _lastCycle)
			{
				loadMotionEvents(); // reload any motion events during loop
				_lastCycle = cycleNum;
			}
		}
	} else {
		//LOG("MeCtMotion::controller_evaluate %s time %f, duration %f", this->motion()->getName().c_str(), t, dur);
		//LOG("%s time %f, motion time %f", this->motion()->getName().c_str(), t, curMotionTime);
		continuing = motionTime <= dur;
	}	
	SmartBody::SBRetarget* retarget = NULL;
	SmartBody::SBMotion* sbMotion = dynamic_cast<SmartBody::SBMotion*>(_motion);
	if (_character)
	{
		SmartBody::SBScene* scene = SmartBody::SBScene::getScene();		
		if (sbMotion)
			retarget = scene->getRetargetManager()->getRetarget(sbMotion->getMotionSkeletonName(),_character->getSkeleton()->getName());	
		if (retarget)
		{
			SrVec leftSholderRot = _character->getVec3Attribute("leftShoulderOffset");
			SrVec rightSholderRot = _character->getVec3Attribute("rightShoulderOffset");
			SrQuat lShlderQuat(leftSholderRot);
			SrQuat rShlderQuat(rightSholderRot);
			retarget->addJointRotOffset("l_shoulder",lShlderQuat);
			retarget->addJointRotOffset("r_shoulder",rShlderQuat);
		}
	}

	// whether use offset
	if (_useOffset)
	{
		if (!sbMotion->getOffsetParent())
			_motion = sbMotion->getOffset();
	}
	else
	{
		if (sbMotion->getOffsetParent())
			_motion = sbMotion->getOffsetParent();
	}

	// Controller Context and FrameData set, use the new available buffer
	//_motion->apply( float(t)*_twarp + float(_offset),
	//	            &(frame.buffer()[0]),  // pointer to buffer's float array
	//				&_mChan_to_buff,
	//	            _play_mode, &_last_apply_frame );	
	//LOG("dt = %f, motionTime = %f, curMotionTime = %f, dur = %f, continue = %d",dt, motionTime, curMotionTime, dur, continuing);
	float motionTime = float(curMotionTime + _offset);
	// check for prestroke or poststroke hold motion
	if (_prestrokeHoldDuration > .001)
	{
		double prestrokeHoldPeriodStart = _prestrokeHoldTime;
		double prestrokeHoldPeriodEnd = _prestrokeHoldTime + _prestrokeHoldDuration;

		if (motionTime > prestrokeHoldPeriodStart &&
			motionTime <= prestrokeHoldPeriodEnd)
		{
			// todo...
		}
	}



	if (_holdDuration > 0.001)
	{
		double holdPeriodStart = _holdTime;
		double holdPeriodEnd = _holdTime + _holdDuration;
		 
		if (motionTime > holdPeriodStart &&
			motionTime <= holdPeriodEnd)
		{
			motionTime = _holdTime;
		}
		else if (motionTime > holdPeriodEnd)
		{
			motionTime = motionTime - _holdDuration;
		}
	}

	if (_prestrokeHoldDuration > .001)
	{
		double prestrokeHoldPeriodStart = _prestrokeHoldTime;
		double prestrokeHoldPeriodEnd = _prestrokeHoldTime + _prestrokeHoldDuration;

		if (motionTime > prestrokeHoldPeriodStart &&
			motionTime <= prestrokeHoldPeriodEnd)
		{
			motionTime = _prestrokeHoldTime;
		}
	}

	_motion->apply( motionTime,
		            &(frame.buffer()[0]),  // pointer to buffer's float array
					&_mChan_to_buff,
		            _play_mode, &_last_apply_frame, _isAdditive, retarget );

#if 1 // disable IK post-processing for posture/motion by default
	if (retarget && sbMotion)
	{
		bool hasCharacterTraj = false;
		std::vector<std::string> jointConsNames = _character->getJointConstraintNames();
		SmartBody::SBJoint* baseJoint = _character->getSkeleton()->getJointByMappedName("base");
		SrMat baseGmat;
		if (baseJoint)
			baseGmat = baseJoint->gmat();		
		for (unsigned int i=0;i<jointConsNames.size();i++)
		{
			SmartBody::TrajectoryRecord* trajRecord = _character->getJointTrajectoryConstraint(jointConsNames[i]);
			if (!trajRecord)
				continue;
			
			SmartBody::SBJoint* refJoint = _character->getSkeleton()->getJointByName(trajRecord->refJointName);
			if (!refJoint)
				continue;			

			SrVec trajOffset;
			bool hasTraj = sbMotion->getTrajPosition(jointConsNames[i],motionTime,trajOffset);
			if (!hasTraj)
			{	
				trajRecord->isEnable = false;
				continue;
			}
			hasCharacterTraj = true;
			trajRecord->isEnable = true;
			trajRecord->jointTrajLocalOffset = trajOffset;
			trajRecord->refJointGlobalPos = refJoint->gmat().get_translation(); 
			retarget->applyRetargetJointTrajectory(*trajRecord,baseGmat);			
		}
		if (hasCharacterTraj)
		{
			_character->setJointTrajBlendWeight(1.0f);
			_character->setUseJointConstraint(true);
		}
		else
		{
			_character->setJointTrajBlendWeight(0.f);
			_character->setUseJointConstraint(false);
		}
	}
#endif

	SkChannelArray& allChannels = _motion->channels();
	for (int i = 0; i < allChannels.size(); ++i)
	{		
		frame.channelUpdated(i);
	}


	checkMotionEvents(curMotionTime);

	return continuing;
}

SkChannelArray& MeCtMotion::controller_channels ()
 {
   return _motion->channels();
 }

double MeCtMotion::controller_duration ()
 {
//   return _loop? -1.0:_duration;
   return _loop? -1.0:phase_duration();
 }

const std::string& MeCtMotion::controller_type () const
{
	return type_name;
}

SrBuffer<int>& MeCtMotion::get_context_map()
{
	return _mChan_to_buff;
}

void MeCtMotion::loadMotionEvents()
{
	// add the event instances to this controller
	while (!_events.empty())
		_events.pop();

	if (_motion)
	{
		std::vector<SmartBody::SBMotionEvent*>& motionEvents = _motion->getMotionEvents();
		for (size_t x = 0; x < motionEvents.size(); x++)
		{
			_events.push(motionEvents[x]);
		}
	}
}

void MeCtMotion::checkMotionEvents(double time)
{
	while (!_events.empty())
	{
		SmartBody::SBMotionEvent* motionEvent = _events.front();		
		if (motionEvent->isEnabled() && time >= motionEvent->getTime())
		{
			SmartBody::SBEventManager* manager = SmartBody::SBScene::getScene()->getEventManager();
			manager->handleEvent(motionEvent);
			std::string type = motionEvent->getType();
			std::string params = motionEvent->getParameters();
			//LOG("EVENT: %f %s %s", time, type.c_str(), params.c_str());
			_events.pop();
		}
		else
		{
			return;
		}
	}
}

void MeCtMotion::setHoldTime(double time)
{
	_holdTime = time;
}

double MeCtMotion::getHoldTime()
{
	return _holdTime;
}

void MeCtMotion::setHoldDuration(double time)
{
	_holdDuration = time;
}

double MeCtMotion::getHoldDuration()
{
	return _holdDuration;
}

void MeCtMotion::setGesture(bool val)
{
	_isGesture = val;
}

bool MeCtMotion::isGesture()
{
	return _isGesture;
}

void MeCtMotion::setPrestrokeHoldTime(double time)
{
	_prestrokeHoldTime = time;
}

double MeCtMotion::getPrestrokeHoldTime()
{
	return _prestrokeHoldTime;
}

void MeCtMotion::setPrestrokeHoldDuration(double time)
{
	_prestrokeHoldDuration = time;
}

double MeCtMotion::getPrestrokeHoldDuration()
{
	return _prestrokeHoldDuration;
}


//======================================= EOF =====================================

