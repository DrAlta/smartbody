/*
 *  me_ct_new_locomotion.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2011  University of Southern California
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
 *      Yuyu Xu, Alain Juarez Perez ICT USC
 */
#include <assert.h>
#include <boost/foreach.hpp>
#include "controllers/me_ct_new_locomotion.h"
#include <controllers/me_ct_scheduler2.h>
#include <sb/SBScene.h>


std::string MeCtNewLocomotion::_type_name = "NewLocomotion";

MeCtNewLocomotion::MeCtNewLocomotion(SbmCharacter* c) :  SmartBody::SBController(), 
															 character(c)
{
	scootSpd = 0.0f;	//	unit: centermeter/sec
	movingSpd = 0.0f;	//	unit: centermeter/sec
	turningSpd = 0.0f;	//	unit: deg/sec
	_valid = false;
	_lastTime = -2.0;
	C  = SmartBody::SBScene::getScene()->getMotion("ChrBrad@Walk01B");
	S = C->smoothCycle("", 0.5f);
	sk=new SmartBody::SBSkeleton(SmartBody::SBScene::getScene()->getSkeleton("ChrBrad.sk"));
	LeftFading.prev_time = -1.0f;
	RightFading.prev_time = -1.0f;
	_duration = -1.0f;
	useIKRt=useIKLf=false;
}

MeCtNewLocomotion::~MeCtNewLocomotion()
{
}

void MeCtNewLocomotion::init(SbmCharacter* sbChar)
{
	_lastTime = -1.0;
		//assert(_skeleton);	
	// root is "world_offset", so we use root->child to get the base joint.
	SmartBody::SBJoint* rootJoint = dynamic_cast<SmartBody::SBJoint*>(sk->root()->child(0));//_skeleton->getJointByName(rootJointName);//_skeleton->root()->child(0);//_skeleton->search_joint("l_acromioclavicular");//_skeleton->root()->child(0);//_skeleton->search_joint("l_acromioclavicular");//_skeleton->root()->child(0);//_skeleton->search_joint("base"); // test for now
	character = sbChar;
	std::vector<std::string> stopJoints;
	stopJoints.push_back("JtHeelLf");
	stopJoints.push_back("JtHeelRt");
	ik_scenario.buildIKTreeFromJointRoot(rootJoint,stopJoints);
	const IKTreeNodeList& nodeList = ik_scenario.ikTreeNodes;		
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		SkJoint* joint = nodeList[i]->joint;
		_channels.add(joint->getMappedJointName(), SkChannel::Quat);	
	}	

	double ikReachRegion = character->getHeight()*0.02f;		
	ikDamp        = ikReachRegion*ikReachRegion*14.0;
	MeController::init(sbChar);
	ik.dampJ = ikDamp;
	ik.refDampRatio = 0.01;
}

