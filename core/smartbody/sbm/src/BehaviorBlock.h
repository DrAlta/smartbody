#ifndef _BEHAVIORBLOCK_
#define _BEHAVIORBLOCK_

#include <sbm/bml.hpp>
#include "NonLinearEditor.h"

class BehaviorMark : public Mark
{
	public:
		BehaviorMark();		
};

class BehaviorTimingMark : public Mark
{
	public:
		BehaviorTimingMark();		
	
	protected:
};

class BehaviorBlock : public Block
{
	public:
		BehaviorBlock();
	
	
	protected:
		BML::BmlRequest* m_behavior;;
		
};

class BehaviorTrack : public Track
{
	public:
		BehaviorTrack();
};

class RequestMark : public Mark
{
	public:
		RequestMark();		
};

class RequestTimingMark : public Mark
{
	public:
		RequestTimingMark();		
	
};

class RequestBlock : public Block
{
	public:
		RequestBlock();
	
		
};

class EventBlock : public Block
{
	public:
		EventBlock();
	
		
};

class MotionBlock : public Block
{
	public:
		MotionBlock();
	
		
};

class NodBlock : public Block
{
	public:
		NodBlock();
	
		
};


class RequestTrack : public Track
{
	public:
		RequestTrack();
};


#endif