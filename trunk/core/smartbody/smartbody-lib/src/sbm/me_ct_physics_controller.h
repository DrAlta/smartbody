#ifndef		_ME_CT_PHYSICS_CONTROLLER_H_
#define _ME_CT_PHYSICS_CONTROLLER_H_

#include <sbm/SBController.h>
#include <sbm/sbm_character.hpp>
#include <map>

class MeCtPhysicsController : public SmartBody::SBController
{
	public:
		static std::string CONTROLLER_TYPE;

	private:
		double			_prevTime;
		double			_dt;
		float 			_duration;
		SkChannelArray	_channels;
		SbmCharacter* 	_character;
		bool			_valid;	
	public:
		MeCtPhysicsController(SbmCharacter* character);
		~MeCtPhysicsController();

		bool getValid()						{return _valid;}
		void setValid(bool v)				{_valid = v;}		
	private:
		virtual bool controller_evaluate(double t, MeFrameData& frame);
		virtual SkChannelArray& controller_channels()	{ return(_channels); }
		virtual double controller_duration()			{ return((double)_duration); }
		virtual const std::string& controller_type() const		{ return(CONTROLLER_TYPE); }
	protected:
		SrVec computePDTorque(SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD);
		
};

#endif