bool MeCtNewLocomotion::controller_evaluate(double t, MeFrameData& frame)
{
	float Dt = 1.0f / 60.0f;
	if (_lastTime > 0.0)
		Dt = float(t - _lastTime);
	_lastTime = t;
	motionTime=fmod((float)t, S->duration());
	if (character && _valid)
	{
		//if (scootSpd == 0.0f && movingSpd == 0.0f && turningSpd == 0.0f)
			//return true;
		//*/

		float x, y, z, yaw, pitch, roll;
		character->get_world_offset(x, y, z, yaw, pitch, roll);

		yaw = desiredHeading;
		float movingDist = movingSpd * Dt;
		x += movingDist * sin(yaw * (float)M_PI / 180.0f);
		z += movingDist * cos(yaw * (float)M_PI / 180.0f);
		//*/
		std::vector<SrQuat> tempQuatList(ik_scenario.ikTreeNodes.size()); 
		if (LeftFading.prev_time == -1.0f && RightFading.prev_time ==-1.0f) // first start
		{		
			// for first frame, update from frame buffer to joint quat in the limb
			// any future IK solving will simply use the joint quat from the previous frame.
			
			character->set_world_offset(x, y, z, yaw, pitch, roll);
			updateChannelBuffer(frame);//Read from skeleton->Write on Buffer
			updateChannelBuffer(frame,tempQuatList, true);//Read from Buffer->Write on tempQuatList	

			ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_INIT);
			ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_PREVREF);
			ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_CUR);
		}

		LeftFading.updateDt((float)t);
		RightFading.updateDt((float)t);

		character->set_world_offset(x, y, z, yaw, pitch, roll);
		updateChannelBuffer(frame);//Read from skeleton->Write on Buffer
		updateChannelBuffer(frame,tempQuatList, true);//Read from Buffer->Write on tempQuatList

		updateConstraints(motionTime/(float)S->getFrameRate());
		
		ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_REF);	
		character->getSkeleton()->update_global_matrices();
		ik_scenario.ikGlobalMat=character->getSkeleton()->getJointByName("JtPelvis")->gmat();
		//*/
		
		//*/Character pose

		ik.setDt(LeftFading.dt);
		if (LeftFading.fadeMode == Fading::FADING_MODE_IN)
			useIKLf = true;
		if (RightFading.fadeMode == Fading::FADING_MODE_IN)
			useIKRt=true;
		
		if (LeftFading.updateFading(LeftFading.dt))
			useIKLf = false;
		if( RightFading.updateFading(RightFading.dt))
			useIKRt = false;
		//if (useIKLf || useIKRt)	
		{		

			//*/Right
			ik_scenario.ikPosEffectors = &posConsRt;
			ik_scenario.ikRotEffectors = &rotConsRt;
			ik.update(&ik_scenario);
			ik_scenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);
			//ik_scenario.getTreeNodeQuat(tempQuatList,QUAT_CUR);			
			for (unsigned int i=7; i<13 ;i++)
			{
				MeCtIKTreeNode* node = ik_scenario.ikTreeNodes[i];
				SrQuat qEval = node->getQuat(QUAT_CUR);
				SrQuat qInit = node->getQuat(QUAT_REF);
				qEval.normalize();
				qInit.normalize();
				tempQuatList[i] = slerp(qInit,qEval,RightFading.blendWeight);
			}
			//*/
			std::vector<SrQuat> tempQuatList2(tempQuatList);
			//*/Left
			ik_scenario.ikPosEffectors = &posConsLf;
			ik_scenario.ikRotEffectors = &rotConsLf;
			ik.update(&ik_scenario);
			ik_scenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);
			//ik_scenario.getTreeNodeQuat(tempQuatList,QUAT_CUR);			
			for (unsigned int i=1; i<7 ;i++)
			{
				MeCtIKTreeNode* node = ik_scenario.ikTreeNodes[i];
				SrQuat qEval = node->getQuat(QUAT_CUR);
				SrQuat qInit = node->getQuat(QUAT_REF);
				qEval.normalize();
				qInit.normalize();
				tempQuatList[i] = slerp(qInit,qEval,LeftFading.blendWeight);
			}	
		}
		updateChannelBuffer(frame,tempQuatList);//Read from tempQuatList->Write on Buffer
	}
	RightFading.prev_time=LeftFading.prev_time = (float)t;
	return true;
}

void MeCtNewLocomotion::updateChannelBuffer(MeFrameData& frame, std::vector<SrQuat>& quatList, bool bRead)
{
	static bool printOnce=true;
	SrBuffer<float>& buffer=frame.buffer();
	int count=0;	
	const IKTreeNodeList& nodeList = ik_scenario.ikTreeNodes;
	BOOST_FOREACH(SrQuat& quat, quatList)
	{
		int chanId;
		int index;
		SkJoint* joint = nodeList[count++]->joint;
		chanId = _context->channels().search(joint->getMappedJointName(), SkChannel::Quat);
		if (chanId < 0)
			continue;

		index = _context->toBufferIndex(chanId);
		if (index < 0 )
		{
			if (bRead)
			{
				quat = SrQuat();
			}
		}
		else
		{
			if (bRead)
			{
				quat.w = buffer[index] ;
				quat.x = buffer[index + 1] ;
				quat.y = buffer[index + 2] ;
				quat.z = buffer[index + 3] ;			
			}
			else
			{
				buffer[index] = quat.w;
				buffer[index + 1] = quat.x;
				buffer[index + 2] = quat.y;
				buffer[index + 3] = quat.z;
			}
		}			
	}
	printOnce=false;
}

