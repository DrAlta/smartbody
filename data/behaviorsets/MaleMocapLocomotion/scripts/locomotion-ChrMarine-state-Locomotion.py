# state marineLocomotion
# autogenerated by SmartBody

stateManager = scene.getStateManager()

statemarineLocomotion = stateManager.createState3D("mocapLocomotion")
statemarineLocomotion.setBlendSkeleton('ChrBackovic.sk')
motions = StringVec()
motions.append("ChrMarine@Idle01")
motions.append("ChrMarine@Jog01")
motions.append("ChrMarine@Meander01")
motions.append("ChrMarine@Run01")
# motions.append("ChrMarine@RunCircleLf01_smooth")
# motions.append("ChrMarine@RunCircleRt01_smooth")
motions.append("ChrMarine@RunCircleLf01_smooth")
motions.append("ChrMarine@RunCircleRt01_smooth")
motions.append("ChrMarine@RunTightCircleLf01")
motions.append("ChrMarine@RunTightCircleRt01")
motions.append("ChrMarine@StrafeSlowLf01")
motions.append("ChrMarine@StrafeSlowRt01")
motions.append("ChrMarine@Walk01")
# motions.append("ChrMarine@WalkCircleLf01_smooth")
# motions.append("ChrMarine@WalkCircleRt01_smooth")
# motions.append("ChrMarine@WalkTightCircleLf01_smooth")
# motions.append("ChrMarine@WalkTightCircleRt01_smooth")
motions.append("ChrMarine@WalkCircleLf01_smooth")
motions.append("ChrMarine@WalkCircleRt01_smooth")
motions.append("ChrMarine@WalkTightCircleLf01_smooth")
motions.append("ChrMarine@WalkTightCircleRt01_smooth")
motions.append("ChrMarine@Turn360Lf01")
motions.append("ChrMarine@Turn360Rt01")
motions.append("ChrMarine@StrafeFastLf01_smooth")
motions.append("ChrMarine@StrafeFastRt01_smooth")

paramsX = DoubleVec()
paramsY = DoubleVec()
paramsZ = DoubleVec()
paramsX.append(0) # ChrMarine@Idle01 X
paramsY.append(0) # ChrMarine@Idle01 Y
paramsZ.append(0) # ChrMarine@Idle01 Z
paramsX.append(0) # ChrMarine@Jog01 X
paramsY.append(0) # ChrMarine@Jog01 Y
paramsZ.append(0) # ChrMarine@Jog01 Z
paramsX.append(0) # ChrMarine@Meander01 X
paramsY.append(0) # ChrMarine@Meander01 Y
paramsZ.append(0) # ChrMarine@Meander01 Z
paramsX.append(0) # ChrMarine@Run01 X
paramsY.append(0) # ChrMarine@Run01 Y
paramsZ.append(0) # ChrMarine@Run01 Z
paramsX.append(0) # ChrMarine@RunCircleLf01_smooth X
paramsY.append(0) # ChrMarine@RunCircleLf01_smooth Y
paramsZ.append(0) # ChrMarine@RunCircleLf01_smooth Z
paramsX.append(0) # ChrMarine@RunCircleRt01_smooth X
paramsY.append(0) # ChrMarine@RunCircleRt01_smooth Y
paramsZ.append(0) # ChrMarine@RunCircleRt01_smooth Z
paramsX.append(0) # ChrMarine@RunTightCircleLf01 X
paramsY.append(0) # ChrMarine@RunTightCircleLf01 Y
paramsZ.append(0) # ChrMarine@RunTightCircleLf01 Z
paramsX.append(0) # ChrMarine@RunTightCircleRt01 X
paramsY.append(0) # ChrMarine@RunTightCircleRt01 Y
paramsZ.append(0) # ChrMarine@RunTightCircleRt01 Z
paramsX.append(0) # ChrMarine@StrafeFastLf01_smooth X
paramsY.append(0) # ChrMarine@StrafeFastLf01_smooth Y
paramsZ.append(0) # ChrMarine@StrafeFastLf01_smooth Z
paramsX.append(0) # ChrMarine@StrafeFastRt01_smooth X
paramsY.append(0) # ChrMarine@StrafeFastRt01_smooth Y
paramsZ.append(0) # ChrMarine@StrafeFastRt01_smooth Z
paramsX.append(0) # ChrMarine@Walk01 X
paramsY.append(0) # ChrMarine@Walk01 Y
paramsZ.append(0) # ChrMarine@Walk01 Z
paramsX.append(0) # ChrMarine@WalkCircleLf01_smooth X
paramsY.append(0) # ChrMarine@WalkCircleLf01_smooth Y
paramsZ.append(0) # ChrMarine@WalkCircleLf01_smooth Z
paramsX.append(0) # ChrMarine@WalkCircleRt01_smooth X
paramsY.append(0) # ChrMarine@WalkCircleRt01_smooth Y
paramsZ.append(0) # ChrMarine@WalkCircleRt01_smooth Z
paramsX.append(0) # ChrMarine@WalkTightCircleLf01_smooth X
paramsY.append(0) # ChrMarine@WalkTightCircleLf01_smooth Y
paramsZ.append(0) # ChrMarine@WalkTightCircleLf01_smooth Z
paramsX.append(0) # ChrMarine@WalkTightCircleRt01_smooth X
paramsY.append(0) # ChrMarine@WalkTightCircleRt01_smooth Y
paramsZ.append(0) # ChrMarine@WalkTightCircleRt01_smooth Z
paramsX.append(0) # ChrMarine@WalkTightCircleLf01_smooth X
paramsY.append(0) # ChrMarine@WalkTightCircleLf01_smooth Y
paramsZ.append(0) # ChrMarine@WalkTightCircleLf01_smooth Z
paramsX.append(0) # ChrMarine@WalkTightCircleRt01_smooth X
paramsY.append(0) # ChrMarine@WalkTightCircleRt01_smooth Y
paramsZ.append(0) # ChrMarine@WalkTightCircleRt01_smooth Z

