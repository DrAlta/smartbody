mirrorMotion = scene.getMotion("ChrMarine@Turn90Rt01")
mirrorMotion.mirror("ChrMarine@Turn90Lf01", "ChrBackovic.sk")
mirrorMotion1 = scene.getMotion("ChrMarine@Turn180Rt01")
mirrorMotion1.mirror("ChrMarine@Turn180Lf01", "ChrBackovic.sk")


stateManager = scene.getStateManager()

#print "** State: ChrMarineIdleTurn"
state = stateManager.createState1D("mocapIdleTurn")
state.setBlendSkeleton('ChrBackovic.sk')
motions = StringVec()
motions.append("ChrMarine@Idle01")
motions.append("ChrMarine@Turn90Lf01")
motions.append("ChrMarine@Turn180Lf01")
motions.append("ChrMarine@Turn90Rt01")
motions.append("ChrMarine@Turn180Rt01")
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
points2.append(0.255738)
points2.append(0.762295)
points2.append(0.87541)
points2.append(0.757377)
points2.append(0.821311)
state.addCorrespondancePoints(motions, points2)
points3 = DoubleVec()
points3.append(2.17)
points3.append(2.13)
points3.append(1.73)
points3.append(2.13)
points3.append(1.73)
state.addCorrespondancePoints(motions, points3)
