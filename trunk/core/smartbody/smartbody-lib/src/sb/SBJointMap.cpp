#include <sb/SBJointMap.h>
#include "sk/sk_channel_array.h"
#include "sr/sr_string.h"
#include <sb/SBMotion.h>
#include <sb/SBSkeleton.h>

namespace SmartBody {

SBJointMap::SBJointMap()
{
}

SBJointMap::~SBJointMap()
{
}

void SBJointMap::applyMotion(SmartBody::SBMotion* motion)
{
	if (!motion)
		return;
	
	SkChannelArray& channels = motion->channels();
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).second;
		channels.changeChannelName(from, to);
	}

}

void SBJointMap::applySkeleton(SmartBody::SBSkeleton* skeleton)
{
	if (!skeleton)
		return;
	
	std::vector<SkJoint*> joints = skeleton->joints();
	for (size_t j = 0; j < joints.size(); j++)
	{
		for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
			 iter != _map.end();
			 iter++)
		{
			std::string from = (*iter).first;
			std::string to = (*iter).second;
			if (joints[j]->name() == from.c_str())
			{
				joints[j]->name(to);
			}
		}
	}
	SkChannelArray& channels = skeleton->channels();
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).second;
		channels.changeChannelName(from, to);
	}
}

void SBJointMap::setMapping(const std::string& from, const std::string& to)
{
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string f = (*iter).first;
		if (from == f)
		{
			(*iter).second = to;
			return;
		}
	}

	_map.push_back(std::pair<std::string, std::string>(from, to));
}

void SBJointMap::removeMapping(const std::string& from)
{
}

std::string SBJointMap::getMapSource(const std::string& to)
{
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string t = (*iter).second;
		if (to == t)
		{
			std::string f = (*iter).first;
			return f;
		}
	}
	return "";
}

std::string SBJointMap::getMapTarget(const std::string& from)
{
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string f = (*iter).first;
		if (from == f)
		{
			std::string t = (*iter).second;
			return t;
		}
	}
	return "";
}

int SBJointMap::getNumMappings()
{
	return _map.size();
}

std::string SBJointMap::getTarget(int num)
{
	if (_map.size() > (size_t) num)
	{
		return _map[num].first;
	}
	return "";
}

std::string SBJointMap::getSource(int num)
{
	if (_map.size() > (size_t) num)
	{
		return _map[num].second;
	}
	return "";
}


