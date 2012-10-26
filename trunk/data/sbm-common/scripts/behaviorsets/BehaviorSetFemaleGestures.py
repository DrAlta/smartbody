scene.run("BehaviorSetCommon.py")

def setupBehaviorSet():
	print "Setting up behavior set for gestures..."
	scene.loadAssetsFromPath("../../../../data/behaviorsets/FemaleGestures/skeletons")
	scene.loadAssetsFromPath("../../../../data/behaviorsets/FemaleGestures/motions")
	scene.addAssetPath("script", "../../../../data/behaviorsets/FemaleGestures/scripts")
	# map the zebra2 skeleton
	scene.run("zebra2-map.py")
	zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
	bradSkeleton = scene.getSkeleton("ChrConnor.sk")
	zebra2Map.applySkeleton(bradSkeleton)
	
	gestureMotions = StringVec()
	gestureMotions.append("ChrConnor@IdleStand01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftLowBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftLowRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftMidBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftMidRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightLowBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightLowRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightMidBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightMidRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoForwardBt01")	
	gestureMotions.append("ChrConnor@IdleStand01_NoForwardRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoLeftBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoLeftRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoRightBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoRightRt01")
	gestureMotions.append("ChrConnor@IdleStand01_ScrewYouBt01")
	gestureMotions.append("ChrConnor@IdleStand01_ScrewYouRt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopForwardBt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopForwardRt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopLeftBt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopLeftRt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopRightBt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopRightRt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouForwardRt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouLeftBt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouLeftRt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouRightBt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouRightRt01")
	gestureMotions.append("ChrConnor@IdleUpright01_KickChair01")
	
	for i in range(0, len(gestureMotions)):
		motion = scene.getMotion(gestureMotions[i])
		zebra2Map.applyMotion(motion)


def retargetBehaviorSet(charName, skelName):
	gestureMotions = StringVec()
	gestureMotions.append("ChrConnor@IdleStand01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftLowBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftLowRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftMidBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateLeftMidRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightLowBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightLowRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightMidBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NegateRightMidRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoForwardBt01")	
	gestureMotions.append("ChrConnor@IdleStand01_NoForwardRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoLeftBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoLeftRt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoRightBt01")
	gestureMotions.append("ChrConnor@IdleStand01_NoRightRt01")
	gestureMotions.append("ChrConnor@IdleStand01_ScrewYouBt01")
	gestureMotions.append("ChrConnor@IdleStand01_ScrewYouRt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopForwardBt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopForwardRt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopLeftBt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopLeftRt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopRightBt01")
	gestureMotions.append("ChrConnor@IdleStand01_StopRightRt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouForwardRt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouLeftBt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouLeftRt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouRightBt01")
	gestureMotions.append("ChrConnor@IdleStand01_YouRightRt01")
	gestureMotions.append("ChrConnor@IdleUpright01_KickChair01")
	
	outDir = '../../../../data/retarget/motion/' + skelName + '/';
	print 'outDir = ' + outDir ;
	if not os.path.exists(outDir):
		os.makedirs(outDir)

	# retarget gestures
	for n in range(0, len(gestureMotions)):
		curMotion = scene.getMotion(gestureMotions[n])
		if curMotion is not None:
			retargetMotion(gestureMotions[n], 'ChrConnor.sk', skelName, outDir + 'FemaleGestures/');
		else:
			print "Cannot find motion " + gestureMotions[n] + ", it will be excluded from the gesture setup..."

		
