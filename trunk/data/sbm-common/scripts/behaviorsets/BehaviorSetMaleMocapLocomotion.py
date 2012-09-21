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

def retargetBehaviorSet(charName, skelName):
	locoMotions = StringVec()
	locoMotions.append("ChrUtah_Walk001")
	locoMotions.append("ChrUtah_Idle001")
	locoMotions.append("ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	locoMotions.append("ChrUtah_Idle01_ToWalk01_Turn360Rt01")
	locoMotions.append("ChrUtah_Meander01")
	locoMotions.append("ChrUtah_Shuffle01")
	locoMotions.append("ChrUtah_Jog001")
	locoMotions.append("ChrUtah_Run001")
	locoMotions.append("ChrUtah_WalkInCircleLeft001")
	locoMotions.append("ChrUtah_WalkInCircleRight001")
	locoMotions.append("ChrUtah_WalkInTightCircleLeft001")
	locoMotions.append("ChrUtah_WalkInTightCircleRight001")
	locoMotions.append("ChrUtah_RunInCircleLeft001")
	locoMotions.append("ChrUtah_RunInCircleRight001")
	locoMotions.append("ChrUtah_RunInTightCircleLeft01")
	locoMotions.append("ChrUtah_RunInTightCircleRight01")
	locoMotions.append("ChrUtah_StrafeSlowRt01")
	locoMotions.append("ChrUtah_StrafeSlowLf01")
	locoMotions.append("ChrUtah_StrafeFastRt01")
	locoMotions.append("ChrUtah_StrafeFastLf01")
	locoMotions.append("ChrUtah_Idle001")
	locoMotions.append("ChrUtah_Turn90Lf01")
	locoMotions.append("ChrUtah_Turn180Lf01")
	locoMotions.append("ChrUtah_Turn90Rt01")
	locoMotions.append("ChrUtah_Turn180Rt01")
	locoMotions.append("ChrUtah_StopToWalkRt01")
	locoMotions.append("ChrUtah_Idle01_ToWalk01_Turn90Lf01")
	locoMotions.append("ChrUtah_Idle01_ToWalk01_Turn180Lf01")
	locoMotions.append("ChrUtah_Idle01_StepBackwardRt01")
	locoMotions.append("ChrUtah_Idle01_StepForwardRt01")
	locoMotions.append("ChrUtah_Idle01_StepSidewaysRt01")
	locoMotions.append("ChrUtah_Idle01_StepBackwardLf01")
	locoMotions.append("ChrUtah_Idle01_StepForwardLf01")
	locoMotions.append("ChrUtah_Idle01_StepSidewaysLf01")
	
	outDir = '../../../../data/retarget/motion/' + skelName + '/';
	if not os.path.exists(outDir):
		os.makedirs(outDir)
		
	# retarget standard locomotions
	for n in range(0, len(locoMotions)):
		curMotion = scene.getMotion(locoMotions[n])
		if curMotion is not None:
			retargetMotion(locoMotions[n], 'test_utah.sk', skelName, outDir + 'MaleMocapLocomotion/');

	# setup standard locomotion
	scene.run("stateAllLocomotion.py")
	locomotionSetup(skelName, "base", skelName, skelName)
	
	# starting state, starting locomotion with different angle
	scene.run("stateAllStarting.py")
	startingSetup(skelName, "base", skelName, skelName)

	# idle turn state, facing adjusting
	scene.run("stateAllIdleTurn.py")
	idleTurnSetup(skelName, "base", skelName, skelName)

	# step state, stepping adjusting
	scene.run("stateAllStep.py")
	stepSetup(skelName, "base", skelName, skelName)

	# transitions
	scene.run("transitions.py")
	transitionSetup(skelName, skelName)
	
	# setup steering
	scene.run("init-steer-agents.py")
	steerManager = scene.getSteerManager()
	steerManager.setEnable(False)
	setupSteerAgent(charName, skelName)	
	steerManager.setEnable(True)
	
