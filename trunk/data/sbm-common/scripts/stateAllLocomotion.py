
def locomotionSetup(skeleton, baseJoint):
	print "** State: allLocomotion"

	mirrorMotion1 = scene.getMotion("ChrUtah_Idle01_ToWalk01_Turn90Lf01")
	mirrorMotion1.mirror("ChrUtah_Idle01_ToWalk01_Turn90Rt01")
	mirrorMotion2 = scene.getMotion("ChrUtah_Idle01_ToWalk01_Turn180Lf01")
	mirrorMotion2.mirror("ChrUtah_Idle01_ToWalk01_Turn180Rt01")

	stateManager = scene.getStateManager()
	state = stateManager.createState3D("allLocomotion")

	# add motions
	motions = StringVec()
	motions.append("ChrUtah_Walk001")
	motions.append("ChrUtah_Idle001")
	motions.append("ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	motions.append("ChrUtah_Idle01_ToWalk01_Turn360Rt01")
	motions.append("ChrUtah_Meander01")
	motions.append("ChrUtah_Shuffle01")
	motions.append("ChrUtah_Jog001")
	motions.append("ChrUtah_Run001")
	motions.append("ChrUtah_WalkInCircleLeft001")
	motions.append("ChrUtah_WalkInCircleRight001")
	motions.append("ChrUtah_WalkInTightCircleLeft001")
	motions.append("ChrUtah_WalkInTightCircleRight001")
	motions.append("ChrUtah_RunInCircleLeft001")
	motions.append("ChrUtah_RunInCircleRight001")
	motions.append("ChrUtah_RunInTightCircleLeft01")
	motions.append("ChrUtah_RunInTightCircleRight01")
	motions.append("ChrUtah_StrafeSlowRt01")
	motions.append("ChrUtah_StrafeSlowLf01")
	motions.append("ChrUtah_StrafeFastRt01")
	motions.append("ChrUtah_StrafeFastLf01")
	for i in range(0, len(motions)):
		state.addMotion(motions[i], 0, 0, 0)


	# correspondance points
	floatarray = ([0, 0.811475, 1.34262, 2.13333, 0, 0.698851, 1.37931, 2.13333, 0, 0.82023, 1.30207, 2.12966, 0, 0.812874, 1.35356, 2.12966, 0, 0.988525, 1.87377, 3.06667, 0, 0.713115, 1.29836, 2.13333, 0, 0.501639, 0.92459, 1.6, 0, 0.422951, 0.772131, 1.33333, 0, 0.840984, 1.39672, 2.13333, 0, 0.840984, 1.29836, 2.13333, 0, 0.845902, 1.30328, 2.13333, 0, 0.880328, 1.33279, 2.13333, 0, 0.437705, 0.811475, 1.33333, 0, 0.452459, 0.791803, 1.33333, 0, 0.462295, 0.757377, 1.33333, 0, 0.452459, 0.796721, 1.33333, 0, 0.90000, 1.41000, 2.13000, 0.72, 1.38, 1.92, 0.72, 0, 0.473684, 0.920079, 1.6, 0.4, 0.893233, 1.28421, 0.4])
	numCorrespondancePoints = 4
	if (len(floatarray) != 4 * len(motions)):
		print "**Correspondance points input wrong"
		
	for i in range(0, numCorrespondancePoints):
		points = DoubleVec()
		for j in range(0, len(motions)):
			points.append(floatarray[j * numCorrespondancePoints + i])
		state.addCorrespondancePoints(motions, points)
		
	# reset parameters (because it needs context of correspondance points, extract parameters from motion)
	skeleton = scene.getSkeleton(skeleton)
	joint = skeleton.getJointByName(baseJoint)

	travelDirection = ([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, -90, 90, -90])
	for i in range(0, len(motions)):
		motion = scene.getMotion(motions[i])
		motion.connect(skeleton)
		correspondancePoints = state.getCorrespondancePoints(i)
		lenCorrespondancePoints = len(correspondancePoints)
		speed = motion.getJointSpeed(joint, correspondancePoints[0], correspondancePoints[lenCorrespondancePoints - 1])
		omega = motion.getJointAngularSpeed(joint, correspondancePoints[0], correspondancePoints[lenCorrespondancePoints - 1])
		direction = travelDirection[i]
		state.setParameter(motions[i], speed, omega, direction)
		motion.disconnect()
										 
	# add tetrahedrons (need automatic way to generate)
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_Meander01", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_StrafeSlowRt01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_Meander01", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_StrafeSlowLf01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_Meander01", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_StrafeSlowRt01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_Meander01", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_StrafeSlowLf01")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Walk001", "ChrUtah_Jog001", "ChrUtah_WalkInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Walk001", "ChrUtah_Jog001", "ChrUtah_WalkInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Jog001", "ChrUtah_Run001", "ChrUtah_RunInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Jog001", "ChrUtah_Run001", "ChrUtah_RunInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInCircleLeft001", "ChrUtah_RunInCircleLeft001", "ChrUtah_Jog001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInCircleRight001", "ChrUtah_RunInCircleRight001", "ChrUtah_Jog001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInCircleLeft001", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_RunInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInCircleRight001", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_RunInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_RunInCircleLeft001", "ChrUtah_RunInTightCircleLeft01")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_RunInCircleRight001", "ChrUtah_RunInTightCircleRight01")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Meander01", "ChrUtah_Walk001", "ChrUtah_WalkInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Meander01", "ChrUtah_WalkInCircleLeft001", "ChrUtah_WalkInTightCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Meander01", "ChrUtah_Walk001", "ChrUtah_WalkInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_Meander01", "ChrUtah_WalkInCircleRight001", "ChrUtah_WalkInTightCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Walk001", "ChrUtah_Jog001", "ChrUtah_WalkInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Walk001", "ChrUtah_Jog001", "ChrUtah_WalkInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Jog001", "ChrUtah_Run001", "ChrUtah_RunInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Jog001", "ChrUtah_Run001", "ChrUtah_RunInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInCircleLeft001", "ChrUtah_RunInCircleLeft001", "ChrUtah_Jog001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInCircleRight001", "ChrUtah_RunInCircleRight001", "ChrUtah_Jog001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInCircleLeft001", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_RunInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInCircleRight001", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_RunInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_RunInCircleLeft001", "ChrUtah_RunInTightCircleLeft01")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_RunInCircleRight001", "ChrUtah_RunInTightCircleRight01")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Meander01", "ChrUtah_Walk001", "ChrUtah_WalkInCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Meander01", "ChrUtah_WalkInCircleLeft001", "ChrUtah_WalkInTightCircleLeft001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Meander01", "ChrUtah_Walk001", "ChrUtah_WalkInCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_Meander01", "ChrUtah_WalkInCircleRight001", "ChrUtah_WalkInTightCircleRight001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_StrafeFastRt01", "ChrUtah_RunInCircleRight001", "ChrUtah_RunInTightCircleRight01")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_StrafeFastRt01", "ChrUtah_RunInCircleRight001", "ChrUtah_Run001")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_StrafeFastRt01", "ChrUtah_RunInCircleLeft001", "ChrUtah_RunInTightCircleLeft01")
	state.addTetrahedron("ChrUtah_StrafeSlowRt01", "ChrUtah_StrafeFastRt01", "ChrUtah_RunInCircleLeft001", "ChrUtah_Run001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_StrafeFastLf01", "ChrUtah_RunInCircleRight001", "ChrUtah_RunInTightCircleRight01")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_StrafeFastLf01", "ChrUtah_RunInCircleRight001", "ChrUtah_Run001")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_StrafeFastLf01", "ChrUtah_RunInCircleLeft001", "ChrUtah_RunInTightCircleLeft01")
	state.addTetrahedron("ChrUtah_StrafeSlowLf01", "ChrUtah_StrafeFastLf01", "ChrUtah_RunInCircleLeft001", "ChrUtah_Run001")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowLf01", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_Idle01_ToWalk01_Turn360Rt01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowLf01", "ChrUtah_Idle001", "ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowLf01", "ChrUtah_Idle001", "ChrUtah_Idle01_ToWalk01_Turn360Rt01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInTightCircleLeft001", "ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowRt01", "ChrUtah_WalkInTightCircleRight001", "ChrUtah_Idle01_ToWalk01_Turn360Rt01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowRt01", "ChrUtah_Idle001", "ChrUtah_Idle01_ToWalk01_Turn360Lf01")
	state.addTetrahedron("ChrUtah_Shuffle01", "ChrUtah_StrafeSlowRt01", "ChrUtah_Idle001", "ChrUtah_Idle01_ToWalk01_Turn360Rt01")

locomotionSetup("common.sk", "base")