#include "action_unit.hpp"

ActionUnit::ActionUnit(SkMotion* unified ) :
		left( unified ),
		right( unified )
{
	m_isLeft = false;
	m_isRight = false;
	m_isBilateral = true;
}

ActionUnit::ActionUnit( SkMotion* left, SkMotion* right ) :
	left( left ),
	right( right )
{
	reset_type();
}

ActionUnit::ActionUnit(ActionUnit* source)
{
	reset_type();

	if (source->is_left())
		set_left();
	if (source->is_right())
		set_right();
	if (source->is_bilateral())
		set_bilateral();
	left = source->left;
	right = source->right;
}

ActionUnit::~ActionUnit()
{
}

bool ActionUnit::is_bilateral() const
{
	return m_isBilateral;
}

void ActionUnit::reset_type()
{
	m_isLeft = false;
	m_isRight = false;
	 m_isBilateral = false;
}

void ActionUnit::set_left()
{
	m_isLeft = true;
}

void ActionUnit::set_bilateral()
{
	m_isBilateral = true;
}

void ActionUnit::set_right()
{
	m_isRight = true;
}

bool ActionUnit::is_left() const
{
	return m_isLeft;
}

bool ActionUnit::is_right() const
{
	return m_isRight;
}

void ActionUnit::set( SkMotion* motion )
{
	set( motion, motion );
}

void ActionUnit::set( SkMotion* left, SkMotion* right )
{
	this->left	= left;
	this->right	= right;
}

