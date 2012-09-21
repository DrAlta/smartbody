scene.run("BehaviorSetCommon.py")

def setupBehaviorSet():
	print "Setting up behavior set for gestures..."
	scene.loadAssetsFromPath("../../../../data/behaviorsets/MocapReaching/skeletons")
	scene.loadAssetsFromPath("../../../../data/behaviorsets/MocapReaching/motions")
	scene.addAssetPath("script", "../../../../data/behaviorsets/MocapReaching/scripts")
	# map the zebra2 skeleton
	scene.run("zebra2-map.py")
	zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
	garzaSkeleton = scene.getSkeleton("ChrGarza.sk")
	zebra2Map.applySkeleton(garzaSkeleton)
	
	mocapReachMotions = StringVec();
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackFloor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackHigh01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackLow01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackMediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackMediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackMediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardFloor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardHigh01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardLow01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardMediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardMediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30MediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60MediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120MediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30MediumNear01")
	
	for i in range(0, len(mocapReachMotions)):
		motion = scene.getMotion(mocapReachMotions[i])
		zebra2Map.applyMotion(motion)

def retargetBehaviorSet(charName, skelName):
	mocapReachMotions = StringVec();
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackFloor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackHigh01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackLow01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackMediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackMediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachBackMediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardFloor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardHigh01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardLow01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardMediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachForwardMediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft30MediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachLeft60MediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight120MediumNear01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30Floor01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30High01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30Low01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30MediumFar01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30MediumMid01")
	mocapReachMotions.append("ChrGarza@IdleStand01_ReachRight30MediumNear01")	
	
	mocapReachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Grasp")
	mocapReachMotions.append("ChrHarmony_Relax001_LHandGraspSmSphere_Grasp")	
	mocapReachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Grasp")	
	mocapReachMotions.append("ChrHarmony_Relax001_LHandGraspSmSphere_Grasp")		
	
	mocapReachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Reach")
	mocapReachMotions.append("ChrHarmony_Relax001_LHandGraspSmSphere_Reach")	
	mocapReachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Reach")	
	mocapReachMotions.append("ChrHarmony_Relax001_LHandGraspSmSphere_Reach")	
	
	mocapReachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Release")
	mocapReachMotions.append("ChrHarmony_Relax001_LHandGraspSmSphere_Release")	
	mocapReachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Release")	
	mocapReachMotions.append("ChrHarmony_Relax001_LHandGraspSmSphere_Release")	

	mocapReachMotions.append("HandsAtSide_RArm_GestureYou")
	mocapReachMotions.append("HandsAtSide_LArm_GestureYou")	
	
	outDir = '../../../../data/retarget/motion/' + skelName + '/';
	print 'outDir = ' + outDir ;
	if not os.path.exists(outDir):
		os.makedirs(outDir)

	# retarget mocap reaching
	for n in range(0, len(mocapReachMotions)):
		motion = scene.getMotion(mocapReachMotions[n])
		if motion is not None:
			retargetMotion(mocapReachMotions[n], 'ChrGarza.sk', skelName, outDir + 'MocapReaching/');
		else:
			print "Cannot find motion " + mocapReachMotions[n] + ", it will be excluded from the reach setup..."

	scene.run("init-example-reach-mocap.py")
	reachSetup(charName, "KNN", skelName)

		