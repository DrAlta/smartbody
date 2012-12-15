# state ChrMarineStartingLeft
# autogenerated by SmartBody
def mocapStartRightSetup(skeletonName, baseJoint, preFix, statePreFix):
	stateManager = scene.getStateManager()

	stateChrMarineStartingLeft = stateManager.createState1D(statePreFix+"StartingRight")

	motions = StringVec()
	motions.append(statePreFix+"ChrMarine@Idle01")
	motions.append(statePreFix+"ChrMarine@Idle01_ToWalk01_Turn90Rt01")
	motions.append(statePreFix+"ChrMarine@Idle01_ToWalk01_Turn180Rt01")

	paramsX = DoubleVec()
	paramsX.append(0) # ChrMarine@Idle01 X
	paramsX.append(-90) # ChrMarine@Idle01_ToWalk01_Turn90Rt01 X
	paramsX.append(-180) # ChrMarine@Idle01_ToWalk01_Turn180Rt01 X
	for i in range(0, len(motions)):
		stateChrMarineStartingLeft.addMotion(motions[i], paramsX[i])

	points0 = DoubleVec()
	points0.append(0) # ChrMarine@Idle01 0
	points0.append(0) # ChrMarine@Idle01_ToWalk01_Turn90Rt01 0
	points0.append(0) # ChrMarine@Idle01_ToWalk01_Turn180Rt01 0
	stateChrMarineStartingLeft.addCorrespondencePoints(motions, points0)
	points1 = DoubleVec()
	points1.append(1.36315) # ChrMarine@Idle01 1
	points1.append(1.53) # ChrMarine@Idle01_ToWalk01_Turn90Rt01 1
	points1.append(1.5) # ChrMarine@Idle01_ToWalk01_Turn180Rt01 1
	stateChrMarineStartingLeft.addCorrespondencePoints(motions, points1)
	points2 = DoubleVec()
	points2.append(2.165) # ChrMarine@Idle01 2
	points2.append(2.43) # ChrMarine@Idle01_ToWalk01_Turn90Rt01 2
	points2.append(2.191) # ChrMarine@Idle01_ToWalk01_Turn180Rt01 2
	stateChrMarineStartingLeft.addCorrespondencePoints(motions, points2)
