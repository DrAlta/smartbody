print "|--------------------------------------------|"
print "|           Starting Blend Demo              |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath("script","../../../../data/examples")
scene.addAssetPath("script","../../../../data/examples/functions")
scene.addAssetPath("script","../../../../data/sbm-common/scripts")
scene.addAssetPath('seq', '../../../../data/sbm-common/scripts')
scene.addAssetPath('seq', '../../../../data/sbm-test/scripts')
scene.addAssetPath('mesh', '../../../../data/mesh')
scene.addAssetPath('mesh', '../../../../data/retarget/mesh')
scene.addAssetPath('audio', '../../../../data/Resources/audio')

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 233.87, 354.69)
camera.setCenter(0, 159.87, 169.69)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad')
setPos('brad', SrVec(-55, 102, 0))
addCharacter('utah', 'utah')
setPos('utah', SrVec(145, 0, 0))
addCharacter('elder', 'elder')
setPos('elder', SrVec(-145, 102, 0))
addCharacter('doctor', 'doctor')
setPos('doctor', SrVec(55, 102, 0))

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Add Blend script
scene.run('Blend.py')

blend0D = blendManager.createBlend0D('blend0D')
motions = StringVec()
motions.append('ChrUtah_WalkInCircleRight001')
blend0D.addMotion(motions[0])

# 1D Blend
blend1D = blendManager.createBlend1D('blend1D')
motions = StringVec()
motions.append('ChrUtah_Idle001')
motions.append('ChrUtah_Turn90Lf01')
motions.append('ChrUtah_Turn180Lf01')
motions.append('ChrUtah_Turn90Rt01')
motions.append('ChrUtah_Turn180Rt01')

paramsX = DoubleVec()
paramsX.append(0) # ChrUtah_Idle001 X
paramsX.append(-90) # ChrUtah_Turn90Lf01 X
paramsX.append(-180) # ChrUtah_Turn180Lf01 X
paramsX.append(90) # ChrUtah_Turn90Rt01 X
paramsX.append(180) # ChrUtah_Turn180Rt01 X
for i in range(0, len(motions)):
	blend1D.addMotion(motions[i], paramsX[i])

points0 = DoubleVec()
points0.append(0) # ChrUtah_Idle001 0
points0.append(0) # ChrUtah_Turn90Lf01 0
points0.append(0) # ChrUtah_Turn180Lf01 0
points0.append(0) # ChrUtah_Turn90Rt01 0
points0.append(0) # ChrUtah_Turn180Rt01 0
blend1D.addCorrespondencePoints(motions, points0)
points1 = DoubleVec()
points1.append(0.255738) # ChrUtah_Idle001 1
points1.append(0.762295) # ChrUtah_Turn90Lf01 1
points1.append(0.87541) # ChrUtah_Turn180Lf01 1
points1.append(0.757377) # ChrUtah_Turn90Rt01 1
points1.append(0.821311) # ChrUtah_Turn180Rt01 1
blend1D.addCorrespondencePoints(motions, points1)
points2 = DoubleVec()
points2.append(0.633333) # ChrUtah_Idle001 2
points2.append(1.96667) # ChrUtah_Turn90Lf01 2
points2.append(2.46667) # ChrUtah_Turn180Lf01 2
points2.append(1.96667) # ChrUtah_Turn90Rt01 2
points2.append(2.46667) # ChrUtah_Turn180Rt01 2
blend1D.addCorrespondencePoints(motions, points2)

# 2D Blend
blend2D = blendManager.createBlend2D("blend2D")

motions = StringVec()
motions.append("ChrUtah_Idle001")
motions.append("ChrUtah_Idle01_StepBackwardRt01")
motions.append("ChrUtah_Idle01_StepForwardRt01")
motions.append("ChrUtah_Idle01_StepSidewaysRt01")
motions.append("ChrUtah_Idle01_StepBackwardLf01")
motions.append("ChrUtah_Idle01_StepForwardLf01")
motions.append("ChrUtah_Idle01_StepSidewaysLf01")

