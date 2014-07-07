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
#include <controllers/me_ct_param_animation_utilities.h>
#include <controllers/me_ct_scheduler2.h>
#include <sb/SBScene.h>
#include <sbm/gwiz_math.h>


std::string MeCtNewLocomotion::_type_name = "NewLocomotion";

MeCtNewLocomotion::MeCtNewLocomotion() :  SmartBody::SBController()
{
	scootSpd = 0.0f;	//	unit: centermeter/sec
	movingSpd = 0.0f;	//	unit: centermeter/sec
	turningSpd = 0.0f;	//	unit: deg/sec
	_valid = false;
	_lastTime = -2.0;
	startTime = -1.0;
	C=S=NULL;
	
	LeftFading.prev_time = -1.0f;
	RightFading.prev_time = -1.0f;
	_duration = -1.0f;
	useIKRt = false;
	useIKLf = false;

	setDefaultAttributeGroupPriority("EnhancedLocomotion", 600);

	addDefaultAttributeString("walkCycle", "ChrBrad@Walk01B", "EnhancedLocomotion");
	addDefaultAttributeString("LEndEffectorJoint", "l_forefoot", "EnhancedLocomotion");
	addDefaultAttributeString("REndEffectorJoint", "r_forefoot", "EnhancedLocomotion");
	addDefaultAttributeString("CenterHipJoint", "JtPelvis", "EnhancedLocomotion");
	addDefaultAttributeDouble("FadeIn", 0.2, "EnhancedLocomotion");
	addDefaultAttributeDouble("FadeOut", 0.4, "EnhancedLocomotion");
	addDefaultAttributeDouble("TurningRate", 15.0, "EnhancedLocomotion");
}

MeCtNewLocomotion::~MeCtNewLocomotion()
{
}

void MeCtNewLocomotion::init(SbmCharacter* sbChar)
{
	character = sbChar;

	attributes_names.push_back("walkCycle");
	attributes_names.push_back("CenterHipJoint");
	attributes_names.push_back("LEndEffectorJoint");
	attributes_names.push_back("REndEffectorJoint");
	attributes_names.push_back("FadeIn");
	attributes_names.push_back("FadeOut");
	for(unsigned int i = 0; i< attributes_names.size(); i++)
	{
		SmartBody::SBAttribute* a = character->getAttribute(attributes_names[i]);
		if (a)
			a->registerObserver(this);
	}
	tempBuffer.size(1000);
	setup();
}

void MeCtNewLocomotion::setup()
{
	bool SameMotion=false;
	if(C)//If there is a motion, check if is the same one
		SameMotion=C->getName()==character->getStringAttribute("walkCycle");
	if(!SameMotion)
		C  = SmartBody::SBScene::getScene()->getMotion(character->getStringAttribute("walkCycle"));
	if (!C)
		return;
	hipjoint = character->getStringAttribute("CenterHipJoint");
	lend = character->getStringAttribute("LEndEffectorJoint");
	rend = character->getStringAttribute("REndEffectorJoint");
	fadein  = (float)character->getDoubleAttribute("FadeIn");
	fadeout = (float)character->getDoubleAttribute("FadeOut");

	if(!SameMotion)
	{
		S = C->smoothCycle("", 0.5f);
		sk = new SmartBody::SBSkeleton(SmartBody::SBScene::getScene()->getSkeleton("ChrBrad.sk"));
		S->connect(sk);
		motionSpd = S->getJointSpeed(sk->getJointByName(hipjoint), (float)S->getTimeStart() , (float)S->getTimeStop())*0.75f;
		S->disconnect();

	}
	_lastTime = -1.0;
	SmartBody::SBJoint* rootJoint = dynamic_cast<SmartBody::SBJoint*>(sk->root()->child(0));//_skeleton->getJointByName(rootJointName);//_skeleton->root()->child(0);//_skeleton->search_joint("l_acromioclavicular");//_skeleton->root()->child(0);//_skeleton->search_joint("l_acromioclavicular");//_skeleton->root()->child(0);//_skeleton->search_joint("base"); // test for now
	
	std::vector<std::string> stopJoints;
	stopJoints.push_back(lend);
	stopJoints.push_back(rend);
	ik_scenario.buildIKTreeFromJointRoot(rootJoint,stopJoints);
	const IKTreeNodeList& nodeList = ik_scenario.ikTreeNodes;		

	double ikReachRegion = character->getHeight()*0.02f;		
	ikDamp = ikReachRegion*ikReachRegion*14.0;
	MeController::init(character);
	ik.dampJ = ikDamp;
	ik.refDampRatio = 0.01;
}