paramsX.append(0) # ChrMarine@WalkTightCircleLf01_smooth X
paramsY.append(0) # ChrMarine@WalkTightCircleLf01_smooth Y
paramsZ.append(0) # ChrMarine@WalkTightCircleLf01_smooth Z
paramsX.append(0) # ChrMarine@WalkTightCircleRt01_smooth X
paramsY.append(0) # ChrMarine@WalkTightCircleRt01_smooth Y
paramsZ.append(0) # ChrMarine@WalkTightCircleRt01_smooth Z

for i in range(0, len(motions)):
	statemarineLocomotion.addMotion(motions[i], paramsX[i], paramsY[i], paramsZ[i])

points0 = DoubleVec()
points0.append(0.2) # ChrMarine@Idle01 0
points0.append(0.4) # ChrMarine@Jog01 0
points0.append(0.75) # ChrMarine@Meander01 0
points0.append(0.3) # ChrMarine@Run01 0
points0.append(0.4) # ChrMarine@RunCircleLf01_smooth 0
points0.append(0.1) # ChrMarine@RunCircleRt01_smooth 0
points0.append(0.1) # ChrMarine@RunTightCircleLf01 0
points0.append(0.4) # ChrMarine@RunTightCircleRt01 0
points0.append(0.1) # ChrMarine@StrafeSlowLf01 0
points0.append(0.75) # ChrMarine@StrafeSlowRt01 0
points0.append(0.65) # ChrMarine@Walk01 0
points0.append(0.15) # ChrMarine@WalkCircleLf01_smooth 0
points0.append(0.85) # ChrMarine@WalkCircleRt01_smooth 0
points0.append(0.1) # ChrMarine@WalkTightCircleLf01_smooth 0
points0.append(1.0) # ChrMarine@WalkTightCircleRt01_smooth 0
points0.append(0.7) # ChrMarine@Turn360Lf01 0
points0.append(0.1) # ChrMarine@Turn360Rt01 0
points0.append(0.45) # ChrMarine@StrafeFastLf01_smooth 0
points0.append(0.1) # ChrMarine@StrafeFastRt01_smooth 0

statemarineLocomotion.addCorrespondencePoints(motions, points0)
points1 = DoubleVec()
points1.append(0.7) # ChrMarine@Idle01 1
points1.append(0.75) # ChrMarine@Jog01 1
points1.append(1.45) # ChrMarine@Meander01 1
points1.append(0.55) # ChrMarine@Run01 1
points1.append(0.85) # ChrMarine@RunCircleLf01_smooth 1
points1.append(0.4) # ChrMarine@RunCircleRt01_smooth 1
points1.append(0.4) # ChrMarine@RunTightCircleLf01 1
points1.append(0.74) # ChrMarine@RunTightCircleRt01 1
points1.append(0.75) # ChrMarine@StrafeSlowLf01 1
points1.append(1.3) # ChrMarine@StrafeSlowRt01 1
points1.append(1.25) # ChrMarine@Walk01 1
points1.append(0.85) # ChrMarine@WalkCircleLf01_smooth 1
points1.append(1.55) # ChrMarine@WalkCircleRt01_smooth 1
points1.append(1.0) # ChrMarine@WalkTightCircleLf01_smooth 1
points1.append(1.6) # ChrMarine@WalkTightCircleRt01_smooth 1
points1.append(1.2) # ChrMarine@Idle01 1
points1.append(0.7) # ChrMarine@Jog01 1
points1.append(0.85) # ChrMarine@StrafeFastLf01_smooth 1
points1.append(0.45) # ChrMarine@StrafeFastRt01_smooth 1