void MeCtNewLocomotion::updateChannelBuffer(MeFrameData& frame)
{
	SrBuffer<float>& buffer=frame.buffer();
	S->connect(sk);
	S->apply(motionTime);
	for(int i=0; i<sk->getNumJoints(); i++)
	{
			SmartBody::SBJoint* joint=sk->getJoint(i);	
			int chanId = _context->channels().search(joint->getMappedJointName(), SkChannel::Quat);
			if (chanId < 0)
				continue;
			int index = _context->toBufferIndex(chanId);
			SrQuat quat= joint->quat()->rawValue();
			if(index<0)
				continue;
			buffer[index + 0] = quat.w;
			buffer[index + 1] = quat.x;
			buffer[index + 2] = quat.y;
			buffer[index + 3] = quat.z;
	}
	S->disconnect();
}

bool MeCtNewLocomotion::addEffectorJointPair( const char* effectorName, const char* effectorRootName, const SrVec& pos, const SrQuat& rot, 
	                                          ConstraintType cType, ConstraintMap& posCons, ConstraintMap& rotCons)
{
	MeCtIKTreeNode* node = ik_scenario.findIKTreeNode(effectorName);
	MeCtIKTreeNode* rootNode = ik_scenario.findIKTreeNode(effectorRootName);	
	if (!node)
		return false;

	std::string rootName = effectorRootName;

	if (!rootNode)
		rootName = ik_scenario.ikTreeRoot->getNodeName();

	// separate position & rotation constraint
	//ConstraintList& jEffectorList = (cType == CONSTRAINT_ROT) ? rotConstraint : posConstraint;
	ConstraintMap& jEffectorMap = (cType == CONSTRAINT_ROT) ? rotCons : posCons;
	//VecOfString& effectorList  = (cType == CONSTRAINT_ROT) ? ik_scenario.ikRotEffectors : ik_scenario.ikPosEffectors;

	std::string str = effectorName;
	//assert(jEffectorList.size() == effectorList.size());
	//std::find()

	//int idx = distance(effectorList.begin(), find(effectorList.begin(),effectorList.end(),str));
	//int idx = //MeCtIKTreeScenario::findIKTreeNodeInList(effectorName,effectorList);
	ConstraintMap::iterator ci = jEffectorMap.find(str);
	if (ci != jEffectorMap.end())//idx != effectorList.size())
	{
		//jEffectorList[idx].targetJoint = targetJoint;	
		//EffectorJointConstraint& cons = jEffectorList[idx];
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>((*ci).second);
		cons->rootName = rootName;//effectorRootName;
		cons->targetPos = pos;
		cons->targetRot = rot;
		
	}
	else // add effector-joint pair
	{
		// initialize constraint
		EffectorConstantConstraint* cons = new EffectorConstantConstraint();
		//constraint.node = node;
		cons->efffectorName = effectorName;
		cons->rootName = rootName;//effectorRootName;
		cons->targetPos = pos;
		cons->targetRot = rot;
		jEffectorMap[str] = cons;
		//effectorList.push_back(effectorName);
		//jEffectorList.push_back(cons);		
	}
	return true;
}
void MeCtNewLocomotion::controller_map_updated() 
{		
	
}

void MeCtNewLocomotion::controller_start()
{
	LeftFading.controlRestart();
	RightFading.controlRestart();
}

void MeCtNewLocomotion::addPawn(SrVec& pos, std::string name)
{
	if(SmartBody::SBScene::getScene()->getPawn(name) == NULL)
		SmartBody::SBScene::getScene()->createPawn(name);
	SmartBody::SBPawn* pawn=SmartBody::SBScene::getScene()->getPawn(name);
	pawn->setPosition(pos);
	pawn->setStringAttribute("collisionShape","sphere");
	pawn->setVec3Attribute("collisionShapeScale",0.1f,0.1f,0.1f);
}

