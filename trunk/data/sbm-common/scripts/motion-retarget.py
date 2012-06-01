print "|-------------------------------------------------|"
print "|  data/sbm-common/scripts/motion-retarget.py     |"
print "|-------------------------------------------------|"

def retargetMotion(motionName, srcSkelName, tgtSkelName, outDir) :	
	testMotion = scene.getMotion(motionName);
	outMotionName = tgtSkelName + motionName		
	existMotion = scene.getMotion(outMotionName)
	if existMotion != None : # do nothing if the retargeted motion is already there
		return	
		
	if not os.path.exists(outDir):
		os.makedirs(outDir)
		
	offsetJoints = VecMap();
	endJoints = StringVec();
	endJoints.append('l_ankle')
	endJoints.append('r_ankle')	
	effectorJoints = StringVec();
	#effectorJoints.append('r_ankle')	
	effectorJoints.append('r_forefoot')
	effectorJoints.append('r_toe')
	#effectorJoints.append('l_ankle')
	effectorJoints.append('l_forefoot')	
	effectorJoints.append('l_toe')		
	
	#print 'Retarget motion = ' + motionName;
	outMotion = testMotion.retarget(outMotionName,srcSkelName,tgtSkelName, endJoints, offsetJoints);	
	cleanMotion = testMotion.constrain(outMotionName, srcSkelName, tgtSkelName, outMotionName, effectorJoints);
	saveCommand = 'animation ' + outMotionName + ' save ' + outDir + outMotionName + '.skm';
	print 'Save command = ' + saveCommand;
	scene.command(saveCommand)

