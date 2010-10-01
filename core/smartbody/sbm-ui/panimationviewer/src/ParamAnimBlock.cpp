#include "ParamAnimBlock.h"

CorrespondenceMark::CorrespondenceMark()
{
	_attached = NULL;
}

CorrespondenceMark::~CorrespondenceMark()
{
}


void CorrespondenceMark::attach(CorrespondenceMark* mark)
{
	_attached = mark;
}

CorrespondenceMark* CorrespondenceMark::getAttachedMark()
{
	return _attached;
}

FootstepMark::FootstepMark()
{
}

ParamAnimBlock::ParamAnimBlock()
{
}

ParamAnimTrack::ParamAnimTrack()
{
}

