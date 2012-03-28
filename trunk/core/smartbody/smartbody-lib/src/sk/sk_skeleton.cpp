/*
*  sk_skeleton.cpp - part of Motion Engine and SmartBody-lib
*  Copyright (C) 2008  University of Southern California
*
*  SmartBody-lib is free software: you can redistribute it and/or
*  modify it under the terms of the Lesser GNU General Public License
*  as published by the Free Software Foundation, version 3 of the
*  license.
*
*  SmartBody-lib is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  Lesser GNU General Public License for more details.
*
*  You should have received a copy of the Lesser GNU General Public
*  License along with SmartBody-lib.  If not, see:
*      http://www.gnu.org/licenses/lgpl-3.0.txt
*
*  CONTRIBUTORS:
*      Marcelo Kallmann, USC (currently UC Merced)
*      Andrew n marshall, USC
*      Marcus Thiebaux, USC
*/

# include <sr/sr_model.h>

# include <sk/sk_skeleton.h>
# include <sk/sk_posture.h>
#include <queue>
#include <sbm/SBJoint.h>

//============================ SkSkeleton ============================

SkSkeleton::SkSkeleton ()
{
	_name = "noname";
	_root = 0;
	_coldetid = -1;       // index used in collision detection
	_gmat_uptodate = false;
	_channels = new SkChannelArray;
	_channels->ref();
	_com.set(0, 0, 0);
}

SkSkeleton::SkSkeleton (SkSkeleton* origSkel)
{
	_name = origSkel->name();
	_skfilename = origSkel->skfilename();
	_root = new SmartBody::SBJoint(this, 0, origSkel->root()->rot_type(), origSkel->root()->index());
	copy_joint(_root, origSkel->root());
	_joints.push_back(_root);
	SkJoint* origParent = origSkel->root();
	SkJoint* thisParent = _root;
	create_joints(origParent, thisParent);

	_gmat_uptodate = origSkel->global_matrices_uptodate();
	_channels = new SkChannelArray;
	_channels->ref();
	SkChannelArray& origChannels = origSkel->channels();
	for (int c = 0; c < origChannels.size(); c++)
	{
		SkChannel& origChannel = origChannels.get(c);
		SkJoint* origJoint = origChannels.joint(c);
		if (origJoint)
		{
			SkJoint* joint = this->search_joint(origJoint->name().c_str());
			_channels->add(joint, origChannel.type, true);
		}
	}
	_channels->count_floats();
	_com = origSkel->com();

	compress ();
	make_active_channels();
}

SkSkeleton::~SkSkeleton ()
{
	init ();
	_channels->unref();
}

void SkSkeleton::copy(SkSkeleton* origSkel)
{
	_name = origSkel->name();
	_skfilename = origSkel->skfilename();
	//	_root = new SkJoint(this, 0, origSkel->root()->rot_type(), origSkel->root()->index());
	_root = new SmartBody::SBJoint(this, 0, origSkel->root()->rot_type(), origSkel->root()->index());
	copy_joint(_root, origSkel->root());
	_joints.push_back(_root);
	SkJoint* origParent = origSkel->root();
	SkJoint* thisParent = _root;
	create_joints(origParent, thisParent);

	_gmat_uptodate = origSkel->global_matrices_uptodate();
	_channels = new SkChannelArray;
	_channels->ref();
	SkChannelArray& origChannels = origSkel->channels();
	for (int c = 0; c < origChannels.size(); c++)
	{
		SkChannel& origChannel = origChannels.get(c);
		SkJoint* origJoint = origChannels.joint(c);
		SkJoint* joint = this->search_joint(origJoint->name().c_str());
		_channels->add(joint, origChannel.type, true);
	}
	_channels->count_floats();
	_com = origSkel->com();

	compress ();
}

void SkSkeleton::init ()
{
	_coldet_free_pairs.clear();
	_channels->init();
	while ( _postures.size()>0 ) 
	{
		SkPosture* posture = _postures.pop();
		delete posture;
	}
	while ( _joints.size()>0 ) 
	{
		SkJoint* joint = _joints[_joints.size() - 1];
		delete joint;
		_joints.pop_back();
	}
	_jointMap.clear();
	_root = 0;
	_gmat_uptodate = false;
}