void MeCtNewLocomotion::updateConstraints(float t)
{
	const float fadein=0.1f;
	const float fadeout=0.5f;
	if(useIKRt)
	{
		if(t>rplant[1] && t<rplant[2] && RightFading.fadeMode==Fading::FADING_MODE_OFF )
		{
			LOG("of R %f  %f", t, RightFading.blendWeight);
			RightFading.setFadeOut(fadeout);
		}
	}
	else if(t>rplant[2] && t<rplant[3] && RightFading.fadeMode==Fading::FADING_MODE_OFF)
	{	
		SmartBody::SBSkeleton *sk2=character->getSkeleton();
		sk2->update_global_matrices();
		ik_scenario.ikGlobalMat=sk2->root()->gmat();
		LOG("on R %f  %f", t, RightFading.blendWeight);;
		SmartBody::SBJoint* joint=sk2->getJointByName("JtHeelRt");	
		SrVec tv = joint->gmat().get_translation();
		SrQuat tq = SrQuat(joint->gmat());
		ConstraintType cType = CONSTRAINT_POS;
		addEffectorJointPair("JtHeelRt", "JtHipRt" ,tv, tq, cType, posConsRt, rotConsRt);
		cType = CONSTRAINT_ROT;
		addEffectorJointPair("JtHeelRt", "JtHipRt" ,tv, tq, cType, posConsRt, rotConsRt);
		RightFading.setFadeIn(fadein);
		//addPawn(tv, "goal");
	}
	if(useIKLf)
	{
		if(t>lplant[1] && LeftFading.fadeMode==Fading::FADING_MODE_OFF )
		{
			LOG("of L %f  %f", t, LeftFading.blendWeight);
			LeftFading.setFadeOut(fadeout);
		}
	}
	else if(t>lplant[0] && t<lplant[1] && LeftFading.fadeMode==Fading::FADING_MODE_OFF)
	{		
		SmartBody::SBSkeleton *sk2=character->getSkeleton();
		sk2->update_global_matrices();
		ik_scenario.ikGlobalMat=sk2->root()->gmat();
		LOG("on L %f  %f", t, LeftFading.blendWeight);
		SmartBody::SBJoint* joint=sk2->getJointByName("JtHeelLf");	
		SrVec tv = joint->gmat().get_translation();
		SrQuat tq = SrQuat(joint->gmat());
		ConstraintType cType = CONSTRAINT_POS;
		addEffectorJointPair("JtHeelLf", "JtHipLf" ,tv, tq, cType, posConsLf, rotConsLf);
		cType = CONSTRAINT_ROT;
		addEffectorJointPair("JtHeelLf", "JtHipLf" ,tv, tq, cType, posConsLf, rotConsLf);
		LeftFading.setFadeIn(fadein);
		//addPawn(tv, "goal");
	}
}

Fading::Fading()
{
	fadeMode = FADING_MODE_OFF;
	blendWeight = 0.0;
	prev_time = 0.0;
	restart = false;
}

bool Fading::updateFading( float dt )
{
	const float FADE_EPSILON = 0.001f;
	bool finishFadeOut = false;
	if (fadeMode)
	{
		fadeRemainTime -= dt;
		if (fadeRemainTime <= 0.0)
			fadeRemainTime = 0.0;

		if (fadeMode == FADING_MODE_IN)
		{			
			float fadeNormal = 1.f - (float)fadeRemainTime/fadeInterval;
			blendWeight = fadeNormal;
			if (blendWeight > 1.0 - FADE_EPSILON)
			{
				blendWeight = 1.0;
				fadeMode = FADING_MODE_OFF;
			}						
		}
		else
		{
			float fadeNormal = fadeRemainTime/fadeInterval;
			blendWeight = fadeNormal;
			if (blendWeight < FADE_EPSILON)
			{
				blendWeight = 0.0;
				fadeMode = FADING_MODE_OFF;
				finishFadeOut = true;
			}	
		}
	}
	return finishFadeOut;
}


void Fading::setFadeIn( float interval )
{
	if (blendWeight == 1.0 && fadeMode == FADING_MODE_OFF)
		return;

	fadeInterval = interval;
	fadeRemainTime = interval;
	fadeMode = FADING_MODE_IN;
}

void Fading::setFadeOut( float interval )
{
	if (blendWeight == 0.0 && fadeMode == FADING_MODE_OFF)
		return;

	fadeInterval = interval;
	fadeRemainTime = interval;
	fadeMode = FADING_MODE_OUT;
}

void Fading::controlRestart()
{
	restart = true;
}

void Fading::updateDt( float curTime )
{
	if (restart)
	{
		dt = 0.f;
		restart = false;
	}
	else
	{
		dt = curTime - prev_time;
	}	
	prev_time = curTime;
}
