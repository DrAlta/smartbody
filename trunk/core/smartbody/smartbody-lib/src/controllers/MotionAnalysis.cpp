#include "MotionAnalysis.h"
#include <sb/SBScene.h>
/************************************************************************/
/* Locomotion Leg Cycle                                                 */
/************************************************************************/


float LocomotionLegCycle::getNormalizedCycleTime( float motionTime )
{
	float warpMotionTime = motionTime;
	if (cycleStartTime > cycleEndTime && motionTime < cycleEndTime) // loop back to beginning
		warpMotionTime = motionEndTime + (motionTime-motionStartTime);
	float cycleTime = (warpMotionTime-stanceTime)/cycleDuration; // normalized cycle time
	return cycleTime;
}

SrVec LocomotionLegCycle::getStanceSupportPos( int idx )
{
	return stanceSupportPos[idx];
}

SrVec LocomotionLegCycle::getSupportPos( float motionTime, int idx )
{	
	float cycleTime = getNormalizedCycleTime(motionTime);
	int index = (int)(cycleTime*samples.size());
	float weight = cycleTime*samples.size() - index;

	SrVec supportPos = samples[index].supportPos[idx]*(1.f-weight) + samples[(index+1)%samples.size()].supportPos[idx]*weight;
	return supportPos;
}

float LocomotionLegCycle::getTransitionWeight( float motionTime, int& phase )
{
	// normalize the time to [0,1]
	float normalizedMotionTime = getNormalizedCycleTime(motionTime);
	float weight = 1.0;
	if (normalizedMotionTime >= normalizeStrikeTime && normalizedMotionTime <= normalizedLandTime) // transition to landing
	{ 
		weight = 1.0;		
		phase = 0;
	}
	else if (normalizedMotionTime >= normalizedLandTime || normalizedMotionTime <= normalizedLiftTime) // foot step
	{
		weight = 0.0;
		phase = 1;
	}
	else if (normalizedMotionTime >= normalizedLiftTime && normalizedMotionTime <= normalizeLiftoffTime) // transition to lifting
	{
		weight = (normalizedMotionTime-normalizedLiftTime)/(normalizeLiftoffTime-normalizedLiftTime);
		phase = 2;
	}
	else
	{
		weight = 1.0;	
		phase = 3;
	}
	return weight;
}

/************************************************************************/
/* Locomotion Analyzer                                                  */
/************************************************************************/
LocomotionAnalyzer::LocomotionAnalyzer()
{

}

LocomotionAnalyzer::~LocomotionAnalyzer()
{

}

LocomotionLegCycle* LocomotionAnalyzer::getLegCycle( int iLeg, float motionTime )
{
	LegCycleVec& legCycleVec = legCycleMap[iLeg];
	for (unsigned int i=0;i<legCycleVec.size();i++)
	{
		int iNext = (i+1)%legCycleVec.size();
		float s1 = legCycleVec[i].stanceTime;
		float s2 = legCycleVec[iNext].stanceTime;
		if (s1 > s2) // loop back to beginning
		{
			if (motionTime < s1 || motionTime >= s2)
				return &legCycleVec[i];
		}
		else // normal interval
		{
			if (motionTime >= s1 && motionTime < s2)
				return &legCycleVec[i];
		}
	}
	return NULL;
}

void LocomotionAnalyzer::initLegCycles( const std::string& name, SmartBody::SBAnimationBlend* locoBlend, KeyTagMap& keyTagMap, SmartBody::SBSkeleton* skelCopy )
{
	KeyTagMap::iterator mi;
	startTime = (float)locoBlend->getMotionKey(name, 0);
	endTime = (float)locoBlend->getMotionKey(name, locoBlend->getNumKeys()-1);
	int nSample = 50;
    motionName = name;
	SmartBody::SBMotion* sbMotion = SmartBody::SBScene::getScene()->getMotion(motionName);
	for ( mi  = keyTagMap.begin(); mi != keyTagMap.end(); mi++)
	{
		KeyTag& keyTag = mi->second;
		if (keyTag.size() == 0) continue;
		legCycleMap[mi->first] = LegCycleVec();
		LegCycleVec& legCycleVec = legCycleMap[mi->first];
		KeyTag::iterator ki = keyTag.begin();
		int nCycles = ki->second.size();
		for (int i=0;i<nCycles;i++)
		{
			legCycleVec.push_back(LocomotionLegCycle());
			LocomotionLegCycle& legCycle = legCycleVec[i];
			legCycle.stanceTime = (float)getKeyTagTime("stance",i,keyTag);
			legCycle.liftTime = (float)getKeyTagTime("lift",i,keyTag);
			legCycle.liftoffTime = (float)getKeyTagTime("liftoff",i,keyTag);
			legCycle.strikeTime = (float)getKeyTagTime("strike",i,keyTag);
			legCycle.landTime = (float)getKeyTagTime("land",i,keyTag);
			legCycle.motionStartTime = startTime;
			legCycle.motionEndTime = endTime;
		}
		for (int i=0;i<nCycles;i++) // compute cycle duration
		{
			LocomotionLegCycle& c1 = legCycleVec[i];
			LocomotionLegCycle& c2 = legCycleVec[(i+1)%nCycles];
			if (c2.stanceTime > c1.stanceTime)
			{
				c1.cycleDuration = c2.stanceTime - c1.stanceTime;				
			}
			else
			{
				c1.cycleDuration = (endTime-c2.stanceTime) + (c1.stanceTime - startTime);
			}
			// compute cycle time and normalized time
			c1.cycleStartTime = c1.stanceTime;
			c1.cycleEndTime = c2.stanceTime;

			c1.normlizedStanceTime = c1.getNormalizedCycleTime(c1.stanceTime);
			c1.normalizedLiftTime = c1.getNormalizedCycleTime(c1.liftTime);
			c1.normalizeLiftoffTime = c1.getNormalizedCycleTime(c1.liftoffTime);
			c1.normalizeStrikeTime = c1.getNormalizedCycleTime(c1.strikeTime);
			c1.normalizedLandTime = c1.getNormalizedCycleTime(c1.landTime);

			sampleLegCycle(legInfos[mi->first],c1, sbMotion, skelCopy, nSample);			
		}
	}
}

