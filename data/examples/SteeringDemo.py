import random
print "|--------------------------------------------|"
print "|          Starting Steering Demo            |"
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
camera.setEye(0, 1549, 2447)
camera.setCenter(0, 1437, 2282)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addMultipleCharacters('utah', 'utah', 15, False, 900, 900)
addMultipleCharacters('brad', 'brad', 15, False, -900, -900)

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Set up list of Utahs and Doctors
utahList = []
bradList = []
for name in scene.getCharacterNames():
	if 'utah' in name:
		utahList.append(scene.getCharacter(name))
	if 'brad' in name:
		bradList.append(scene.getCharacter(name))

# Add Locomotion script
scene.run('Locomotion.py')

# Paths for characters
utahPath = [SrVec(-900, -900, 0), SrVec(900, 900, 0)]
bradPath = [SrVec(900, 900, 0), SrVec(-900, -900, 0)]
pathAmt = len(utahPath)

# Enable collision
collisionManager = getScene().getCollisionManager()
collisionManager.setBoolAttribute('singleChrCapsuleMode', True)
collisionManager.setStringAttribute('collisionResolutionType', 'default')
collisionManager.setEnable(True)

bradCur = 0
utahCur = 0
canTime = True
last = 0
delay = 20
class LocomotionDemo(SBScript):
	def update(self, time):
		global bradCur, utahCur, canTime, last
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		# When time's up, do action
		if canTime:
			last = time
			canTime = False
			# Move all utah
			for utah in utahList:
				move(utah.getName(), utahPath[utahCur], random.uniform(1, 5))
			utahCur = utahCur + 1
			# If reaches max path, reset
			if utahCur >= pathAmt:
				utahCur = 0
			# Move all brad 
			for brad in bradList:
				move(brad.getName(), bradPath[bradCur], random.uniform(1, 5))
			bradCur = bradCur + 1
			# If reaches max path, reset
			if bradCur >= pathAmt:
				bradCur = 0
			
# Run the update script
scene.removeScript('locomotiondemo')
locomotiondemo = LocomotionDemo()
scene.addScript('locomotiondemo', locomotiondemo)