scene.run("BehaviorSetCommon.py")

def setupBehaviorSet():
	print "Setting up behavior set for Female Locomotion..."
	scene.loadAssetsFromPath("../../../../data/behaviorsets/jumping/skeletons")
	scene.loadAssetsFromPath("../../../../data/behaviorsets/jumping/motions")
	scene.addAssetPath("script", "../../../../data/behaviorsets/jumping/scripts")
	# map the Backovic skeleton
	scene.run("zebra2-map.py")
	zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
	zebra2Skeleton = scene.getSkeleton("ChrGarza.sk")
	zebra2Map.applySkeleton(zebra2Skeleton)
	
	jumpMotions = StringVec()
	
	jumpMotions.append("ChrGarza@IdleStand01")	
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardHighMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardHighNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardLowFar01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardLowMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardLowNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45HighMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45HighNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowFar01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90HighMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowFar01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpUpHigh01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpUpLow01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpUpMedium01")
	
	for i in range(0, len(jumpMotions)):
		motion = scene.getMotion(jumpMotions[i])
		zebra2Map.applyMotion(motion)
	
	mirroredMotions = StringVec()
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft45HighMid01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft45HighNear01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft45LowFar01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft45LowMid01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft45LowNear01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft90HighMid01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft90LowFar01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft90LowMid01")
	mirroredMotions.append("ChrGarza@IdleStand01_JumpLeft90LowNear01")	
	
	for i in range(0,len(mirroredMotions)):
		mirrorMotion = scene.getMotion(mirroredMotions[i])
		if mirrorMotion != None:
			mirrorMotion.mirror(mirroredMotions[i]+"Rt", "ChrGarza.sk")


def retargetBehaviorSet(charName, skelName):
	jumpMotions = StringVec()
	
	jumpMotions.append("ChrGarza@IdleStand01")	
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardHighMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardHighNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardLowFar01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardLowMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpForwardLowNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45HighMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45HighNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowFar01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90HighMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowFar01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowMid01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowNear01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpUpHigh01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpUpLow01")
	jumpMotions.append("ChrGarza@IdleStand01_JumpUpMedium01")
	#include the mirrored motions
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45HighMid01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45HighNear01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowFar01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowMid01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft45LowNear01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90HighMid01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowFar01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowMid01"+"Rt")
	jumpMotions.append("ChrGarza@IdleStand01_JumpLeft90LowNear01"+"Rt")	
	
	outDir = '../../../../data/retarget/motion/' + skelName + '/';
	if not os.path.exists(outDir):
		os.makedirs(outDir)
		
	# retarget jumping
	for n in range(0, len(jumpMotions)):
		retargetMotion(jumpMotions[n], 'ChrGarza.sk', skelName, outDir + 'jumping/');
		


	# setup standard locomotion
	scene.run("stateJumping.py")
	jumpingSetup(skelName, "base", skelName, skelName)
	
		