statemarineLocomotion.addCorrespondencePoints(motions, points1)
points2 = DoubleVec()
points2.append(1.25) # ChrMarine@Idle01 2
points2.append(1.1) # ChrMarine@Jog01 2
points2.append(1.95) # ChrMarine@Meander01 2
points2.append(0.85) # ChrMarine@Run01 2
points2.append(1.25) # ChrMarine@RunCircleLf01_smooth 2
points2.append(0.85) # ChrMarine@RunCircleRt01_smooth 2
points2.append(0.74) # ChrMarine@RunTightCircleLf01 2
points2.append(1.1) # ChrMarine@RunTightCircleRt01 2
points2.append(1.3) # ChrMarine@StrafeSlowLf01 2
points2.append(1.85) # ChrMarine@StrafeSlowRt01 2
points2.append(1.80) # ChrMarine@Walk01 2
points2.append(1.55) # ChrMarine@WalkCircleLf01_smooth 2
points2.append(2.2) # ChrMarine@WalkCircleRt01_smooth 2
points2.append(1.6) # ChrMarine@WalkTightCircleLf01_smooth 2
points2.append(2.45) # ChrMarine@WalkTightCircleRt01_smooth 2
points2.append(1.75) # ChrMarine@Idle01 2
points2.append(1.2) # ChrMarine@Jog01 2
points2.append(1.2) # ChrMarine@StrafeFastLf01_smooth 2
points2.append(0.85) # ChrMarine@StrafeFastRt01_smooth 2



statemarineLocomotion.addCorrespondencePoints(motions, points2)
points3 = DoubleVec()
points3.append(1.80) # ChrMarine@Idle01 3
points3.append(1.45) # ChrMarine@Jog01 3
points3.append(2.5333) # ChrMarine@Meander01 3
points3.append(1.166) # ChrMarine@Run01 3
points3.append(1.666) # ChrMarine@RunCircleLf01_smooth 3
points3.append(1.25) # ChrMarine@RunCircleRt01_smooth 3
points3.append(1.1) # ChrMarine@RunTightCircleLf01 3
points3.append(1.5) # ChrMarine@RunTightCircleRt01 3
points3.append(1.85) # ChrMarine@StrafeSlowLf01 3
points3.append(2.4) # ChrMarine@StrafeSlowRt01 3
points3.append(2.216) # ChrMarine@Walk01 3
points3.append(2.2) # ChrMarine@WalkCircleLf01_smooth 3
points3.append(2.75) # ChrMarine@WalkCircleRt01_smooth 3
points3.append(2.45) # ChrMarine@WalkTightCircleLf01_smooth 3
points3.append(3.03) # ChrMarine@WalkTightCircleRt01_smooth 3
points3.append(2.23) # ChrMarine@Idle01 3
points3.append(1.75) # ChrMarine@Jog01 3
points3.append(1.70) # ChrMarine@StrafeFastLf01_smooth 3
points3.append(1.20) # ChrMarine@StrafeFastRt01_smooth 3

statemarineLocomotion.addCorrespondencePoints(motions, points3)
points4 = DoubleVec()
points4.append(2.27) # ChrMarine@Idle01 4
points4.append(1.80) # ChrMarine@Jog01 4
points4.append(3.0833) # ChrMarine@Meander01 4
points4.append(1.366) # ChrMarine@Run01 4
points4.append(1.966) # ChrMarine@RunCircleLf01_smooth 4
points4.append(1.666) # ChrMarine@RunCircleRt01_smooth 4
points4.append(1.5) # ChrMarine@RunTightCircleLf01 4
points4.append(1.8) # ChrMarine@RunTightCircleRt01 4
points4.append(2.4) # ChrMarine@StrafeFastLf01_smooth 4
points4.append(3.05) # ChrMarine@StrafeFastRt01_smooth 4
points4.append(2.71666) # ChrMarine@Walk01 4
points4.append(2.75) # ChrMarine@WalkCircleLf01_smooth 4
points4.append(3.45) # ChrMarine@WalkCircleRt01_smooth 4
points4.append(3.03) # ChrMarine@WalkTightCircleLf01_smooth 4
points4.append(3.93) # ChrMarine@WalkTightCircleRt01_smooth 4
points4.append(2.83) # ChrMarine@Idle01 4
points4.append(2.23) # ChrMarine@Jog01 4
points4.append(2.05) # ChrMarine@StrafeSlowLf01 4
points4.append(1.70) # ChrMarine@StrafeSlowRt01 4