// Automatic joint name matching to standard SmartBody names
// Based mainly on skeleton hierarchy/symmetry but also used a few hardcoded keywords
bool SBJointMap::guessMapping(SmartBody::SBSkeleton* skeleton)
{
	/* this function tries to search the following joints for SmartBody
	// added by David Huang 2012-06
	SmartBody_name <- (MotionBuilder name)

	base (Hips)
	- spine1 <- (Spine)
	-- spine2 <- (Spine1)
	--- spine3 <- (Spine3, chest equivalent)
	----- spine4 <- (Neck)
	------- skullbase <- (Head)
	- l_acromioclavicular <- (LeftShoulder)
	--- l_shoulder <- (LeftArm)
	----- l_elbow  <- (LeftForeArm)
	------- l_forearm <- (LeftForeArmTwist/LeftForeArmRoll)
	------- l_wrist <- (LeftHand)
	--------- l_thumb1~4 <- (LeftHandThumb1~4)
	--------- l_index1~4 <- (LeftHandIndex1~4)
	--------- l_middle1~4 <- (LeftHandMiddle1~4)
	--------- l_ring1~4  <- (LeftHandRing1~4)
	--------- l_pinky1~4 <- (LeftHandPinky1~4)
	- l_hip <- (LeftUpLeg)
	--- l_knee <- (LeftLeg)
	----- l_ankle <- (LeftFoot)
	------- l_forefoot <- (LeftToeBase)
	(... right side joints ...)

	note: SmartBody doesn't have "Heel" and "UpArmTwist/UpArmRoll" joints.	*/

	if (!skeleton)
		return false;

	SkJoint *base = 0;
	SkJoint *spine1 = 0;
	SkJoint *spine2 = 0;
	SkJoint *spine3_chest = 0; // SB: spine3
	SkJoint *spine4_neck = 0; // SB: spine4
	SkJoint *skullbase_head = 0;
	SkJoint *l_AC_shoulder=0,	*r_AC_shoulder=0; // SB: l/r_acromioclavicular
	SkJoint *l_shoulder_arm=0,	*r_shoulder_arm=0; // SB: l/r_shoulder
	SkJoint *l_elbow=0,			*r_elbow=0;
	SkJoint *l_forearm_roll=0,	*r_forearm_roll=0; // SB: l/r_forearm
	SkJoint *l_wrist=0,			*r_wrist=0;
	SkJoint *l_hip_upleg=0,		*r_hip_upleg=0; // SB: l/r_hip
	SkJoint *l_knee=0,			*r_knee=0;
	SkJoint *l_ankle=0,			*r_ankle=0;
	SkJoint *l_forefoot_toe=0,	*r_forefoot_toe=0; // SB: l/r_forefoot

	SkJoint *l_thumb1=0,*l_thumb2=0, *l_thumb3=0, *l_thumb4=0;
	SkJoint *l_index1=0, *l_index2=0, *l_index3=0, *l_index4=0;
	SkJoint *l_middle1=0, *l_middle2=0, *l_middle3=0, *l_middle4=0;
	SkJoint *l_ring1=0,	*l_ring2=0, *l_ring3=0, *l_ring4=0;
	SkJoint *l_pinky1=0, *l_pinky2=0, *l_pinky3=0, *l_pinky4=0;

	SkJoint *r_thumb1=0, *r_thumb2=0, *r_thumb3=0, *r_thumb4=0;
	SkJoint *r_index1=0, *r_index2=0, *r_index3=0, *r_index4=0;
	SkJoint *r_middle1=0, *r_middle2=0, *r_middle3=0, *r_middle4=0;
	SkJoint *r_ring1=0, *r_ring2=0, *r_ring3=0, *r_ring4=0;
	SkJoint *r_pinky1=0, *r_pinky2=0, *r_pinky3=0, *r_pinky4=0;


	const std::vector<SkJoint*> jnts = skeleton->joints();

	LOG("Automatic joint name matching for %s \n", skeleton->name().c_str());

	// TODO: check joint names make sure they are unique !

	//-------------------------------------------------------------------------
	// first find base
	for(unsigned int i=0; i<jnts.size(); i++)
	{
		if(jnts[i]->num_children()>=2)
		{
			base = jnts[i];
			setJointMap("base", base);
			break;
		}
	}
	if(!base)
	{
		LOG("guessMap: base joint NOT found, aborting...\n");
		return false;
	}
	if(base->num_children()==2)
	{
		SkJoint* j1 = base->child(0);
		SkJoint* j2 = base->child(1);
		SkJoint* ja;
		SkJoint* jb;
		if(j1->num_children()==2 && j2->num_children()==2)
		{
			LOG("guessMap: TODO: base has 2 children, each has 2 children of their own.\n");
			return false;
		}
		else if(j1->num_children()==2) // j1 is parent of 2 uplegs
		{
			ja = j1->child(0); jb = j1->child(1); spine1 = j2;
		}
		else if(j2->num_children()==2) // j2 is parent of 2 uplegs
		{
			ja = j2->child(0); jb = j2->child(1); spine1 = j1;
		}
		else
		{
			LOG("guessMap: TODO: base has 2 children.\n");
			return false;
		}
		// guess left/right upleg from the names
		guessLeftRightFromJntNames(ja, jb, l_hip_upleg, r_hip_upleg);

		setJointMap("spine1", spine1);
		setJointMap("l_hip_upleg", l_hip_upleg);
		setJointMap("r_hip_upleg", r_hip_upleg);
	}
	else if(base->num_children()==3)
	{
		//mDBGTXT("base has 3 children.");
		SkJoint* j1 = base->child(0);
		SkJoint* j2 = base->child(1);
		SkJoint* j3 = base->child(2);
		SkJoint* ja;
		SkJoint* jb;

		if(countChildren(j1)==countChildren(j2) && countChildren(j1)==countChildren(j3))
		{
			// TODO: need to dig further, but should never happen
			LOG("guessMap: Can not figure out UpLeg joints, aborting...\n");
			return false;
		}
		else if(countChildren(j1)==countChildren(j2))
		{
			// j1, j2 are upleg joints
			ja = j1; jb = j2; spine1 = j3;
		}
		else if(countChildren(j1)==countChildren(j3))
		{
			// j1, j3 are upleg joints
			ja = j1; jb = j3; spine1 = j2;
		}
		else if(countChildren(j2)==countChildren(j3))
		{
			// j2, j3 are upleg joints
			ja = j2; jb = j3; spine1 = j1;
		}
		else
		{
			// error, j1 j2 j3 each has different children joints counts
			LOG("guessMap: can not figure out UpLeg joint, aborting...\n");
			return false;
		}
		// guess left/right upleg from the names
		guessLeftRightFromJntNames(ja, jb, l_hip_upleg, r_hip_upleg);
		
		setJointMap("spine1", spine1);
		setJointMap("l_hip_upleg", l_hip_upleg);
		setJointMap("r_hip_upleg", r_hip_upleg);
	}
	else if(base->num_children() > 3)
	{
		// TODO: need to dig further, but should never happen
		LOG("guessMap: TODO: base has 4 or more children.\n");
		return false;
	}

	//-------------------------------------------------------------------------
	// CONTINUE with spine1, try finding spine3_chest, spine4_neck, l/r_AC_shoulder
	for(unsigned int i=getJointIndex(spine1); i<jnts.size(); i++)
	{
		SkJoint* j = jnts[i];
		if(j->num_children()>=2)
		{
			spine3_chest = j;
			break;
		}
	}
	if(!spine3_chest)
	{
		LOG("guessMap: spine3_chest joint NOT found, aborting...\n");
		return false;
	}
	setJointMap("spine3_chest", spine3_chest);

	if(spine3_chest->num_children() == 2)
	{
		LOG("guessMap: spine3_chest has 2 children, NOT tested.\n");
		SkJoint* j1 = spine3_chest->child(0);
		SkJoint* j2 = spine3_chest->child(1);
		SkJoint* ja;
		SkJoint* jb;
		if(j1->num_children()==2 && j2->num_children()==2)
		{
			LOG("guessMap: TODO: spine3_chest has 2 children, each has 2 children of their own.\n");
			return false;
		}
		else if(j1->num_children()==2) // j1 is parent of 2 shoulders
		{
			ja = j1->child(0); jb = j1->child(1); spine4_neck = j2;
		}
		else if(j2->num_children()==2) // j2 is parent of 2 shoulders
		{
			ja = j2->child(0); jb = j2->child(1); spine4_neck = j1;
		}
		else
		{
			LOG("guessMap: TODO: spine3_chest has 2 children.\n");
			return false;
		}
		// guess left/right shoulder from the names
		guessLeftRightFromJntNames(ja, jb, l_AC_shoulder, r_AC_shoulder);

		setJointMap("spine4_neck", spine4_neck);
		setJointMap("l_AC_shoulder", l_AC_shoulder);
		setJointMap("r_AC_shoulder", r_AC_shoulder);
	}
	else if(spine3_chest->num_children() == 3)
	{
		//mDBGTXT("spine3_chest has 3 children.");
		SkJoint* j1 = spine3_chest->child(0);
		SkJoint* j2 = spine3_chest->child(1);
		SkJoint* j3 = spine3_chest->child(2);
		SkJoint* ja;
		SkJoint* jb;
		if(countChildren(j1)==countChildren(j2) && countChildren(j1)==countChildren(j3))
		{
			// TODO: need to dig further, but should never happen
			LOG("guessMap: can not figure out Shoulder joints, aborting...\n");
			return false;
		}
		else if(countChildren(j1)==countChildren(j2))
		{
			// j1, j2 are upleg joints
			ja = j1; jb = j2; spine4_neck = j3;
		}
		else if(countChildren(j1)==countChildren(j3))
		{
			// j1, j3 are upleg joints
			ja = j1; jb = j3; spine4_neck = j2;
		}
		else if(countChildren(j2)==countChildren(j3))
		{
			// j2, j3 are upleg joints
			ja = j2; jb = j3; spine4_neck = j1;
		}
		else
		{
			// error, j1 j2 j3 each has different children joints counts
			LOG("guessMap: can not figure out shoulder joint, aborting...\n");
			return false;
		}
		// guess left/right shoulder from the names
		guessLeftRightFromJntNames(ja, jb, l_AC_shoulder, r_AC_shoulder);

		setJointMap("spine4_neck", spine4_neck);
		setJointMap("l_AC_shoulder", l_AC_shoulder);
		setJointMap("r_AC_shoulder", r_AC_shoulder);
	}
	else if(spine3_chest->num_children() > 3)
	{
		// probably not a humanoid, should never happen
		LOG("guessMap: spine3_chest has 4 or more children, probably not humanoid.\n");
		return false;
	}


	//-------------------------------------------------------------------------
	// CONTINUE with spine4_neck, try finding skullbase_head
	if(spine4_neck && spine4_neck->num_children()>0)
	{
		std::vector<SkJoint*> j_list;
		listChildrenJoints(spine4_neck, j_list);
		for(unsigned int i=0; i<j_list.size(); i++)
		{
			SkJoint* j = j_list[i];
			SrString jname(j->name().c_str());
			if(jname.search("head")>=0) // FIXME: try search keyword "head"
			{
				skullbase_head = j;
				break;
			}
		}
		if(skullbase_head==0)
		{
			skullbase_head = getDeepestLevelJoint(j_list);
			if(skullbase_head)
			{
				// try to avoid Head_End
				if(getJointHierarchyLevel(skullbase_head->parent()) > getJointHierarchyLevel(spine4_neck))
					skullbase_head = skullbase_head->parent();
			}
		}
		setJointMap("skullbase_head", skullbase_head);

		// TODO: for those with facial bones, maybe check global position ?
	}

	//-------------------------------------------------------------------------
	// CONTINUE with left/right shoulders, try finding hand/hand_end
	{
		SkJoint* j1 = 0;
		SkJoint* j2 = 0;
		SkJoint* ja = 0;
		SkJoint* jb = 0;
		for(unsigned int i=getJointIndex(l_AC_shoulder), j=getJointIndex(r_AC_shoulder); i<jnts.size()&&j<jnts.size(); i++,j++)
		{
			j1 = jnts[i];
			j2 = jnts[j];
			SrString jname(j1->name().c_str());
			if(j1->num_children()==5 && j2->num_children()==5) // this must be hand joint
			{
				ja = j1; jb = j2;
				break;
			}
			else if(jname.search("hand")>=0 || jname.search("wrist")>=0) // FIXME: try search keyword "hand"
			{
				ja = j1; jb = j2;
				break;
			}
		}
		if(!ja || !jb) 
		{
			// try finding the deepest joint as Hand_End
			std::vector<SkJoint*> j_list1, j_list2;
			listChildrenJoints(l_AC_shoulder, j_list1); listChildrenJoints(r_AC_shoulder, j_list2);
			j1 = getDeepestLevelJoint(j_list1); j2 = getDeepestLevelJoint(j_list2);
			if(j1 && j2)
			{
				LOG("guessMap: Hand_End joints: %s, %s \n", j1->name().c_str(), j2->name().c_str());
				// try finding hand joints
				ja = j1->parent();
				jb = j2->parent();
			}
		}
		if(ja && jb)
		{
			// guess left/right hand from the names
			guessLeftRightFromJntNames(ja, jb, l_wrist, r_wrist);

			setJointMap("l_wrist", l_wrist);
			setJointMap("r_wrist", r_wrist);
		}
		else
		{
			LOG("guessMap: hand/hand_end joint NOT found, aborting...\n");
			return false;
		}
	}

	//-------------------------------------------------------------------------
	// CONTINUE to find l/r_elbow using l/r_AC_shoulder and l/r_wrist
	{
		if(!(l_wrist&&l_AC_shoulder))
		{
			LOG("guessMap: l_wrist or l_AC_shoulder NOT found, abort finding l_elbow...\n");
			return false;
		}
		if(l_wrist->num_children()==5) // this must be hand joint
		{
			if(getJointHierarchyLevel(l_wrist) - getJointHierarchyLevel(l_AC_shoulder) == 2)
			{
				l_elbow = l_wrist->parent();
				r_elbow = r_wrist->parent();
				setJointMap("l_elbow", l_elbow);
				setJointMap("r_elbow", r_elbow);
				l_shoulder_arm = l_AC_shoulder; // use upperArm to replace AC_shoulder
				r_shoulder_arm = r_AC_shoulder;
				LOG("guessMap: Use l/r_shoulder_arm to replace l/r_AC_shoulder.\n");
				setJointMap("l_shoulder_arm", l_shoulder_arm);
				setJointMap("r_shoulder_arm", r_shoulder_arm);
			}
			else if(getJointHierarchyLevel(l_wrist) - getJointHierarchyLevel(l_AC_shoulder) == 3)
			{
				if(l_AC_shoulder->num_children()==1 && (l_AC_shoulder->child(0)->num_children()==1||l_AC_shoulder->child(0)->num_children()==2))
				{
					l_shoulder_arm = l_AC_shoulder->child(0); // most likely the upperArm joint
					r_shoulder_arm = r_AC_shoulder->child(0);
					setJointMap("l_shoulder_arm", l_shoulder_arm);
					setJointMap("r_shoulder_arm", r_shoulder_arm);
					l_elbow = l_wrist->parent();
					r_elbow = r_wrist->parent();
					setJointMap("l_elbow", l_elbow);
					setJointMap("r_elbow", r_elbow);
					if(l_shoulder_arm->num_children()==2)
						LOG("guessMap: Might have an upperArm twist/roll joint.\n");
				}
				else if(l_AC_shoulder->num_children()==2) // should be upperArm, with elbow and 1 twist children joints
				{
					l_shoulder_arm = l_AC_shoulder; // should be upperArm joint
					r_shoulder_arm = r_AC_shoulder;
					setJointMap("l_shoulder_arm", l_shoulder_arm);
					setJointMap("r_shoulder_arm", r_shoulder_arm);
					l_AC_shoulder = 0; r_AC_shoulder = 0; // don't use AC_shoulder anymore

					LOG("guessMap: Might have two arm twist/roll joints.\n");
				}
			}
			else if(getJointHierarchyLevel(l_wrist) - getJointHierarchyLevel(l_AC_shoulder) == 4)
			{ /* Could be shoulder -> uparm -> elbow -> forarmTwist -> wrist
				 Or       shoulder -> uparm -> uparmTwist -> elbow -> wrist */
				l_shoulder_arm = l_AC_shoulder->child(0);
				r_shoulder_arm = r_AC_shoulder->child(0);
				setJointMap("l_shoulder_arm", l_shoulder_arm);
				setJointMap("r_shoulder_arm", r_shoulder_arm);
				SkJoint* ja = l_wrist->parent();
				SkJoint* jb = r_wrist->parent();
				SrString jname(ja->name().c_str());
				if(jname.search("twist")>=0 || jname.search("roll")>=0)
				{
					l_forearm_roll = ja;
					r_forearm_roll = jb;
					setJointMap("l_forearm_roll", l_forearm_roll);
					setJointMap("r_forearm_roll", r_forearm_roll);
					l_elbow = ja->parent();
					r_elbow = jb->parent();
				}
				else
				{
					l_elbow = ja;
					r_elbow = jb;
				}
				setJointMap("l_elbow", l_elbow);
				setJointMap("r_elbow", r_elbow);
			}
			else if(getJointHierarchyLevel(l_wrist) - getJointHierarchyLevel(l_AC_shoulder) == 5)
			{ // Might be shoulder -> uparm -> uparmTwist -> elbow -> forearmTwist -> wrist 
				l_shoulder_arm = l_AC_shoulder->child(0);
				r_shoulder_arm = r_AC_shoulder->child(0);
				setJointMap("l_shoulder_arm", l_shoulder_arm);
				setJointMap("r_shoulder_arm", r_shoulder_arm);
				l_forearm_roll = l_wrist->parent();
				r_forearm_roll = r_wrist->parent();
				setJointMap("l_forearm_roll", l_forearm_roll);
				setJointMap("r_forearm_roll", r_forearm_roll);
				l_elbow = l_forearm_roll->parent();
				r_elbow = r_forearm_roll->parent();
				setJointMap("l_elbow", l_elbow);
				setJointMap("r_elbow", r_elbow);
			}
		}
		else // using guessed hand joint with less than 5 fingers
		{
			if(getJointHierarchyLevel(l_wrist) - getJointHierarchyLevel(l_AC_shoulder) == 2)
			{
				l_elbow = l_wrist->parent();
				r_elbow = r_wrist->parent();
				setJointMap("l_elbow", l_elbow);
				setJointMap("r_elbow", r_elbow);
				l_shoulder_arm = l_AC_shoulder; // make upperArm the same as AC_shoulder
				r_shoulder_arm = r_AC_shoulder;
				setJointMap("l_shoulder_arm", l_shoulder_arm);
				setJointMap("r_shoulder_arm", r_shoulder_arm);
			}
			else
			{
				l_elbow = l_wrist->parent();
				r_elbow = r_wrist->parent();
				setJointMap("l_elbow", l_elbow);
				setJointMap("r_elbow", r_elbow);
			}
		}

	}

	//-------------------------------------------------------------------------
	// CONTINUE with left/right uplegs, try finding foot/toe_end
	{
		SkJoint* j1 = 0;
		SkJoint* j2 = 0;
		SkJoint* ja = 0;
		SkJoint* jb = 0;
		for(unsigned int i=getJointIndex(l_hip_upleg), j=getJointIndex(r_hip_upleg); i<jnts.size()&&j<jnts.size(); i++,j++)
		{
			j1 = jnts[i];
			j2 = jnts[j];
			if(j1->num_children()==2 && j2->num_children()==2) // ankle with foot and heel children joints
			{
				ja = j1; jb = j2;
				break;
			}
		}
		if(ja && jb)
		{
			// guess left/right ankle from the names
			guessLeftRightFromJntNames(ja, jb, l_ankle, r_ankle);

			setJointMap("l_ankle", l_ankle);
			setJointMap("r_ankle", r_ankle);
		}
		else if(!ja || !jb) // no heel
		{
			// first try search "ankle" or "foot"
			for(unsigned int i=getJointIndex(l_hip_upleg), j=getJointIndex(r_hip_upleg); i<jnts.size()&&j<jnts.size(); i++,j++)
			{
				j1 = jnts[i];
				j2 = jnts[j];
				SrString jname(j1->name().c_str());
				if(jname.search("ankle")>=0 || jname.search("foot")>=0) // try search keyword "ankle" FIXME
				{
					ja = j1; jb = j2;
					break;
				}
			}
			if(ja && jb)
			{
				// guess left/right ankle from the names
				guessLeftRightFromJntNames(ja, jb, l_ankle, r_ankle);

				setJointMap("l_ankle", l_ankle);
				setJointMap("r_ankle", r_ankle);
			}
		}

		if(l_ankle && r_ankle) // find toe (assuming they exist)
		{
			std::vector<SkJoint*> j_list1, j_list2;
			listChildrenJoints(l_ankle, j_list1); listChildrenJoints(r_ankle, j_list2);
			j1 = getDeepestLevelJoint(j_list1); j2 = getDeepestLevelJoint(j_list2);
			if(j1 && j2)
			{
				//gsout << "  Toe_End joints: " << j1->name() <<gspc<< j2->name() << gsnl;
				if(getJointHierarchyLevel(j1)-getJointHierarchyLevel(l_ankle)>2)
					guessLeftRightFromJntNames(j1->parent(), j2->parent(), l_forefoot_toe, r_forefoot_toe);
				else
					guessLeftRightFromJntNames(j1, j2, l_forefoot_toe, r_forefoot_toe);
				setJointMap("l_forefoot_toe", l_forefoot_toe);
				setJointMap("r_forefoot_toe", r_forefoot_toe);
			}
		}
		else // l/r_ankle not found, try make deepest joint as toe then make its parent as ankle
		{	
			std::vector<SkJoint*> j_list1, j_list2;
			listChildrenJoints(l_hip_upleg, j_list1); listChildrenJoints(r_hip_upleg, j_list2);
			j1 = getDeepestLevelJoint(j_list1); j2 = getDeepestLevelJoint(j_list2);
			if(j1 && j2)
			{
				//gsout << "  Toe_End joints: " << j1->name() <<gspc<< j2->name() << gsnl;
				if(getJointHierarchyLevel(j1)-getJointHierarchyLevel(l_hip_upleg)>3)
					guessLeftRightFromJntNames(j1->parent(), j2->parent(), l_forefoot_toe, r_forefoot_toe);
				else
					guessLeftRightFromJntNames(j1, j2, l_forefoot_toe, r_forefoot_toe);
				setJointMap("l_forefoot_toe", l_forefoot_toe);
				setJointMap("r_forefoot_toe", r_forefoot_toe);

				// try finding foot(ankle) joints
				ja = j1->parent()->parent();
				jb = j2->parent()->parent();

				// guess left/right ankle from the names
				guessLeftRightFromJntNames(ja, jb, l_ankle, r_ankle);

				setJointMap("l_ankle", l_ankle);
				setJointMap("r_ankle", r_ankle);
			}
		}

		if(!l_ankle || !r_ankle)
		{
			LOG("guessMap: ankle/toe_end joints NOT found, aborting...\n");
			return false;
		}
	}

	//-------------------------------------------------------------------------
	// CONTINUE to find l/r_knee using l/r_hip_upleg and l/r_ankle
	{
		if(!(l_ankle&&l_hip_upleg))
		{
			LOG("guessMap: l_ankle or l_hip_upleg NOT found, abort finding l_knee...\n");
			return false;
		}

		if(getJointHierarchyLevel(l_ankle) - getJointHierarchyLevel(l_hip_upleg) == 2)
		{
			l_knee = l_ankle->parent();
			r_knee = r_ankle->parent();
			setJointMap("l_knee", l_knee);
			setJointMap("r_knee", r_knee);
		}
		if(getJointHierarchyLevel(l_ankle) - getJointHierarchyLevel(l_hip_upleg) == 4)
		{ // leg has two twist joints
			l_knee = l_ankle->parent()->parent();
			r_knee = r_ankle->parent()->parent();
			setJointMap("l_knee", l_knee);
			setJointMap("r_knee", r_knee);
		}
	}

	//-------------------------------------------------------------------------
	// CONTINUE to guess 5 fingers names (must have all 5 fingers to proceed here!)
	// l_thumb1~4 (LeftHandThumb1~4)
	// l_index1~4 (LeftHandIndex1~4)
	// l_middle1~4 (LeftHandMiddle1~4)
	// l_ring1~4 (LeftHandRing1~4)
	// l_pinky1~4 (LeftHandPinky1~4)
	{
		if(!l_wrist || !r_wrist)
		{
			LOG("guessMap: l/r_wrist NOT found, abort finding fingers...\n");
			return false;
		}
		if(l_wrist->num_children()!=5 || r_wrist->num_children()!=5)
		{
			LOG("guessMap: l/r_wrist must have 5 children joints to proceed, abort finding fingers...\n");
			return false;
		}


		for(unsigned int i=0; i<5; i++)
		{
			SkJoint* ja = l_wrist->child(i);
			SkJoint* jb = r_wrist->child(i);
			SrString janame(ja->name().c_str());
			SrString jbname(jb->name().c_str());
			// name search, alternative names from wiki
			if(!l_thumb1 && (janame.search("thumb")>=0||janame.search("pollex")>=0||janame.search("finger0")>=0))
				l_thumb1 = ja;
			if(!l_index1 && (janame.search("index")>=0||janame.search("pointer")>=0||janame.search("forefinger")>=0))
				l_index1 = ja;
			if(!l_middle1 && (janame.search("middle")>=0||janame.search("medius")>=0||janame.search("mid")>=0))
				l_middle1 = ja;
			if(!l_ring1 && (janame.search("ring")>=0||janame.search("fourth")>=0||janame.search("finger4")>=0))
				l_ring1 = ja;
			if(!l_pinky1 && (janame.search("pinky")>=0||janame.search("little")>=0||janame.search("finger5")>=0))
				l_pinky1 = ja;

			if(!r_thumb1 && (jbname.search("thumb")>=0||jbname.search("pollex")>=0||jbname.search("finger0")>=0))
				r_thumb1 = jb;
			if(!r_index1 && (jbname.search("index")>=0||jbname.search("pointer")>=0||jbname.search("forefinger")>=0))
				r_index1 = jb;
			if(!r_middle1 && (jbname.search("middle")>=0||jbname.search("medius")>=0||jbname.search("mid")>=0))
				r_middle1 = jb;
			if(!r_ring1 && (jbname.search("ring")>=0||jbname.search("fourth")>=0||janame.search("finger4")>=0))
				r_ring1 = jb;
			if(!r_pinky1 && (jbname.search("pinky")>=0||jbname.search("little")>=0||janame.search("finger5")>=0))
				r_pinky1 = jb;
		}
		//setJointMap("l_thumb1", l_thumb1);
		//setJointMap("l_index1", l_index1);
		//setJointMap("l_middle1", l_middle1);
		//setJointMap("l_ring1", l_ring1);
		//setJointMap("l_pinky1", l_pinky1);

		//setJointMap("r_thumb1", r_thumb1);
		//setJointMap("r_index1", r_index1);
		//setJointMap("r_middle1", r_middle1);
		//setJointMap("r_ring1", r_ring1);
		//setJointMap("r_pinky1", r_pinky1);

		// left hand finger children
		if(l_thumb1 && l_thumb1->num_children()>0)
		{
			l_thumb2 = l_thumb1->child(0);
			if(l_thumb2 && l_thumb2->num_children()>0)
			{
				l_thumb3 = l_thumb2->child(0);
				if(l_thumb3 && l_thumb3->num_children()>0)
				{
					l_thumb4 = l_thumb3->child(0);
				}
			}
		}
		if(l_index1 && l_index1->num_children()>0)
		{
			l_index2 = l_index1->child(0);
			if(l_index2 && l_index2->num_children()>0)
			{
				l_index3 = l_index2->child(0);
				if(l_index3 && l_index3->num_children()>0)
				{
					l_index4 = l_index3->child(0);
				}
			}
		}
		if(l_middle1 && l_middle1->num_children()>0)
		{
			l_middle2 = l_middle1->child(0);
			if(l_middle2 && l_middle2->num_children()>0)
			{
				l_middle3 = l_middle2->child(0);
				if(l_middle3 && l_middle3->num_children()>0)
				{
					l_middle4 = l_middle3->child(0);
				}
			}
		}
		if(l_ring1 && l_ring1->num_children()>0)
		{
			l_ring2 = l_ring1->child(0);
			if(l_ring2 && l_ring2->num_children()>0)
			{
				l_ring3 = l_ring2->child(0);
				if(l_ring3 && l_ring3->num_children()>0)
				{
					l_ring4 = l_ring3->child(0);
				}
			}
		}
		if(l_pinky1 && l_pinky1->num_children()>0)
		{
			l_pinky2 = l_pinky1->child(0);
			if(l_pinky2 && l_pinky2->num_children()>0)
			{
				l_pinky3 = l_pinky2->child(0);
				if(l_pinky3 && l_pinky3->num_children()>0)
				{
					l_pinky4 = l_pinky3->child(0);
				}
			}
		}
		
		// right hand finger children
		if(r_thumb1 && r_thumb1->num_children()>0)
		{
			r_thumb2 = r_thumb1->child(0);
			if(r_thumb2 && r_thumb2->num_children()>0)
			{
				r_thumb3 = r_thumb2->child(0);
				if(r_thumb3 && r_thumb3->num_children()>0)
				{
					r_thumb4 = r_thumb3->child(0);
				}
			}
		}
		if(r_index1 && r_index1->num_children()>0)
		{
			r_index2 = r_index1->child(0);
			if(r_index2 && r_index2->num_children()>0)
			{
				r_index3 = r_index2->child(0);
				if(r_index3 && r_index3->num_children()>0)
				{
					r_index4 = r_index3->child(0);
				}
			}
		}
		if(r_middle1 && r_middle1->num_children()>0)
		{
			r_middle2 = r_middle1->child(0);
			if(r_middle2 && r_middle2->num_children()>0)
			{
				r_middle3 = r_middle2->child(0);
				if(r_middle3 && r_middle3->num_children()>0)
				{
					r_middle4 = r_middle3->child(0);
				}
			}
		}
		if(r_ring1 && r_ring1->num_children()>0)
		{
			r_ring2 = r_ring1->child(0);
			if(r_ring2 && r_ring2->num_children()>0)
			{
				r_ring3 = r_ring2->child(0);
				if(r_ring3 && r_ring3->num_children()>0)
				{
					r_ring4 = r_ring3->child(0);
				}
			}
		}
		if(r_pinky1 && r_pinky1->num_children()>0)
		{
			r_pinky2 = r_pinky1->child(0);
			if(r_pinky2 && r_pinky2->num_children()>0)
			{
				r_pinky3 = r_pinky2->child(0);
				if(r_pinky3 && r_pinky3->num_children()>0)
				{
					r_pinky4 = r_pinky3->child(0);
				}
			}
		}

		setJointMap("l_thumb1", l_thumb1);	setJointMap("l_thumb2", l_thumb2);	setJointMap("l_thumb3", l_thumb3);	if(l_thumb4) setJointMap("l_thumb4", l_thumb4);
		setJointMap("l_index1", l_index1);	setJointMap("l_index2", l_index2);	setJointMap("l_index3", l_index3);	if(l_index4) setJointMap("l_index4", l_index4);
		setJointMap("l_middle1", l_middle1);setJointMap("l_middle2", l_middle2);setJointMap("l_middle3", l_middle3);if(l_middle4) setJointMap("l_middle4", l_middle4);
		setJointMap("l_ring1", l_ring1);	setJointMap("l_ring2", l_ring2);	setJointMap("l_ring3", l_ring3);	if(l_ring4) setJointMap("l_ring4", l_ring4);
		setJointMap("l_pinky1", l_pinky1);	setJointMap("l_pinky2", l_pinky2);	setJointMap("l_pinky3", l_pinky3);	if(l_pinky4) setJointMap("l_pinky4", l_pinky4);

		setJointMap("r_thumb1", r_thumb1);	setJointMap("r_thumb2", r_thumb2);	setJointMap("r_thumb3", r_thumb3);	if(r_thumb4) setJointMap("r_thumb4", r_thumb4);
		setJointMap("r_index1", r_index1);	setJointMap("r_index2", r_index2);	setJointMap("r_index3", r_index3);	if(r_index4) setJointMap("r_index4", r_index4);
		setJointMap("r_middle1", r_middle1);setJointMap("r_middle2", r_middle2);setJointMap("r_middle3", r_middle3);if(r_middle4) setJointMap("r_middle4", r_middle4);
		setJointMap("r_ring1", r_ring1);	setJointMap("r_ring2", r_ring2);	setJointMap("r_ring3", r_ring3);	if(r_ring4) setJointMap("r_ring4", r_ring4);
		setJointMap("r_pinky1", r_pinky1);	setJointMap("r_pinky2", r_pinky2);	setJointMap("r_pinky3", r_pinky3);	if(r_pinky4) setJointMap("r_pinky4", r_pinky4);

	}	
	
	return true;
}