def getStandardLocomomtionAnimations(locoMotions, preFix):
	locoMotions.append(preFix+"ChrUtah_Walk001")
	locoMotions.append(preFix+"ChrUtah_Idle001")
	locoMotions.append(preFix+"ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	locoMotions.append(preFix+"ChrUtah_Idle01_ToWalk01_Turn360Rt01")
	locoMotions.append(preFix+"ChrUtah_Meander01")
	locoMotions.append(preFix+"ChrUtah_Shuffle01")
	locoMotions.append(preFix+"ChrUtah_Jog001")
	locoMotions.append(preFix+"ChrUtah_Run001")
	locoMotions.append(preFix+"ChrUtah_WalkInCircleLeft001")
	locoMotions.append(preFix+"ChrUtah_WalkInCircleRight001")
	locoMotions.append(preFix+"ChrUtah_WalkInTightCircleLeft001")
	locoMotions.append(preFix+"ChrUtah_WalkInTightCircleRight001")
	locoMotions.append(preFix+"ChrUtah_RunInCircleLeft001")
	locoMotions.append(preFix+"ChrUtah_RunInCircleRight001")
	locoMotions.append(preFix+"ChrUtah_RunInTightCircleLeft01")
	locoMotions.append(preFix+"ChrUtah_RunInTightCircleRight01")
	locoMotions.append(preFix+"ChrUtah_StrafeSlowRt01")
	locoMotions.append(preFix+"ChrUtah_StrafeSlowLf01")
	locoMotions.append(preFix+"ChrUtah_StrafeFastRt01")
	locoMotions.append(preFix+"ChrUtah_StrafeFastLf01")
	locoMotions.append(preFix+"ChrUtah_Idle001")
	locoMotions.append(preFix+"ChrUtah_Turn90Lf01")
	locoMotions.append(preFix+"ChrUtah_Turn180Lf01")
	locoMotions.append(preFix+"ChrUtah_Turn90Rt01")
	locoMotions.append(preFix+"ChrUtah_Turn180Rt01")
	locoMotions.append(preFix+"ChrUtah_StopToWalkRt01")
	locoMotions.append(preFix+"ChrUtah_Idle01_ToWalk01_Turn90Lf01")
	locoMotions.append(preFix+"ChrUtah_Idle01_ToWalk01_Turn180Lf01")
	locoMotions.append(preFix+"ChrUtah_Idle01_StepBackwardRt01")
	locoMotions.append(preFix+"ChrUtah_Idle01_StepForwardRt01")
	locoMotions.append(preFix+"ChrUtah_Idle01_StepSidewaysRt01")
	locoMotions.append(preFix+"ChrUtah_Idle01_StepBackwardLf01")
	locoMotions.append(preFix+"ChrUtah_Idle01_StepForwardLf01")
	locoMotions.append(preFix+"ChrUtah_Idle01_StepSidewaysLf01")	
	
def getMarineLocomomtionAnimations(marineLocomotions, preFix):		
	marineLocomotions.append("ChrMarine@Idle01")
	marineLocomotions.append("ChrMarine@Jog01")
	marineLocomotions.append("ChrMarine@Meander01")
	marineLocomotions.append("ChrMarine@Run01")
	marineLocomotions.append("ChrMarine@RunCircleLf01_smooth")
	marineLocomotions.append("ChrMarine@RunCircleRt01_smooth")
	marineLocomotions.append("ChrMarine@RunTightCircleLf01")
	marineLocomotions.append("ChrMarine@RunTightCircleRt01")
	marineLocomotions.append("ChrMarine@StrafeSlowLf01")
	marineLocomotions.append("ChrMarine@StrafeSlowRt01")
	marineLocomotions.append("ChrMarine@Walk01")
	marineLocomotions.append("ChrMarine@WalkCircleLf01_smooth")
	marineLocomotions.append("ChrMarine@WalkCircleRt01_smooth")
	marineLocomotions.append("ChrMarine@WalkTightCircleLf01_smooth")
	marineLocomotions.append("ChrMarine@WalkTightCircleRt01_smooth")
	marineLocomotions.append("ChrMarine@Turn360Lf01")
	marineLocomotions.append("ChrMarine@Turn360Rt01")
	marineLocomotions.append("ChrMarine@StrafeFastLf01_smooth")
	marineLocomotions.append("ChrMarine@StrafeFastRt01_smooth")
	marineLocomotions.append("ChrMarine@Idle01_StepBackwardsLf01")
	marineLocomotions.append("ChrMarine@Idle01_StepBackwardsRt01")
	marineLocomotions.append("ChrMarine@Idle01_StepForwardLf01")
	marineLocomotions.append("ChrMarine@Idle01_StepForwardRt01")
	marineLocomotions.append("ChrMarine@Idle01_StepSidewaysLf01")
	marineLocomotions.append("ChrMarine@Idle01_StepSidewaysRt01")
	marineLocomotions.append("ChrMarine@Turn90Lf01")
	marineLocomotions.append("ChrMarine@Turn180Lf01")
	marineLocomotions.append("ChrMarine@Turn90Rt01")
	marineLocomotions.append("ChrMarine@Turn180Rt01")
	marineLocomotions.append("ChrMarine@Idle01_ToWalkLf01")
	marineLocomotions.append("ChrMarine@Idle01_ToWalk01_Turn90Lf01")
	marineLocomotions.append("ChrMarine@Idle01_ToWalk01_Turn180Lf01")
	marineLocomotions.append("ChrMarine@Idle01_ToWalk01")
	marineLocomotions.append("ChrMarine@Idle01_ToWalk01_Turn90Rt01")
	marineLocomotions.append("ChrMarine@Idle01_ToWalk01_Turn180Rt01")
	
def getStandardReachMotions(reachMotions, preFix):	
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachRtHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachRtMidHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachRtMidLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachLfLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachLfHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachLfMidHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachRtMidLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachRtLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachMiddleHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachMiddleMidHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachMiddleMidLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachMiddleLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachClose_Lf")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachClose_Rt")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachClose_MiddleHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachClose_MiddleLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachClose_MiddleMidHigh")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachClose_MiddleMidLow")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_High1")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_High2")	
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_Low1")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_Low2")	
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_MidHigh1")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_MidHigh2")	
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_MidLow1")
	reachMotions.append(preFix+"ChrHarmony_Relax001_ArmReachBehind_MidLow2")	
	reachMotions.append(preFix+"ChrHarmony_Relax001_HandGraspSmSphere_Grasp")
	reachMotions.append(preFix+"ChrHarmony_Relax001_HandGraspSmSphere_Reach")
	reachMotions.append(preFix+"ChrHarmony_Relax001_HandGraspSmSphere_Release")

