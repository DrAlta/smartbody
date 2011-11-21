def reachSetup(characterName):
	print "**Setup Reach Motions**"	
	rightHandMotions = StringVec();
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachRtHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachRtMidHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachRtMidLow")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachLfLow")
	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachLfHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachLfMidHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachRtMidLow")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachRtLow")
	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachMiddleHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachMiddleMidHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachMiddleMidLow")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachMiddleLow")
	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachClose_Lf")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachClose_Rt")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleLow")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleMidHigh")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachClose_MiddleMidLow")
	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_High1")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_High2")	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_Low1")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_Low2")	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidHigh1")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidHigh2")	
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidLow1")
	rightHandMotions.append("ChrHarmony_Relax001_ArmReachBehind_MidLow2")	
	
	leftHandMotions = StringVec();
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachRtHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachRtMidHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachRtMidLow")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachLfLow")
	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachLfHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachLfMidHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachRtMidLow")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachRtLow")
	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachMiddleHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachMiddleMidHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachMiddleMidLow")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachMiddleLow")
	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachClose_Lf")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachClose_Rt")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachClose_MiddleHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachClose_MiddleLow")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachClose_MiddleMidHigh")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachClose_MiddleMidLow")
	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_High1")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_High2")	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_Low1")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_Low2")	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_MidHigh1")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_MidHigh2")	
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_MidLow1")
	leftHandMotions.append("ChrHarmony_Relax001_LArmReachBehind_MidLow2")	
	
	scene = getScene()
	
	reachManager = scene.getReachManager()
	reach = reachManager.createReach(characterName)	
	for i in range(0,len(rightHandMotions)):
		mirrorMotion1 = scene.getMotion(rightHandMotions[i])
		mirrorMotion1.mirror(leftHandMotions[i])
		reach.addMotion("right",scene.getMotion(rightHandMotions[i]))
		reach.addMotion("left",scene.getMotion(leftHandMotions[i]))
		
	
	grabMirrorMotion = scene.getMotion("ChrHarmony_Relax001_HandGraspSmSphere_Grasp")
	grabMirrorMotion.mirror("ChrHarmony_Relax001_LHandGraspSmSphere_Grasp")
	reach.setGrabHandMotion("right",scene.getMotion("ChrHarmony_Relax001_HandGraspSmSphere_Grasp"));
	reach.setGrabHandMotion("left",scene.getMotion("ChrHarmony_Relax001_LHandGraspSmSphere_Grasp"));
	
	reachMirrorMotion = scene.getMotion("ChrHarmony_Relax001_HandGraspSmSphere_Reach")
	reachMirrorMotion.mirror("ChrHarmony_Relax001_LHandGraspSmSphere_Reach")
	reach.setReachHandMotion("right",scene.getMotion("ChrHarmony_Relax001_HandGraspSmSphere_Reach"));
	reach.setReachHandMotion("left",scene.getMotion("ChrHarmony_Relax001_LHandGraspSmSphere_Reach"));
	
	releaseMirrorMotion = scene.getMotion("ChrHarmony_Relax001_HandGraspSmSphere_Release")
	releaseMirrorMotion.mirror("ChrHarmony_Relax001_LHandGraspSmSphere_Release")
	reach.setReleaseHandMotion("right",scene.getMotion("ChrHarmony_Relax001_HandGraspSmSphere_Release"));
	reach.setReleaseHandMotion("left",scene.getMotion("ChrHarmony_Relax001_LHandGraspSmSphere_Release"));
	
	reach.build(scene.getCharacter(characterName))		
# To-Do : handle the hand grasp event
#	scene.getEventManager();
#2 registerevent reach "$1"

#reachSetup("doctor")














