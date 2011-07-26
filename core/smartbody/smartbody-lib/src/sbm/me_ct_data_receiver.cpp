#include "me_ct_data_receiver.h"
#include <sr/sr_euler.h>
#include <sbm/mcontrol_util.h>

const char* MeCtDataReceiver::CONTROLLER_TYPE = "DataReceiver";

MeCtDataReceiver::MeCtDataReceiver(SkSkeleton* skel) : MeController()
{
	_skeleton = skel;
	skel->ref();

	_valid = true;
	_prevTime = 0.0;
	_duration = 0.0;
}

MeCtDataReceiver::~MeCtDataReceiver()
{
	_skeleton->unref();
	_posMap.clear();
	_startingPos.clear();
	_quatMap.clear();
}

void MeCtDataReceiver::setGlobalPosition(std::string jName, SrVec& pos)
{
	std::map<std::string, SrVec>::iterator iter = _posMap.find(jName);
	if (iter == _posMap.end())
	{
		_posMap.insert(std::make_pair(jName, pos));
		_startingPos.insert(std::make_pair(jName, pos));
	}
	else
		iter->second = pos;
}

void MeCtDataReceiver::setLocalRotation(std::string jName, SrQuat& q)
{
	std::map<std::string, SrQuat>::iterator iter = _quatMap.find(jName);
	if (iter == _quatMap.end())
		_quatMap.insert(std::make_pair(jName, q));
	else
		iter->second = q;
}

bool MeCtDataReceiver::controller_evaluate(double t, MeFrameData& frame)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (_prevTime == 0)
		_dt = 0.016;
	else
	{
		_dt = t - _prevTime;
		_prevTime = t;
	}
	if (_valid && _context)
	{
		// set local rotation directly
		std::map<std::string, SrQuat>::iterator iter;
		for (iter = _quatMap.begin(); iter != _quatMap.end(); iter++)
		{
			std::string jName = iter->first;
			SkJoint* joint = _skeleton->search_joint(jName.c_str());
			if (!joint)	continue;
			int channelId = _context->channels().search(SkJointName(jName.c_str()), SkChannel::Quat);
			if (channelId < 0)	continue;
			int bufferId = frame.toBufferIndex(channelId);
			if (bufferId < 0)	continue;
			SrQuat quat = iter->second;
			frame.buffer()[bufferId + 0] = quat.w;
			frame.buffer()[bufferId + 1] = quat.x;
			frame.buffer()[bufferId + 2] = quat.y;
			frame.buffer()[bufferId + 3] = quat.z;
		}

		// get difference of global position and add up to frame buffer
		std::map<std::string, SrVec>::iterator iter1;
		for (iter1 = _posMap.begin(); iter1 != _posMap.end(); iter1++)
		{
			std::string jName = iter1->first;
			SkJoint* joint = _skeleton->search_joint(jName.c_str());
			if (!joint)	continue;
			int channelId = _context->channels().search(SkJointName(jName.c_str()), SkChannel::XPos);
			if (channelId < 0)	continue;
			int bufferId = frame.toBufferIndex(channelId);
			if (bufferId < 0)	continue;
			SrVec curr = iter1->second;
			SrVec start = _startingPos[jName];
			SrVec diff = curr - start;

			frame.buffer()[bufferId + 0] = -diff.x;
			frame.buffer()[bufferId + 1] = diff.y;
			frame.buffer()[bufferId + 2] = -diff.z;
		}
	}
	return true;
}
