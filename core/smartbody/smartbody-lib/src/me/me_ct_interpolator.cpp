#include <ME/me_ct_interpolator.h>
#include <sbm/mcontrol_util.h>

const char* MeCtInterpolator::Context::CONTEXT_TYPE = "MeCtInterpolator::Context";
const char* MeCtInterpolator::CONTROLLER_TYPE = "MeCtInterpolator";

void MeCtInterpolator::Context::child_channels_updated( MeController* child )
{
}

MeCtInterpolator::MeCtInterpolator(MeController* child1, MeController* child2, float weight, bool loop) : MeCtContainer(new MeCtInterpolator::Context(this)), _child1(child1), _child2(child2), _paramValue(weight), _loop(loop)
{
	if (_child1)
	{
		_sub_context->add_controller(_child1);
		_child1->ref();
	}
	if (_child2)
	{
		_sub_context->add_controller(_child2);
		_child2->ref();
	}

	// init channels
	_channels.init();
	_channels.merge(_child1->controller_channels());
	_channels.merge(_child2->controller_channels());

	// init keys
	initKeys();
	initDuration();
}

MeCtInterpolator::~MeCtInterpolator()
{
	if (_child1)
	{
		_sub_context->remove_controller(_child1);
		_child1->unref();
		_child1 = NULL;
	}
	if (_child2)
	{
		_sub_context->remove_controller(_child2);
		_child2->unref();
		_child2 = NULL;
	}
}

MeController* MeCtInterpolator::child(size_t n)
{
	if (n == 0) return _child1;
	else if (n == 1) return _child2;
	else
	{
		std::cout << "MeCtInterpolator Error: No accessable Controller!" << std::endl;
		return NULL;
	}
}

void MeCtInterpolator::initKeys()
{
	MeCtMotion* motionCt1 = dynamic_cast<MeCtMotion*> (_child1);
	MeCtMotion* motionCt2 = dynamic_cast<MeCtMotion*> (_child2);
	SkMotion* motion1 = motionCt1->motion();
	SkMotion* motion2 = motionCt2->motion();

	// get key map
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, std::vector<double>>::iterator iter = mcu.panim_key_map.find(motion1->name());
	if (iter != mcu.panim_key_map.end())
		_key1 = iter->second;
	iter = mcu.panim_key_map.find(motion2->name());
	if (iter != mcu.panim_key_map.end())
		_key2 = iter->second;

	// check key map validation
	if (_key1.size() != _key2.size() || _key1.size() == 1 || _key2.size() == 1)
	{
		_key1.clear();
		_key2.clear();
		std::cout << " MeCtInterpolator::initKeys() : Key map mismatching !!!" << std::endl;
	}
	else
	{
		for (size_t i = 0; i < _key1.size(); i++)
		{
			if (_key1[i] < 0)						_key1[i] = 0;
			if (_key1[i] > motion1->duration())		_key1[i] = motion1->duration();
			if (_key2[i] < 0)						_key2[i] = 0;
			if (_key2[i] > motion2->duration())		_key2[i] = motion2->duration();
		}
	}

	if (_key1.empty() && _key2.empty())
	{
		_key1.push_back(0);
		_key2.push_back(0);
		_key1.push_back(motion1->duration());
		_key2.push_back(motion2->duration());
	}
}

void MeCtInterpolator::initDuration()
{
	// init duration - has to happen after init keys
	double dur1 = _key1[_key1.size() - 1] - _key1[0];
	double dur2 = _key2[_key2.size() - 1] - _key2[0];

	_duration = dur1 * _paramValue + dur2 * (1 - _paramValue);
}

void MeCtInterpolator::controller_map_updated()
{
	_child1->remap();
	_child2->remap();
}