void SkSkeleton::refresh_joints()
{
	_joints.clear();

	SkJoint* j = _root;
	std::queue<SkJoint*> queue;
	if (_root)
		queue.push(j);

	int curIndex = 0;

	while (!queue.empty())
	{
		SkJoint* joint = queue.front();
		joint->set_index(curIndex);
		curIndex++;
		_joints.push_back(joint);
		for (int c = 0; c < joint->num_children(); c++)
			queue.push(joint->child(c));
		queue.pop();
	}
}


SkJoint* SkSkeleton::add_joint ( SkJoint::RotType rtype, int parentid )
{
	_jointMap.clear();

	SkJoint* parent=0;
	if ( parentid<0 ) {
		if( _joints.size()>0 )
			parent =_joints[_joints.size() - 1];
		else
			parent=NULL;
	} else {
		parent = _joints[parentid];
	}

	SkJoint* j = new SmartBody::SBJoint ( this, parent, rtype, _joints.size() );
	_joints.push_back(j);

	if ( parent==NULL )
		_root=j;

	return j;
}

SkJoint* SkSkeleton::insert_new_root_joint ( SkJoint::RotType rtype )
{
	_jointMap.clear();
	SkJoint* old_root = _root;

	_root = new SmartBody::SBJoint ( this, 0, rtype, _joints.size() );
	_joints.push_back(_root);

	if( old_root ) {
		old_root->_parent = _root;

		/* thiebaux 6/19/2006 */
		_root->_children.push_back( old_root );
	}

	return _root;
}

SkJoint* SkSkeleton::linear_search_joint ( const char* n ) const
{
	std::string name(n);
	for (size_t i=0; i<_joints.size(); i++ )
	{ 
		if (_joints[i]->name() == name )
			return _joints[i];
	}
	return 0;
}

SkJoint* SkSkeleton::search_joint ( const char* n )
{
	if (_jointMap.size() == 0 &&
		_joints.size() > 0)
	{
		int jointSize = _joints.size();
		for (int i = 0; i < jointSize; i++)
		{
			_jointMap.insert(std::pair<std::string, SkJoint*>(_joints[i]->name(), _joints[i]));
		}
	}

	if (_extJointMap.size() == 0 &&
		_joints.size() > 0)
	{
		int jointSize = _joints.size();
		for (int i = 0; i < jointSize; i++)
		{
			_extJointMap.insert(std::pair<std::string, SkJoint*>(_joints[i]->extName(), _joints[i]));
		}
	}

	if (_extIDJointMap.size() == 0 &&
		_joints.size() > 0)
	{
		int jointSize = _joints.size();
		for (int i = 0; i < jointSize; i++)
		{
			_extIDJointMap.insert(std::pair<std::string, SkJoint*>(_joints[i]->extID(), _joints[i]));
		}
	}

	if (_extSIDJointMap.size() == 0 &&
		_joints.size() > 0)
	{
		int jointSize = _joints.size();
		for (int i = 0; i < jointSize; i++)
		{
			_extSIDJointMap.insert(std::pair<std::string, SkJoint*>(_joints[i]->extSID(), _joints[i]));
		}
	}

	std::map<std::string, SkJoint*>::iterator iter = _jointMap.find(n);
	if (iter != _jointMap.end())
	{
		return (*iter).second;
	}
	else if ( _extJointMap.find(n) != _extJointMap.end())
	{
		return (_extJointMap.find(n)->second);
	}
	else if ( _extIDJointMap.find(n) != _extIDJointMap.end())
	{
		return (_extIDJointMap.find(n)->second);
	}
	else if ( _extSIDJointMap.find(n) != _extSIDJointMap.end())
	{
		return (_extSIDJointMap.find(n)->second);
	}
	else
	{
		return NULL;
	}
}

void SkSkeleton::update_global_matrices ()
{
	if ( _gmat_uptodate ) return;
	if (!_root)
		return;
	_root->update_gmat();
	_gmat_uptodate = true;
}