double LocomotionAnalyzer::getKeyTagTime( const std::string& key, int iCycle, KeyTag& tag )
{
	double value = -1.0;
	if (tag.find(key) != tag.end())
	{
		std::vector<double>& valueList = tag[key];
		if (iCycle >=0 && iCycle < (int)valueList.size())
		{
			value = valueList[iCycle];
		}
	}
	return value;
}

void LocomotionAnalyzer::sampleLegCycle(LegInfo* legInfo, LocomotionLegCycle& legCycle, SmartBody::SBMotion* motion, SmartBody::SBSkeleton* skel, int nSample )
{
	legCycle.samples.resize(nSample);
	
	SBJoint* baseJoint = skel->getJointByName(legInfo->base);
	motion->connect(skel);
	double dt = legCycle.cycleDuration/nSample;	
	for (int i=0;i<nSample;i++)
	{
		double motionTime = legCycle.stanceTime + dt*i;
		motion->apply((float)motionTime);		
		skel->update_global_matrices();
		LegCycleSample& legSample = legCycle.samples[i];
		legSample.supportPos.resize(legInfo->supportJoints.size());
		for (unsigned int sup = 0; sup < legInfo->supportJoints.size(); sup++)
		{
			SBJoint* supJoint = skel->getJointByName(legInfo->supportJoints[sup]);		
			legSample.supportPos[sup] = supJoint->gmat().get_translation()*baseJoint->gmat().inverse();
		}		
	}		
	motion->apply(legCycle.stanceTime);
	legCycle.stanceSupportPos.resize(legInfo->supportJoints.size());
	for (unsigned int sup = 0; sup < legInfo->supportJoints.size(); sup++)
	{
		SBJoint* supJoint = skel->getJointByName(legInfo->supportJoints[sup]);		
		legCycle.stanceSupportPos[sup] = supJoint->gmat().get_translation()*baseJoint->gmat().inverse();
	}		
	motion->disconnect();
}

/************************************************************************/
/* Motion Analysis                                                      */
/************************************************************************/

MotionAnalysis::MotionAnalysis(void)
{
}

MotionAnalysis::~MotionAnalysis(void)
{
}

void MotionAnalysis::init(std::string skeletonName, std::string baseJoint, SmartBody::SBAnimationBlend* locomotionBlend )
{		
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBSkeleton* sbSkel = scene->getSkeleton(skeletonName);
	skelCopy = new SmartBody::SBSkeleton(sbSkel);
	initLegInfos();
	int numMotions = locomotionBlend->getNumMotions();	
	for (int i=0;i<numMotions;i++)
	{
		const std::string& motionName = locomotionBlend->getMotionName(i);
		KeyTagMap& keyTagMap = *locomotionBlend->getKeyTagMap(motionName);
		LocomotionAnalyzer* analyzer = new LocomotionAnalyzer();
		analyzer->legInfos = legInfos;
		analyzer->initLegCycles(motionName,locomotionBlend,keyTagMap,skelCopy);
		locoAnalyzers.push_back(analyzer);
	}	
}