bool MeCtNewLocomotion::controller_evaluate(double t, MeFrameData& frame)
{
	if (!C)
		return true;
	BufferRef=&(frame.buffer());
	if (character && _valid)
	{
		play((float)t);
	}
	return true;
}

void MeCtNewLocomotion::loopMotion(float def)
{
	char pawnname[20];
	sprintf(pawnname, "pawn%.1f", def);
	startTime = 0.0f;
	float diff = 1.0f/30.0f;
	setDesiredHeading(0.0f);
	def = def/(float)S->frames();
	for(float t = 0; t < S->duration() + 0.01; t += diff)
	{
		setDesiredHeading(getDesiredHeading()+def);
		play(t, true);
	}
	float x, y, z, yaw, pitch, roll;		
 	character->get_world_offset(x, y, z, yaw, pitch, roll); 
	SrVec pos(x,y,z);
	//LOG("%s : %f %f %f",pawnname, x,y,z);
	LOG("%f   %f",legDistance(1) ,legDistance(0));
	addPawn(pos,pawnname);
	reset();
}

void MeCtNewLocomotion::play(float t, bool useTemp)
{
	if(useTemp)
		BufferRef = &tempBuffer;
	float Dt = 1.0f / 60.0f;
	if (_lastTime > 0.0)
		Dt = float(t - _lastTime);
	_lastTime = t;
	SrQuat woQuat;
	SrVec woPos;
	if(startTime < 0.0)
	{
		startTime = t;
	}
	motionTime = fmod(t-startTime, S->duration());
	float x, y, z, yaw, pitch, roll;		
 	character->get_world_offset(x, y, z, yaw, pitch, roll); 
 	yaw = desiredHeading;
 	float movingDist = motionSpd*0.85f * Dt;
 	x += movingDist * sin(yaw * (float)M_PI / 180.0f);
 	z += movingDist * cos(yaw * (float)M_PI / 180.0f);

	gwiz::quat_t q = gwiz::euler_t(pitch,yaw,roll);
	woQuat = SrQuat(q.wf() ,q.xf(),q.yf(),q.zf());
	woPos = SrVec(x,y,z); 		
	//*/
	std::vector<SrQuat> tempQuatList(ik_scenario.ikTreeNodes.size()); 
	if (fabs(LeftFading.prev_time + RightFading.prev_time + 2.0f) <0.001f ) // first start
	{					
		updateChannelBuffer(*BufferRef);//Read from skeleton->Write on Buffer
		updateWorldOffset(*BufferRef, woQuat, woPos);
		updateChannelBuffer(*BufferRef,tempQuatList, true);//Read from Buffer->Write on tempQuatList


		ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_INIT);
		ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_PREVREF);
		ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_CUR);
	}

	LeftFading.updateDt(t);
	RightFading.updateDt(t);

		
	updateChannelBuffer(*BufferRef);//Read from skeleton->Write on Buffer
	updateWorldOffset(*BufferRef, woQuat, woPos);
	updateChannelBuffer(*BufferRef,tempQuatList, true);//Read from Buffer->Write on tempQuatList

	updateConstraints(motionTime/(float)S->getFrameRate());
		
	ik_scenario.setTreeNodeQuat(tempQuatList,QUAT_REF);	
	character->getSkeleton()->update_global_matrices();
	ik_scenario.ikGlobalMat = character->getSkeleton()->getJointByName("JtPelvis")->gmat();
	//*/Character pose
	ik.setDt(LeftFading.dt);
	if (LeftFading.fadeMode == Fading::FADING_MODE_IN)
		useIKLf = true;
	if (RightFading.fadeMode == Fading::FADING_MODE_IN)
		useIKRt = true;
	if (LeftFading.updateFading(LeftFading.dt))
		useIKLf = false;
	if( RightFading.updateFading(RightFading.dt))
		useIKRt = false;
	//*/Left
	ik_scenario.ikPosEffectors = &posConsLf;
	ik_scenario.ikRotEffectors = &rotConsLf;
	ik.update(&ik_scenario);
	ik_scenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);			
	for (unsigned int i = 0; i < 7; i++)
	{
		MeCtIKTreeNode* node = ik_scenario.ikTreeNodes[i];
		SrQuat qEval = node->getQuat(QUAT_CUR);
		SrQuat qInit = node->getQuat(QUAT_REF);
		qEval.normalize();
		qInit.normalize();
		tempQuatList[i] = slerp(qInit,qEval,LeftFading.blendWeight);
	}	
	//*/
	//*/Right
	ik_scenario.ikPosEffectors = &posConsRt;
	ik_scenario.ikRotEffectors = &rotConsRt;
	ik.update(&ik_scenario);
	ik_scenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);	
	for (unsigned int i = 7; i < ik_scenario.ikTreeNodes.size(); i++)
	{
		MeCtIKTreeNode* node = ik_scenario.ikTreeNodes[i];
		SrQuat qEval = node->getQuat(QUAT_CUR);
		SrQuat qInit = node->getQuat(QUAT_REF);
		qEval.normalize();
		qInit.normalize();
		tempQuatList[i] = slerp(qInit,qEval,RightFading.blendWeight);
	}
	//*/
	updateChannelBuffer(*BufferRef,tempQuatList);//Read from tempQuatList->Write on Buffer
	updateWorldOffset(*BufferRef, woQuat, woPos);	
	character->set_world_offset(x, y, z, yaw, pitch, roll); 
	RightFading.prev_time=LeftFading.prev_time = t;
}