void SkSkeleton::compress ()
{
	/*
	_coldet_free_pairs.compress();
	_channels->compress();
	_postures.compress();
	_joints.compress();

	int i;
	for ( i=0; i<_joints.size(); i++ )
	_joints[i]->_children.compress();
	*/
}

void SkSkeleton::set_geo_local ()
{
	SkJoint* j;

	for (size_t i=0; i<_joints.size(); i++ ) _joints[i]->init_values();
	update_global_matrices ();

	SrMat mat;
	for (size_t i=0; i<_joints.size(); i++ )
	{ j = _joints[i];
	mat = j->gmat();
	mat.invert();
	if ( j->_visgeo ) j->_visgeo->apply_transformation(mat);
	if ( j->_colgeo ) j->_colgeo->apply_transformation(mat);
	}


	update_global_matrices ();
}

float SkSkeleton::getCurrentHeight()
{
	SrBox initialBoundingBox = getBoundingBox();	
	return initialBoundingBox.size().y;
}

SrBox SkSkeleton::getBoundingBox()
{
	SrBox initialBoundingBox;
	update_global_matrices();
	std::vector<SkJoint*>& joints = get_joint_array();
	for (size_t j = 0; j < joints.size(); j++)
	{
		joints[j]->update_gmat();
		const SrMat& gmat = joints[j]->gmat();
		SrVec point(gmat.get(3, 0), gmat.get(3, 1), gmat.get(3, 2));
		initialBoundingBox.extend(point);
	}

	return initialBoundingBox;
}


void SkSkeleton::copy_joint(SkJoint* dest, SkJoint* src)
{
	dest->name(src->name());
	dest->extName(src->extName());
	dest->extID(src->extID());
	dest->extSID(src->extSID());
	dest->visgeo(src->visgeo());
	dest->colgeo(src->colgeo());

	SkJointPos* srcPos = src->pos();
	SkJointPos* destPos = dest->pos();
	if (!srcPos->frozen(SkVecLimits::X))
	{
		destPos->limits(SkVecLimits::X, srcPos->limits(SkVecLimits::X));
		destPos->lower_limit(SkVecLimits::X, srcPos->lower_limit(SkVecLimits::X));
		destPos->upper_limit(SkVecLimits::X, srcPos->upper_limit(SkVecLimits::X));
	}
	if (!srcPos->frozen(SkVecLimits::Y))
	{
		destPos->limits(SkVecLimits::Y, srcPos->limits(SkVecLimits::Y));
		destPos->lower_limit(SkVecLimits::Y, srcPos->lower_limit(SkVecLimits::Y));
		destPos->upper_limit(SkVecLimits::Y, srcPos->upper_limit(SkVecLimits::Y));
	}
	if (!srcPos->frozen(SkVecLimits::Z))
	{
		destPos->limits(SkVecLimits::Z, srcPos->limits(SkVecLimits::Z));
		destPos->lower_limit(SkVecLimits::Z, srcPos->lower_limit(SkVecLimits::Z));
		destPos->upper_limit(SkVecLimits::Z, srcPos->upper_limit(SkVecLimits::Z));
	}



	if (src->quat()->active())
		dest->quat()->activate();

	dest->offset(src->offset());
	dest->updateGmatZero(src->gmatZero());
	SkJointQuat* srcQuat = src->quat();
	SkJointQuat* destQuat = dest->quat();
	destQuat->value(srcQuat->value());
	destQuat->prerot(srcQuat->prerot());
	destQuat->postrot(srcQuat->postrot());
	destQuat->orientation(srcQuat->orientation());

	dest->mass(src->mass());
}

void SkSkeleton::create_joints(SkJoint* origParent, SkJoint* parent)
{
	for (int i = 0; i < origParent->num_children(); i++)
	{
		SkJoint* newJoint = new SmartBody::SBJoint(this, parent, origParent->child(i)->rot_type(), origParent->child(i)->index());
		// ??? SkJoint* newJoint = new SkJoint(this, parent, origParent->child(i)->rot_type(), _joints.size());
		_joints.push_back(newJoint);

		copy_joint(newJoint, origParent->child(i));
		create_joints(origParent->child(i), newJoint);
	}
}


//============================ End of File ============================