paramsX = DoubleVec()
paramsY = DoubleVec()
paramsX.append(0) # ChrUtah_Idle001 X
paramsY.append(0) # ChrUtah_Idle001 Y
paramsX.append(-0.0275919) # ChrUtah_Idle01_StepBackwardRt01 X
paramsY.append(-19.5057) # ChrUtah_Idle01_StepBackwardRt01 Y
paramsX.append(0.0240943) # ChrUtah_Idle01_StepForwardRt01 X
paramsY.append(45.4044) # ChrUtah_Idle01_StepForwardRt01 Y
paramsX.append(28.8772) # ChrUtah_Idle01_StepSidewaysRt01 X
paramsY.append(0.00321) # ChrUtah_Idle01_StepSidewaysRt01 Y
paramsX.append(-0.0212764) # ChrUtah_Idle01_StepBackwardLf01 X
paramsY.append(-39.203) # ChrUtah_Idle01_StepBackwardLf01 Y
paramsX.append(0.0480087) # ChrUtah_Idle01_StepForwardLf01 X
paramsY.append(47.8086) # ChrUtah_Idle01_StepForwardLf01 Y
paramsX.append(-31.7367) # ChrUtah_Idle01_StepSidewaysLf01 X
paramsY.append(0) # ChrUtah_Idle01_StepSidewaysLf01 Y
for i in range(0, len(motions)):
	blend2D.addMotion(motions[i], paramsX[i], paramsY[i])

points0 = DoubleVec()
points0.append(0) # ChrUtah_Idle001 0
points0.append(0) # ChrUtah_Idle01_StepBackwardRt01 0
points0.append(0) # ChrUtah_Idle01_StepForwardRt01 0
points0.append(0) # ChrUtah_Idle01_StepSidewaysRt01 0
points0.append(0) # ChrUtah_Idle01_StepBackwardLf01 0
points0.append(0) # ChrUtah_Idle01_StepForwardLf01 0
points0.append(0) # ChrUtah_Idle01_StepSidewaysLf01 0
blend2D.addCorrespondencePoints(motions, points0)
points1 = DoubleVec()
points1.append(0.556322) # ChrUtah_Idle001 1
points1.append(0.556322) # ChrUtah_Idle01_StepBackwardRt01 1
points1.append(0.543678) # ChrUtah_Idle01_StepForwardRt01 1
points1.append(0.482989) # ChrUtah_Idle01_StepSidewaysRt01 1
points1.append(0.395402) # ChrUtah_Idle01_StepBackwardLf01 1
points1.append(0.531034) # ChrUtah_Idle01_StepForwardLf01 1
points1.append(0.473563) # ChrUtah_Idle01_StepSidewaysLf01 1
blend2D.addCorrespondencePoints(motions, points1)
points2 = DoubleVec()
points2.append(1.46414) # ChrUtah_Idle001 2
points2.append(1.46414) # ChrUtah_Idle01_StepBackwardRt01 2
points2.append(1.46414) # ChrUtah_Idle01_StepForwardRt01 2
points2.append(1.46414) # ChrUtah_Idle01_StepSidewaysRt01 2
points2.append(1.33333) # ChrUtah_Idle01_StepBackwardLf01 2
points2.append(1.33333) # ChrUtah_Idle01_StepForwardLf01 2
points2.append(1.33103) # ChrUtah_Idle01_StepSidewaysLf01 2
blend2D.addCorrespondencePoints(motions, points2)

blend2D.addTriangle("ChrUtah_Idle001", "ChrUtah_Idle01_StepBackwardLf01", "ChrUtah_Idle01_StepSidewaysLf01")
blend2D.addTriangle("ChrUtah_Idle001", "ChrUtah_Idle01_StepForwardLf01", "ChrUtah_Idle01_StepSidewaysLf01")
blend2D.addTriangle("ChrUtah_Idle001", "ChrUtah_Idle01_StepBackwardRt01", "ChrUtah_Idle01_StepSidewaysRt01")
blend2D.addTriangle("ChrUtah_Idle001", "ChrUtah_Idle01_StepForwardRt01", "ChrUtah_Idle01_StepSidewaysRt01")

last = 0
canTime = True
delay = 5
played = False
class BlendDemo(SBScript):
	def update(self, time):
		global canTime, last, played
		if canTime:
			last = time
			canTime = False
		diff = time - last
		# Start blend after delay
		if diff > delay and not played:
			bml.execBML('elder', '<blend name="blend0D"/>')
			bml.execBML('brad', '<blend name="blend1D" x="-90"/>')
			bml.execBML('doctor', '<blend name="blend2D" x="9.16" y="7.21"/>')
			#bml.execBML('utah', '<blend name="blend3D" x=".5" y=".35" z=".7"/>')
			played = True
		
# Run the update script
scene.removeScript('blenddemo')
blenddemo = BlendDemo()
scene.addScript('blenddemo', blenddemo)