void MeCtNewLocomotion::updateChannelBuffer(SrBuffer<float>& buffer, std::vector<SrQuat>& quatList, bool bRead)
{
	int count = 0;	
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
}

void MeCtNewLocomotion::updateChannelBuffer(SrBuffer<float>& buffer)
{
	S->connect(sk);
	S->apply(motionTime);
	for(int i = 0; i < sk->getNumJoints(); i++)
	{
			SmartBody::SBJoint* joint = sk->getJoint(i);	
			int chanId = _context->channels().search(joint->getMappedJointName(), SkChannel::Quat);
			if (chanId < 0)
				continue;
			int index = _context->toBufferIndex(chanId);
			SrQuat quat = joint->quat()->rawValue();
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
	ConstraintMap& jEffectorMap = (cType == CONSTRAINT_ROT) ? rotCons : posCons;

	std::string str = effectorName;

	ConstraintMap::iterator ci = jEffectorMap.find(str);
	if (ci != jEffectorMap.end())
	{
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>((*ci).second);
		cons->rootName = rootName;
		cons->targetPos = pos;
		cons->targetRot = rot;
		
	}
	else // add effector-joint pair
	{
		// initialize constraint
		EffectorConstantConstraint* cons = new EffectorConstantConstraint();
		cons->efffectorName = effectorName;
		cons->rootName = rootName;
		cons->targetPos = pos;
		cons->targetRot = rot;
		jEffectorMap[str] = cons;
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
	
	SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn(name);
	if(pawn == NULL)
		pawn = SmartBody::SBScene::getScene()->createPawn(name);
	pawn->setPosition(pos);
	pawn->setStringAttribute("collisionShape","sphere");
	pawn->setVec3Attribute("collisionShapeScale",0.03f,0.03f,0.03f);
}

void MeCtNewLocomotion::updateConstraints(float t)
{
	if(useIKRt)
	{
		if(t > rplant[1] && t < rplant[2] && RightFading.fadeMode == Fading::FADING_MODE_OFF )
		{
			RightFading.setFadeOut(fadeout);
		}
	}
	else if(((t > rplant[2] && t < rplant[3] )|| t > rplant[0] && t < rplant[1])&& RightFading.fadeMode == Fading::FADING_MODE_OFF)
	{	
		SmartBody::SBSkeleton *sk2 = character->getSkeleton();
		sk2->update_global_matrices();
		ik_scenario.ikGlobalMat = sk2->root()->gmat();
		SmartBody::SBJoint* joint = sk2->getJointByName(rend);	
		SrVec tv = joint->gmat().get_translation();
		SrQuat tq = SrQuat(joint->gmat());
		tv.y=0.0f;
		ConstraintType cType = CONSTRAINT_ROT;
		addEffectorJointPair(rend.c_str(), hipjoint.c_str(), tv, tq, cType, posConsRt, rotConsRt);
		cType = CONSTRAINT_POS;
		addEffectorJointPair(rend.c_str(), hipjoint.c_str(), tv, tq, cType, posConsRt, rotConsRt);
		RightFading.setFadeIn(fadein);
	}
	if(useIKLf)
	{
		if(t > lplant[1] && LeftFading.fadeMode == Fading::FADING_MODE_OFF )
		{
			LeftFading.setFadeOut(fadeout);
		}
	}
	else if(t > lplant[0] && t < lplant[1] && LeftFading.fadeMode == Fading::FADING_MODE_OFF)
	{		
		SmartBody::SBSkeleton *sk2 = character->getSkeleton();
		sk2->update_global_matrices();
		ik_scenario.ikGlobalMat = sk2->root()->gmat();
		SmartBody::SBJoint* joint = sk2->getJointByName(lend);	
		SrVec tv = joint->gmat().get_translation();
		SrQuat tq = SrQuat(joint->gmat());
		tv.y=0.0f;
		ConstraintType cType = CONSTRAINT_ROT;
		addEffectorJointPair(lend.c_str(), hipjoint.c_str(), tv, tq, cType, posConsLf, rotConsLf);
		cType = CONSTRAINT_POS;
		addEffectorJointPair(lend.c_str(), hipjoint.c_str(), tv, tq, cType, posConsLf, rotConsLf);
		LeftFading.setFadeIn(fadein);
	}
}

void MeCtNewLocomotion::updateWorldOffset(SrBuffer<float>& buffer, SrQuat& rot, SrVec& pos )
{	
	JointChannelId baseChanID, baseBuffId;
	baseChanID.x = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::XPos);
	baseChanID.y = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::YPos);
	baseChanID.z = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::ZPos);
	baseChanID.q = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::Quat);

	baseBuffId.x = _context->toBufferIndex(baseChanID.x);
	baseBuffId.y = _context->toBufferIndex(baseChanID.y);
	baseBuffId.z = _context->toBufferIndex(baseChanID.z);	
	baseBuffId.q = _context->toBufferIndex(baseChanID.q);	

	buffer[baseBuffId.x] = pos[0];
	buffer[baseBuffId.y] = pos[1];
	buffer[baseBuffId.z] = pos[2];	
	for (int k = 0; k < 4; k++)
		buffer[baseBuffId.q + k] = rot.getData(k);
}


