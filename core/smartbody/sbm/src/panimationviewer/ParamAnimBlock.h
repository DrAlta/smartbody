#ifndef _PARAMANIMBLOCK_
#define _PARAMANIMBLOCK_

#include "nle/NonLinearEditor.h"

class CorrespondenceMark : public nle::Mark
{
	public:
		CorrespondenceMark();
		~CorrespondenceMark();

		void attach(CorrespondenceMark* mark);
		CorrespondenceMark* getAttachedMark();

	CorrespondenceMark* _attached;
};


class FootstepMark : public nle::Mark
{
	public:
		FootstepMark();		
};

class ParamAnimBlock : public nle::Block
{
	public:
		ParamAnimBlock();
		
};

class ParamAnimTrack : public nle::Track
{
	public:
		ParamAnimTrack();
};


#endif