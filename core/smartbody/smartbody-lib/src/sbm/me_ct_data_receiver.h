#ifndef _ME_CT_DATA_RECEIVER_H_
#define _ME_CT_DATA_RECEIVER_H_

#include <sbm/SBController.h>
#include <sk/sk_skeleton.h>
#include <map>

class MeCtDataReceiver : public SmartBody::SBController
{
	public:
		static const char* CONTROLLER_TYPE;

	private:
		double			_prevTime;
		double			_dt;
		float 			_duration;
		SkChannelArray	_channels;
		SkSkeleton* 	_skeleton;
		bool			_valid;
		std::map<std::string, SrVec>	_posMap;				// global position
		std::map<std::string, SrVec>	_startingPos;			// starting position
		std::map<std::string, SrQuat>	_quatMap;				// local rotation

	public:
		MeCtDataReceiver(SkSkeleton* skel);
		~MeCtDataReceiver();

		bool getValid()						{return _valid;}
		void setValid(bool v)				{_valid = v;}

		void setGlobalPosition(std::string jName, SrVec& pos);
		void setLocalRotation(std::string jName, SrQuat& q);

	private:
		virtual bool controller_evaluate(double t, MeFrameData& frame);
		virtual SkChannelArray& controller_channels()	{ return(_channels); }
		virtual double controller_duration()			{ return((double)_duration); }
		virtual const char* controller_type() const		{ return(CONTROLLER_TYPE); }
};

#endif