def mocapIdleTurnSetup(origSkelName, skeletonName, baseJoint, preFix, statePreFix):
		
	stateManager = scene.getStateManager()

	print "** State: ChrMarineIdleTurn"
	state = stateManager.createState1D(statePreFix+"IdleTurn")
	state.setBlendSkeleton(origSkelName)
	originalMotions = StringVec()
	originalMotions.append("ChrMarine@Idle01")
	originalMotions.append("ChrMarine@Turn90Lf01")
	originalMotions.append("ChrMarine@Turn180Lf01_smooth")
	originalMotions.append("ChrMarine@Turn90Rt01")
	originalMotions.append("ChrMarine@Turn180Rt01_smooth")
	
	motions = StringVec()
	assetManager = scene.getAssetManager()
	for i in range(0, len(originalMotions)):
		motions.append(preFix + originalMotions[i])
		sbMotion = assetManager.getMotion(originalMotions[i])
		if sbMotion != None:
			sbMotion.setMotionSkeletonName(origSkelName)
			
	params = DoubleVec()
	params.append(0)
	params.append(-90)
	params.append(-180)
	params.append(90)
	params.append(180)

	for i in range(0, len(motions)):
		state.addMotion(motions[i], params[i])

	points1 = DoubleVec()
	points1.append(0)
	points1.append(0)
	points1.append(0)
	points1.append(0)
	points1.append(0)
	state.addCorrespondancePoints(motions, points1)
	points2 = DoubleVec()
	points2.append(0.655738)
	points2.append(0.762295)
	points2.append(0.821311)
	points2.append(0.762295)
	points2.append(0.821311)
	state.addCorrespondancePoints(motions, points2)
	points3 = DoubleVec()
	points3.append(1.333333)
	points3.append(1.42)
	points3.append(1.46667)
	points3.append(1.42)
	points3.append(1.46667)
	state.addCorrespondancePoints(motions, points3)
	points4 = DoubleVec()
	points4.append(2.17)
	points4.append(2.13)
	points4.append(1.733333)
	points4.append(2.13)
	points4.append(1.733333)
	state.addCorrespondancePoints(motions, points4)