void MotionAnalysis::initLegInfos()
{
	const std::string lFootName[] = {"l_forefoot", "l_ankle" };
	const std::string rFootName[] = {"r_forefoot", "r_ankle" };

	LegInfo* lLeg = new LegInfo();	
	lLeg->supportJoints.resize(2);
	lLeg->supportJoints[1] = lFootName[1];
	lLeg->supportJoints[0] = lFootName[0];
	lLeg->base = "base";
	lLeg->hip = "l_hip";	

	LegInfo* rLeg = new LegInfo();
	rLeg->supportJoints.resize(2);
	rLeg->supportJoints[1] = rFootName[1];
	rLeg->supportJoints[0] = rFootName[0];
	rLeg->base = "base";
	rLeg->hip = "r_hip";

	legInfos.push_back(lLeg);
	legInfos.push_back(rLeg);
	legStates.resize(2);

	for (int i=0;i<2;i++)
	{
		EffectorConstantConstraint* lFoot = new EffectorConstantConstraint();
		lFoot->efffectorName = lFootName[i];
		lFoot->rootName = "";
		lLeg->ikConstraint[lFoot->efffectorName] = lFoot;

		EffectorConstantConstraint* rFoot = new EffectorConstantConstraint();
		rFoot->efffectorName = rFootName[i];
		rFoot->rootName = "";
		rLeg->ikConstraint[rFoot->efffectorName] = rFoot;

		legStates[i].curSupportPos.resize(2);
	}

	std::vector<std::string> stopJoint;
	stopJoint.push_back("spine1");
	ikScenario.buildIKTreeFromJointRoot(skelCopy->getJointByName("base"),stopJoint);
}

void MotionAnalysis::applyIKFix( std::vector<double>& weights, PATimeManager* timeManager, SrMat worldOffsetMat, BodyMotionFrame& inputFrame, BodyMotionFrame& outFrame )
{
	ikScenario.ikGlobalMat = worldOffsetMat;
	ikScenario.ikTreeRootPos = inputFrame.rootPos;
	ikScenario.setTreeNodeQuat(inputFrame.jointQuat, QUAT_INIT);
	std::string baseName = "base";	
	MeCtIKTreeNode* rootNode = ikScenario.findIKTreeNode(baseName.c_str());
	ikScenario.updateNodeGlobalMat(rootNode, QUAT_INIT);
	SrMat gmatBase = rootNode->gmat;
	for (unsigned int k=0;k<legStates.size();k++)
	{
		std::vector<SrVec>& supPos = legStates[k].curSupportPos;
		std::fill(supPos.begin(),supPos.end(),SrVec()); // clear the support position
	}
	float transitionWeight = 0.0;
	int maxWeightIndex = -1;
	double maxBlendWeight = -1;
	for (unsigned int i=0;i<locoAnalyzers.size();i++) // weighted sum of local foot base
	{
		LocomotionAnalyzer* analyzer = locoAnalyzers[i];
		double moTime = timeManager->motionTimes[i];
		for (unsigned int k=0;k<legStates.size();k++)
		{
			LegCycleState& legState = legStates[k];
			LocomotionLegCycle* legCycle = analyzer->getLegCycle(k,(float)moTime);
			
			for (unsigned int m=0;m<legState.curSupportPos.size();m++)
			{
				// get weighted sum 
				legState.curSupportPos[m] += legCycle->getSupportPos((float)moTime,m)*(float)weights[i];
			}			
		}
		if (weights[i] > maxBlendWeight)
		{
			maxWeightIndex = i;
			maxBlendWeight = weights[i];
		}
	}

	LocomotionAnalyzer* dominantAnalyzer = locoAnalyzers[maxWeightIndex];
	double dominantMotime = timeManager->motionTimes[maxWeightIndex];

	for (unsigned int k=0;k<legStates.size();k++)
	{
		LegCycleState& legState = legStates[k];
		for (unsigned int m=0;m<legState.curSupportPos.size();m++)
		{
			legState.curSupportPos[m] = legState.curSupportPos[m]*gmatBase; // transform to global space	
		}
		if (legState.stanceSupportPos.size() != legState.curSupportPos.size()) // if stance pos is not initialized yet
			legState.stanceSupportPos = legState.curSupportPos;
		int phase = 3;		
		LocomotionLegCycle* legCycle = dominantAnalyzer->getLegCycle(k,(float)dominantMotime);
		transitionWeight = legCycle->getTransitionWeight((float)dominantMotime,phase);
		if (phase == 0)
		{
			legState.stanceSupportPos = legState.curSupportPos;
		}
		// blend in/out of stance leg position
		for (unsigned int m=0;m<legState.curSupportPos.size();m++)
		{
			legState.curSupportPos[m] = legState.curSupportPos[m]*transitionWeight + legState.stanceSupportPos[m]*(1.f-transitionWeight); 
		}
	}
	
	// set ik constraint
	for (unsigned int i=0;i<legInfos.size();i++)
	{
		LegInfo* leg = legInfos[i];
		LegCycleState& legState = legStates[i];
		ConstraintMap& constraint = leg->ikConstraint;
		ConstraintMap::iterator mi;
		for (unsigned int k=0;k<leg->supportJoints.size();k++)
		{			
			EffectorConstantConstraint* foot = dynamic_cast<EffectorConstantConstraint*>(constraint[leg->supportJoints[k]]);
			foot->targetPos = legState.curSupportPos[k];
		}		
		ikScenario.ikPosEffectors = &constraint;
		ikCCD.update(&ikScenario);		
	}
	outFrame = inputFrame;
	// write the result to output frame
	ikScenario.getTreeNodeQuat(outFrame.jointQuat,QUAT_CUR); 		
}