#ifndef _ME_CT_INTERPOLATOR_H_
#define _ME_CT_INTERPOLATOR_H_

#include <Me/me_ct_container.hpp>
#include <ME/me_ct_motion.h>
#include <map>

// This controller takes two motions and blend them according to key time
// Should only hold two controllers
class MeCtInterpolator : public MeCtContainer
{
public:
	static const char* CONTROLLER_TYPE;

	class Context : public MeCtContainer::Context 
	{
	protected:
		static const char* CONTEXT_TYPE;
	public:
		Context( MeCtInterpolator* container, MeControllerContext* context = NULL )
			:	MeCtContainer::Context( container, context )
		{}

		const char* context_type() const {	return CONTEXT_TYPE; }
		void child_channels_updated( MeController* child );
	};

public:
	// constructor
	MeCtInterpolator(MeController* child1 = NULL, MeController* child2 = NULL, float weight = 1.0, bool loop = false);
	~MeCtInterpolator();

	// child accessor
	MeController* child(size_t n);

	// set and get param value (blending weight)
	float getParamValue() {return _paramValue;}
	void setParamValue(float weight) {_paramValue = weight;}

	// callbacks for the base class
	virtual void controller_map_updated();
    virtual bool controller_evaluate( double t, MeFrameData& frame );
    virtual SkChannelArray& controller_channels();
    virtual double controller_duration();
	virtual const char* controller_type() const {return CONTROLLER_TYPE;}

private:
	// Key Info Init
	void initKeys();
	// Duration init, has to happen after init keys
	void initDuration();

private:

	bool _loop;
	double _duration;
	MeController* _child1;
	MeController* _child2;
	float _paramValue;
	
	// key time infomations, matching key info, ascending order
	std::vector<double> _key1;				// key frame numbers for motion1, these got passed in
	std::vector<double> _key2;				// key frame numbers for motion2, these got passed in

	SkChannelArray _channels;
};

#endif