statemarineLocomotion.addCorrespondencePoints(motions, points4)


stanceArray = []
stanceArray.append([0.2, 0.7, 1.25, 1.80]) # idle
stanceArray.append([0.4, 0.05,1.1,0.75]) # jog
stanceArray.append([0.75,0.2, 1.95, 1.45]) # meander
stanceArray.append([0.3, 0.1, 0.85, 0.55]) # run
stanceArray.append([0.4, 0.1, 1.25, 0.85]) # run circle lf
stanceArray.append([0.1, 0.4, 0.85, 1.25]) # run circle rt
stanceArray.append([0.1, 0.4, 0.74, 1.10]) # run tight circle lf
stanceArray.append([0.4, 0.1, 1.10, 0.74]) # run tight circle rt
stanceArray.append([0.1, 0.75, 1.3, 1.85]) # strafe slow lf
stanceArray.append([0.75, 0.1, 1.85, 1.3]) # strafe slow rt
stanceArray.append([0.65, 0.15, 1.80, 1.25]) # walk
stanceArray.append([0.15, 0.85, 1.55, 2.20]) # walk circle lf
stanceArray.append([0.85, 0.15, 2.20, 1.55]) # walk circle rt
stanceArray.append([0.1, 1.0, 1.6, 2.45]) # walk tight circle lf
stanceArray.append([1.0, 0.1, 2.45, 1.6]) # walk tight circle rt
stanceArray.append([0.7, 0.1, 1.75, 1.20]) # turn 360 lf
stanceArray.append([0.1, 0.7, 1.20, 1.75]) # turn 360 rt	
stanceArray.append([0.45, 0.1, 1.20, 0.85]) # strafe fast lf
stanceArray.append([0.1, 0.45, 0.85, 1.20]) # strafe fast rt

liftArray = []
liftArray.append([0.7, 0.2, 1.80, 1.25]) # idle
liftArray.append([0.55, 0.22,1.22,0.92]) # jog
liftArray.append([0.2, 0.75, 1.45, 1.95]) # meander
liftArray.append([0.4, 0.2, 0.95, 0.65]) # run
liftArray.append([0.5, 0.2, 1.35, 0.95]) # run circle lf
liftArray.append([0.2, 0.5, 0.95, 1.35]) # run circle rt
liftArray.append([0.2, 0.5, 0.84, 1.20]) # run tight circle lf
liftArray.append([0.5, 0.2, 1.20, 0.84]) # run tight circle rt
liftArray.append([0.75, 0.1, 1.85, 1.3]) # strafe slow lf
liftArray.append([0.1, 0.75, 1.3, 1.85]) # strafe slow rt
liftArray.append([0.15, 0.65, 1.25,  1.80]) # walk
liftArray.append([0.85, 0.15, 2.20, 1.55]) # walk circle lf
liftArray.append([0.15, 0.85, 1.55, 2.20]) # walk circle rt
liftArray.append([1.0, 0.1, 2.45, 1.6]) # walk tight circle lf
liftArray.append([0.1, 1.0, 1.6, 2.45]) # walk tight circle rt
liftArray.append([0.1, 0.7, 1.20, 1.75]) # turn 360 lf
liftArray.append([0.7, 0.1, 1.75, 1.20]) # turn 360 rt	
liftArray.append([0.1, 0.45, 0.85, 1.20]) # strafe fast lf
liftArray.append([0.45, 0.1, 1.20, 0.85]) # strafe fast rt
	