void  MeCtNewLocomotion::notify(SmartBody::SBSubject* subject)
{
	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (attribute)
	{
		const std::string& name = attribute->getName();
		bool check_attributes=false;
		for(unsigned int i = 0; i< attributes_names.size(); i++)
			if (name == attributes_names[i])
				check_attributes=true;
		if(check_attributes)
			setup();
	}
}

void MeCtNewLocomotion::reset()
{	
	scootSpd = 0.0f;
	movingSpd = 0.0f;
	turningSpd = 0.0f;
	_valid = false;
	startTime = -1.0;
	useIKRt = false;
	useIKLf = false;
	setup();	
	posConsRt.clear();
	rotConsRt.clear();
	posConsLf.clear();
	rotConsLf.clear();
	LeftFading.controlRestart();
	RightFading.controlRestart();
	character->set_world_offset(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f); 
}

float MeCtNewLocomotion::legDistance(bool Leftleg)
{
	std::vector<std::string> jointNames;
	std::vector<SrVec> jointPositions;
	(Leftleg)? jointNames.push_back("l_hip") : jointNames.push_back("r_hip");
	(Leftleg)? jointNames.push_back("l_forefoot") : jointNames.push_back("r_forefoot");
	sk->getJointPositions(jointNames, jointPositions);
	return (jointPositions[0] - jointPositions[1]).len();;
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
	fadeMode = FADING_MODE_OFF;
	blendWeight = 0.0;
	prev_time = 0.0;
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