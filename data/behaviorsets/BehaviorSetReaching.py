scene.run("BehaviorSetCommon.py")
def setupBehaviorSet():
	print "Setting up behavior set for Reaching ..."
	scene.loadAssetsFromPath("behaviorsets/reaching/skeletons")
	scene.loadAssetsFromPath("behaviorsets/reaching/motions")
	scene.addAssetPath("script", "behaviorsets/reaching/scripts")
	
	
def retargetBehaviorSet(charName, skelName):
	reachMotions = StringVec()
	reachMotions.append("ChrHarmony_Relax001_ArmReachRtHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachRtMidHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachRtMidLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachLfLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachLfHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachLfMidHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachRtMidLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachRtLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachMiddleHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachMiddleMidHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachMiddleMidLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachMiddleLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachClose_Lf")
	reachMotions.append("ChrHarmony_Relax001_ArmReachClose_Rt")
	reachMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleMidHigh")
	reachMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleMidLow")
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_High1")
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_High2")	
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_Low1")
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_Low2")	
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidHigh1")
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidHigh2")	
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidLow1")
	reachMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidLow2")	
	reachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Grasp")
	reachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Reach")
	reachMotions.append("ChrHarmony_Relax001_HandGraspSmSphere_Release")
	reachMotions.append("HandsAtSide_RArm_GestureYou")
	
	outDir = scene.getMediaPath() + 'retarget/motion/' + skelName + '/';
	print 'outDir = ' + outDir ;
	if not os.path.exists(outDir):
		os.makedirs(outDir)

	createRetargetInstance('common.sk', skelName)
	# retarget reaching
	#for n in range(0, len(reachMotions)):
	#	retargetMotion(reachMotions[n], 'common.sk', skelName, outDir + 'Reaching/');	
	assetManager = scene.getAssetManager()
	for i in range(0, len(reachMotions)):		
		sbMotion = assetManager.getMotion(reachMotions[i])
		if sbMotion != None:
			sbMotion.setMotionSkeletonName('common.sk')

	scene.run("init-example-reach.py")
	reachSetup(charName, "KNN", 'common.sk', '')	