for i in range(0, len(motions)):
	motionName = motions[i]
	motion = scene.getMotion(motionName)
	motionDuration = motion.getDuration()		
	for j in range(0,2):
		s1 = stanceArray[i][0+j]
		s2 = stanceArray[i][2+j]
		leg = j
		statemarineLocomotion.addKeyTagValue(motionName,leg,"stance",s1);
		statemarineLocomotion.addKeyTagValue(motionName,leg,"stance",s2);	
		lift1 = liftArray[i][0+j]%motionDuration
		lift2 = liftArray[i][2+j]%motionDuration
		statemarineLocomotion.addKeyTagValue(motionName,leg,"lift",lift1);
		statemarineLocomotion.addKeyTagValue(motionName,leg,"lift",lift2);	
		land1 = (s2-0.05)%motionDuration
		land2 = (s1-0.05)%motionDuration
		statemarineLocomotion.addKeyTagValue(motionName,leg,"land",land1);
		statemarineLocomotion.addKeyTagValue(motionName,leg,"land",land2);	
		off1 = (lift1+0.1)%motionDuration
		off2 = (lift2+0.1)%motionDuration
		statemarineLocomotion.addKeyTagValue(motionName,leg,"lift-off",off1);
		statemarineLocomotion.addKeyTagValue(motionName,leg,"lift-off",off2);
		str1 = (land1-0.15)%motionDuration
		str2 = (land2-0.15)%motionDuration
		statemarineLocomotion.addKeyTagValue(motionName,leg,"strike",str1);
		statemarineLocomotion.addKeyTagValue(motionName,leg,"strike",str2);	

statemarineLocomotion.buildMotionAnalysis("ChrBackovic.sk","base",motions, '');		

skeleton = scene.getSkeleton("ChrBackovic.sk")
joint = skeleton.getJointByName("base")
travelDirection = ([0, 0, 0, 0, 0, 0, 0, 0, -90, 90, 0, 0, 0, 0 ,0, 0, 0, -90, 90 ])

for i in range(0, len(motions)):
	motion = scene.getMotion(motions[i])
	motion.connect(skeleton)
	correspondancePoints = statemarineLocomotion.getCorrespondancePoints(i)
	lenCorrespondancePoints = len(correspondancePoints)
	#speed = motion.getJointSpeed(joint, correspondancePoints[0], correspondancePoints[lenCorrespondancePoints - 1])
	#omega = motion.getJointAngularSpeed(joint, correspondancePoints[0], correspondancePoints[lenCorrespondancePoints - 1])
	speed = motion.getJointSpeed(joint, 0, motion.getDuration())
	#speedZ = motion.getJointSpeedAxis(joint, "Z", 0, motion.getDuration())
	#if (travelDirection[i] != 0):
	#	direction = motion.getJointSpeedAxis(joint, "X", 0, motion.getDuration())
	#else:
	direction = travelDirection[i]
	
	omega = motion.getJointAngularSpeed(joint, 0, motion.getDuration())
	
	statemarineLocomotion.setParameter(motions[i], speed, omega, direction)
	print "Motion " + motion.getName() + " (" + str(speed) + ", " + str(omega) + ", " + str(direction) + ")"
	motion.disconnect()
	
	
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeSlowRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeSlowRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastRt01_smooth")

statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeSlowLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeSlowLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastLf01_smooth")

statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeSlowRt01", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeSlowRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeSlowRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastRt01_smooth")

statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeSlowLf01", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeSlowLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeSlowLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleRt01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastLf01_smooth")
statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastLf01_smooth")

	
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunCircleLf01_smooth", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
#statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Run01", "ChrMarine@Turn360Lf01")

# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleLf01_smooth", "ChrMarine@WalkTightCircleLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunCircleLf01_smooth", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@StrafeFastRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Run01", "ChrMarine@RunCircleLf01_smooth", "ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkTightCircleLf01_smooth", "ChrMarine@Turn360Lf01")
#statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleLf01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Run01", "ChrMarine@Turn360Lf01")

# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunCircleRt01_smooth", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastLf01_smooth", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")

# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@Meander01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Meander01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01", "ChrMarine@WalkCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkCircleRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunCircleRt01_smooth", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@Walk01")
# statemarineLocomotion.addTetrahedron("ChrMarine@Jog01", "ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@StrafeFastRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Run01", "ChrMarine@RunCircleRt01_smooth", "ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth")
# statemarineLocomotion.addTetrahedron("ChrMarine@Idle01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")
# statemarineLocomotion.addTetrahedron("ChrMarine@RunTightCircleRt01", "ChrMarine@StrafeFastRt01_smooth", "ChrMarine@WalkTightCircleRt01_smooth", "ChrMarine@Turn360Rt01")

