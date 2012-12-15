scene.run("BehaviorSetCommon.py")

def setupBehaviorSet():
	print "Setting up behavior set for MaleMocapLocomotion..."
	scene.loadAssetsFromPath("../../../../data/behaviorsets/MaleMocapLocomotion/skeletons")
	scene.loadAssetsFromPath("../../../../data/behaviorsets/MaleMocapLocomotion/motions")
	scene.addAssetPath("script", "../../../../data/behaviorsets/MaleMocapLocomotion/scripts")
	# map the zebra2 skeleton
	scene.run("zebra2-map.py")
	zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
	bradSkeleton = scene.getSkeleton("ChrBackovic.sk")
	zebra2Map.applySkeleton(bradSkeleton)	
	locoMotions = StringVec()
	locoMotions.append("ChrMarine@Idle01")
	locoMotions.append("ChrMarine@Jog01")
	locoMotions.append("ChrMarine@Meander01")
	locoMotions.append("ChrMarine@Run01")
	locoMotions.append("ChrMarine@RunCircleLf01_smooth")
	locoMotions.append("ChrMarine@RunCircleRt01_smooth")
	locoMotions.append("ChrMarine@RunTightCircleLf01")
	locoMotions.append("ChrMarine@RunTightCircleRt01")
	locoMotions.append("ChrMarine@StrafeSlowLf01")
	locoMotions.append("ChrMarine@StrafeSlowRt01")
	locoMotions.append("ChrMarine@Walk01")
	locoMotions.append("ChrMarine@WalkCircleLf01_smooth")
	locoMotions.append("ChrMarine@WalkCircleRt01_smooth")
	locoMotions.append("ChrMarine@WalkTightCircleLf01_smooth")
	locoMotions.append("ChrMarine@WalkTightCircleRt01_smooth")
	locoMotions.append("ChrMarine@Turn360Lf01")
	locoMotions.append("ChrMarine@Turn360Rt01")
	locoMotions.append("ChrMarine@StrafeFastLf01_smooth")
	locoMotions.append("ChrMarine@StrafeFastRt01_smooth")
	locoMotions.append("ChrMarine@Idle01_StepBackwardsLf01")
	locoMotions.append("ChrMarine@Idle01_StepBackwardsRt01")
	locoMotions.append("ChrMarine@Idle01_StepForwardLf01")
	locoMotions.append("ChrMarine@Idle01_StepForwardRt01")
	locoMotions.append("ChrMarine@Idle01_StepSidewaysLf01")
	locoMotions.append("ChrMarine@Idle01_StepSidewaysRt01")
	locoMotions.append("ChrMarine@Turn90Lf01")
	locoMotions.append("ChrMarine@Turn180Lf01")
	locoMotions.append("ChrMarine@Turn90Rt01")
	locoMotions.append("ChrMarine@Turn180Rt01")
	locoMotions.append("ChrMarine@Idle01_ToWalkLf01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn90Lf01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn180Lf01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn90Rt01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn180Rt01")
	for i in range(0, len(locoMotions)):
		motion = scene.getMotion(locoMotions[i])
		zebra2Map.applyMotion(motion)

def retargetBehaviorSet(charName, skelName):

	scene.run('locomotion-ChrMarine-init.py')
	
	locoMotions = StringVec()
	locoMotions.append("ChrMarine@Idle01")
	locoMotions.append("ChrMarine@Jog01")
	locoMotions.append("ChrMarine@Meander01")
	locoMotions.append("ChrMarine@Run01")
	locoMotions.append("ChrMarine@RunCircleLf01_smooth")
	locoMotions.append("ChrMarine@RunCircleRt01_smooth")
	locoMotions.append("ChrMarine@RunTightCircleLf01")
	locoMotions.append("ChrMarine@RunTightCircleRt01")
	locoMotions.append("ChrMarine@StrafeSlowLf01")
	locoMotions.append("ChrMarine@StrafeSlowRt01")
	locoMotions.append("ChrMarine@Walk01")
	locoMotions.append("ChrMarine@WalkCircleLf01_smooth")
	locoMotions.append("ChrMarine@WalkCircleRt01_smooth")
	locoMotions.append("ChrMarine@WalkTightCircleLf01_smooth")
	locoMotions.append("ChrMarine@WalkTightCircleRt01_smooth")
	locoMotions.append("ChrMarine@Turn360Lf01")
	locoMotions.append("ChrMarine@Turn360Rt01")
	locoMotions.append("ChrMarine@StrafeFastLf01_smooth")
	locoMotions.append("ChrMarine@StrafeFastRt01_smooth")
	locoMotions.append("ChrMarine@Idle01_StepBackwardsLf01")
	locoMotions.append("ChrMarine@Idle01_StepBackwardsRt01")
	locoMotions.append("ChrMarine@Idle01_StepForwardLf01")
	locoMotions.append("ChrMarine@Idle01_StepForwardRt01")
	locoMotions.append("ChrMarine@Idle01_StepSidewaysLf01")
	locoMotions.append("ChrMarine@Idle01_StepSidewaysRt01")
	locoMotions.append("ChrMarine@Turn90Lf01")
	locoMotions.append("ChrMarine@Turn180Lf01")
	locoMotions.append("ChrMarine@Turn90Rt01")
	locoMotions.append("ChrMarine@Turn180Rt01")
	locoMotions.append("ChrMarine@Idle01_ToWalkLf01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn90Lf01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn180Lf01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn90Rt01")
	locoMotions.append("ChrMarine@Idle01_ToWalk01_Turn180Rt01")
	
	
		
	outDir = '../../../../data/retarget/motion/' + skelName + '/';
	if not os.path.exists(outDir):
		os.makedirs(outDir)
		
	# retarget standard locomotions
	for n in range(0, len(locoMotions)):
		curMotion = scene.getMotion(locoMotions[n])
		if curMotion is not None:
			retargetMotion(locoMotions[n], 'ChrBackovic.sk', skelName, outDir + 'MaleMocapLocomotion/');

	# setup standard locomotion
	scene.run("locomotion-ChrMarine-state-Locomotion.py")
	marineLocomotionSetup(skelName, "base", skelName, skelName)
	
	# starting state, starting locomotion with different angle
	#scene.run("locomotion-ChrMarine-state-StartingLeft.py")
	#startingSetup(skelName, "base", skelName, skelName)

	# idle turn state, facing adjusting
	scene.run("locomotion-ChrMarine-state-IdleTurn.py")
	mocapIdleTurnSetup(skelName, "base", skelName, skelName)

	# step state, stepping adjusting
	scene.run("locomotion-ChrMarine-state-Step.py")
	mocapStepSetup(skelName, "base", skelName, skelName)
	
		
	# setup steering
	scene.run("init-steer-agents.py")
	steerManager = scene.getSteerManager()
	steerManager.setEnable(False)
	setupSteerAgent(charName, skelName)	
	steerManager.setEnable(True)
	
