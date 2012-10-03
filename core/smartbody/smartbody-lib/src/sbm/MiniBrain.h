#ifndef _MINIBRAIN_H_
#define _MINIBRAIN_H_

#include <sr/sr_vec.h>
#include <map>
#include <string>
#include <list>

struct ObjectData
{
	SrVec velocity;
	SrVec position;
	std::list<SrVec> cachePositions;
	SrVec relativeVelocity;
	SrVec relativePosition;
	double startGazeTime;
	bool isAnimate;
};

class SbmCharacter;

class MiniBrain
{
	public:
		MiniBrain();
		virtual ~MiniBrain();

		virtual void update(SbmCharacter* character, double time, double dt);

	protected:
		std::map<std::string, ObjectData> _data;
		int _cacheLimit;
};

#endif