bool MeCtInterpolator::controller_evaluate(double t, MeFrameData& frame)
{
	// update the weight and duration 
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	float weight;
	if (mcu.is_fixed_weight)	weight = _paramValue;
	else						weight = mcu.panim_weight;

	if (!mcu.is_fixed_weight)
	{
		double d1 = _key1[_key1.size() - 1] - _key1[0];
		double d2 = _key2[_key2.size() - 1] - _key2[0];
		_duration = d1 * weight + d2 * (1 - weight);
	}

	bool continuing = true;
	if (_loop)
	{
		double x = t/_duration;
		if (x > 1.0)
			t = _duration * (x - int(x));
	}
	else
		continuing = t < _duration;



	// variable and pre-processing
	MeCtMotion* motionCt1 = dynamic_cast<MeCtMotion*> (_child1);
	MeCtMotion* motionCt2 = dynamic_cast<MeCtMotion*> (_child2);
	SkMotion* motion1 = motionCt1->motion();
	SkMotion* motion2 = motionCt2->motion();
	SrBuffer<float> buffer1;
	SrBuffer<float> buffer2;
	buffer1.size(frame.buffer().size());
	buffer2.size(frame.buffer().size());

	double delta_t1 = motion1->duration()/double(motion1->frames() - 1);	// 1/fps for motion1
	double delta_t2 = motion2->duration()/double(motion2->frames() - 1);	// 1/fps for motion2

	double dur = 0.0;
	double dur1 = 0.0;
	double dur2 = 0.0;
	double t1_offset = _key1[0];
	double t2_offset = _key2[0];
	double t_offset = t1_offset * weight + t2_offset * (1 - weight);
	double t_offset_absolute = t_offset;

	//------ Time Scaling
	bool warningFlag = false;
	for (size_t i = 0; i < _key1.size(); i++)
	{
		if (i != 0)
		{
			t1_offset = _key1[i - 1];
			t2_offset = _key2[i - 1];
			t_offset = t1_offset * weight + t2_offset * (1 - weight);
		}
		double t_absolute = _key1[i] * weight + _key2[i] * (1 - weight);
		if (t <= t_absolute - t_offset_absolute)
		{
			dur = t_absolute - t_offset;
			dur1 = _key1[i] - t1_offset;
			dur2 = _key2[i] - t2_offset;
			warningFlag = true;
			break;
		}
	}
	if (!warningFlag)
		std::cout << "MeCtInterpolator WARNING: Check the Timing" << std::endl;

	double t1 = ((t + t_offset_absolute - t_offset)/dur) * dur1 + t1_offset;
	double t2 = ((t + t_offset_absolute - t_offset)/dur) * dur2 + t2_offset;
	int last_frame1 = int(t1/delta_t1);
	int last_frame2 = int(t2/delta_t2);

//	std::cout << "t: " << t << " t1: " << t1 << " t2: " << t2 << " dur: " << dur << " dur1: " << dur1 << " dur2: " << dur2 << " weight: " << _paramValue << std::endl;

	//------ Do the interpolation here
	motion1->apply( float(t1), &buffer1[0], &motionCt1->get_context_map(), SkMotion::Linear, &last_frame1);
	motion2->apply( float(t2), &buffer2[0], &motionCt2->get_context_map(), SkMotion::Linear, &last_frame2);
	frame.buffer().setall(0);
	SkChannelArray& motion1Chan = motion1->channels();
	SkChannelArray& motion2Chan = motion2->channels();
	for (int i = 0; i < motion1Chan.size(); i++)
	{
		SkJoint* joint = motion1Chan[i].joint;
		if (joint)
		{
			int idMotion2 = motion2Chan.search(SkJointName(joint->name()), motion1Chan[i].type);
			// if both motion have this channel, do the interpolation
			if (idMotion2 >= 0)
			{
				int index1 = motionCt1->get_context_map().get(i);
				int index2 = motionCt2->get_context_map().get(idMotion2);
				int num = motion1->channels()[i].interp(&frame.buffer()[index1], &buffer1[index1], &buffer2[index2], 1 - weight);
			}
		}
	}
	return continuing;
}

SkChannelArray& MeCtInterpolator::controller_channels()
{
	return _channels;
}

double MeCtInterpolator::controller_duration()
{
	return _loop? -1.0: _duration;
}