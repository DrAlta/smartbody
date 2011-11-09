stateManager = scene.getStateManager()

print "** State: allIdleTurnLeft"
state1 = stateManager.createState0D("allIdleTurnLeft")
motions1 = StringVec()
motions1.append("ChrUtah_LiftFootLf01")
motions1.append("ChrUtah_Turn90Lf01")
motions1.append("ChrUtah_Turn180Lf01")

for i in range(0, len(motions1)):
	state1.addMotion(motions1[i])

points1 = DoubleVec()
points1.append(0)
points1.append(0)
points1.append(0)
state1.addCorrespondancePoints(motions1, points1)
points2 = DoubleVec()
points2.append(0.255738)
points2.append(0.762295)
points2.append(0.87541)
state1.addCorrespondancePoints(motions1, points2)
points3 = DoubleVec()
points3.append(0.633333)
points3.append(1.96667)
points3.append(2.46667)
state1.addCorrespondancePoints(motions1, points3)

print "** State: allIdleTurnRight"
state2 = stateManager.createState0D("allIdleTurnRight")
motions2 = StringVec()
motions2.append("ChrUtah_LiftFootRt01")
motions2.append("ChrUtah_Turn90Rt01")
motions2.append("ChrUtah_Turn180Rt01")

for i in range(0, len(motions2)):
	state2.addMotion(motions2[i])

# Since the right is symetric with the left, so the correspondance points are the same
state2.addCorrespondancePoints(motions2, points1)
state2.addCorrespondancePoints(motions2, points2)
state2.addCorrespondancePoints(motions2, points3)