def getStandardGestureMotions(gestureMotions, preFix):
	gestureMotions.append(preFix+"HandsAtSide_Motex")
	gestureMotions.append(preFix+"HandsAtSide_Arms_Sweep")
	gestureMotions.append(preFix+"HandsAtSide_Motex_Softened")
	gestureMotions.append(preFix+"HandsAtSide_RArm_GestureYou")
	gestureMotions.append(preFix+"HandsAtSide_Transition_LHandOnHip")
	gestureMotions.append(preFix+"LHandOnHip_Arms_GestureWhy")
	gestureMotions.append(preFix+"LHandOnHip_Motex")
	gestureMotions.append(preFix+"LHandOnHip_RArm_GestureOffer")
	gestureMotions.append(preFix+"LHandOnHip_RArm_SweepRight")
	gestureMotions.append(preFix+"LHandOnHip_Transition_HandsAtSide")	


def retargetSetup(targetSkelName):
	gestureMotions = StringVec()
	reachMotions = StringVec()
	locoMotions = StringVec()
	marineLocomotions = StringVec()
	getStandardLocomomtionAnimations(locoMotions,"")
	getMarineLocomomtionAnimations(marineLocomotions,"")
	getStandardGestureMotions(gestureMotions,"")
	getStandardReachMotions(reachMotions,"")
	
	outDir = '../../../../data/sbm-common/common-sk/motion/' + targetSkelName + '/';
	if not os.path.exists(outDir):
		os.makedirs(outDir)
	# retarget reach motions
	for n in range(0, len(reachMotions)):
		retargetMotion(reachMotions[n], 'common.sk', targetSkelName, outDir+'reachMotion/');
	
	# retarget standard locomotions
	for n in range(0, len(locoMotions)):
		retargetMotion(locoMotions[n], 'test_utah.sk', targetSkelName, outDir+'locoMotion/');
	
	# retarget marine locomotions
	#for n in range(0, len(marineLocomotions)):
	#	retargetMotion(marineLocomotions[n], 'ChrBackovic.sk', targetSkelName, outDir+'marineLocomotion/');
		
	# retarget gesture motions
	for n in range(0, len(gestureMotions)):
		retargetMotion(gestureMotions[n], 'common.sk', targetSkelName, outDir);
	

	
def retargetCharacter(charName, targetSkelName):
	retargetSetup(targetSkelName) # make sure all retargeted motions are already created
	
	# setup standard locomotion
	scene.run("stateAllLocomotion.py")
	locomotionSetup(targetSkelName, "base", targetSkelName, targetSkelName)
	
	# setup marine locomotion
	#scene.run("stateMarineLocomotion.py")
	#marineLocomotionSetup(targetSkelName, "base", targetSkelName, targetSkelName)

	# starting state, starting locomotion with different angle
	scene.run("stateAllStarting.py")
	startingSetup(targetSkelName, "base", targetSkelName, targetSkelName)

	# idle turn state, facing adjusting
	scene.run("stateAllIdleTurn.py")
	idleTurnSetup(targetSkelName, "base", targetSkelName, targetSkelName)

	# step state, stepping adjusting
	scene.run("stateAllStep.py")
	stepSetup(targetSkelName, "base", targetSkelName, targetSkelName)

	# transitions
	scene.run("transitions.py")
	transitionSetup(targetSkelName, targetSkelName)
	
	# setup reach 
	scene.run("init-example-reach.py")
	reachSetup(charName,targetSkelName)
	
	# setup steering
	scene.run("init-steer-agents.py")
	steerManager = scene.getSteerManager()
	steerManager.setEnable(False)
	setupSteerAgent(charName,targetSkelName)	
	steerManager.setEnable(True)
		
	




	

	