// get joint index from joint array (linear search)
int SBJointMap::getJointIndex(SkJoint* j)
{
	if(j==0)
		return -1; // not found

	SkSkeleton* sk = j->skeleton();
	if(sk==0)
		return -1; // not found

	for(unsigned int i=0; i<sk->joints().size(); i++)
	{
		if(sk->joints()[i]==j) // found
		{
			return i;
			break;
		}
	}
	return -1; // not found
}

// get joint hierachy level (how many levels below j_top), use root() as j_top if not specified
int SBJointMap::getJointHierarchyLevel(SkJoint* j, SkJoint* j_top)
{
	if(!j)
		return -1;

	if(j_top==0)
		j_top = j->skeleton()->root();

	int level = 0;
	while(j != j_top)
	{
		level ++;
		j = j->parent();
	}
	return level;
}

// (recursive) count all num_children() joints below given joint in the hierachy
int SBJointMap::countChildren(SkJoint* j)
{
	int count = 0;
	for(int i=0; i<j->num_children(); i++)
	{
		count += 1 + countChildren(j->child(i));
	}
	return count;
}


// count given char in string
int SBJointMap::countChar(const char* string, char c, bool isCaseSensitive)
{
	int count = 0;
	unsigned int len = strlen(string);

	if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z'))) // check if NOT letter
		isCaseSensitive = false; // not letter -> not case sensitive

	if(isCaseSensitive)
	{
		for(unsigned int i=0; i<len; i++)
		{
			if(string[i] == c)
				count ++;
		}
	}
	else
	{
		if(c >= 'A' && c <= 'Z')
			c = c - ('A'-'a'); // change to lower case

		for(unsigned int i=0; i<len; i++)
		{
			if(string[i] == c)
				count ++;
			if(string[i] == c+('A'-'a'))
				count ++;
		}
	}
	return count;
}


