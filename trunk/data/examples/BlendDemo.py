print "|--------------------------------------------|"
print "|           Starting Blend Demo              |"
print "|--------------------------------------------|"
# Blend walks and jumps etc

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
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(10000)
camera.setNearPlane(1)
camera.setAspectRatio(1.02632)

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

# Add Blend script
scene.run('Blend.py')

motionList = []
motionList.append('ChrUtah_WalkInCircleRight001')
motionList.append('ChrUtah_Jog001')
motionList.append('ChrUtah_RunJumpRun01')
motionList.append('ChrUtah_Relax001_CrouchProtectHead_right')

create0DBlend('0DBlend', motionList)
create1DBlend('1DBlend', motionList)
create2DBlend('2DBlend', motionList)
create3DBlend('3DBlend', motionList)

last = 0
canTime = True
delay = 5
played = False
class Blend(SBScript):
	def update(self, time):
		global canTime, last, played
		if canTime:
			last = time
			canTime = False
		diff = time - last
		
		if diff > 5 and not played:
			bml.execBML('elder', '<blend name="0DBlend"/>')
			bml.execBML('brad', '<blend name="1DBlend" x=".8"/>')
			bml.execBML('doctor', '<blend name="2DBlend" x=".12" y=".65"/>')
			bml.execBML('utah', '<blend name="3DBlend" x=".5" y=".35" z=".7"/>')
			played = True
		
		# Maintain position and facing direction
		setPos('brad', SrVec(-55, 102, 0))
		setPos('utah', SrVec(155, 0, 0))
		setPos('elder', SrVec(-155, 102, 0))
		setPos('doctor', SrVec(55, 102, 0))
		'''
		setFacing('brad', 0)
		setFacing('utah', 0)
		setFacing('elder', 0)
		setFacing('doctor', 0)
		'''
scene.removeScript('blend')
blend = Blend()
scene.addScript('blend', blend)