// guess which joint is left/right by counting letters in joint names (ja, jb)
void SBJointMap::guessLeftRightFromJntNames(SkJoint* ja, SkJoint* jb,
											  SkJoint*& l_j, SkJoint*& r_j)
{
	if(countChar(ja->name().c_str(), 'l', false) > countChar(jb->name().c_str(), 'l', false))
	{
		l_j = ja; r_j = jb;
	}
	else if(countChar(ja->name().c_str(), 'l', false) < countChar(jb->name().c_str(), 'l', false))
	{
		l_j = jb; r_j = ja;
	}
	else if(countChar(ja->name().c_str(), 'r', false) < countChar(jb->name().c_str(), 'r', false))
	{
		l_j = ja; r_j = jb;
	}
	else if(countChar(ja->name().c_str(), 'r', false) > countChar(jb->name().c_str(), 'r', false))
	{
		l_j = jb; r_j = ja;
	}
	else // can NOT figure out which is left/right from name (letter counting)
	{
		l_j = ja; r_j = jb; // 50% chance wrong
		LOG("guessMap: 50% chance wrong: %s <=> %s \n", ja->name().c_str(), jb->name().c_str());
	}
}

// push all num_children() joints into given list, make sure list is empty! (recursive)
void SBJointMap::listChildrenJoints(SkJoint* j, std::vector<SkJoint*>& j_list)
{
	if(!j) return;

	j_list.push_back(j);
	for(int i=0; i<j->num_children(); i++)
	{
		listChildrenJoints(j->child(i), j_list);
	}
}

SkJoint* SBJointMap::getDeepestLevelJoint(const std::vector<SkJoint*>& j_list)
{
	unsigned int size = j_list.size();
	if(size <= 1) return 0;

	SkJoint* return_j = j_list[0];
	int max_level = getJointHierarchyLevel(return_j);
	for(unsigned int i=1; i<size; i++)
	{
		int cur_level = getJointHierarchyLevel(j_list[i]);
		if(cur_level > max_level)
		{
			max_level = cur_level;
			return_j = j_list[i];
		}
	}
	return return_j;
}

void SBJointMap::setJointMap(const char* SB_jnt, SkJoint* j)
{
	if(j==0)
	{
		LOG("WARNING: joint not found! %s \n", SB_jnt);
		return;
	}
	//printf("%-15s <=> %-20s, %d chd(s), level %d \n",
	//	SB_jnt, j->name().c_str(),	j->num_children(), getJointHierarchyLevel(j));

	setMapping(j->name().c_str(), SB_jnt); // set the mapping here

	LOG("%-15s <=> %-20s, %d chd(s), level %d \n",
		SB_jnt, j->name().c_str(),	j->num_children(), getJointHierarchyLevel(j));
}


} // namespace SmartBody 

