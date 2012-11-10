import random
print "|--------------------------------------------|"
print "|         Starting Locomotion Demo           |"
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
addMultipleCharacters('utah', 'utah', 8, False, -1000, 1000)
addMultipleCharacters('doctor', 'doctor', 8, False, 1000, 1000)
addMultipleCharacters('elder', 'elder', 8, False, 1000, -1000)
addMultipleCharacters('brad', 'brad', 8, False, -1000, -1000)

# Add pawns in scene
addPawn('pawn0', 'sphere', SrVec(1, 1, 1))
addPawn('pawn1', 'box', SrVec(1, 1, 1))
setPawnPos('pawn0', SrVec(-1000, 0, -1000))
setPawnPos('pawn1', SrVec(-200, 0, 1000))
# Pawns work in x, y, z, Locomotion works in x, y

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Set up list of Utahs and Doctors
utahList = []
doctorList = []
elderList = []
bradList = []
for name in scene.getCharacterNames():
	if 'utah' in name:
		utahList.append(scene.getCharacter(name))
	if 'doctor' in name:
		doctorList.append(scene.getCharacter(name))
	if 'elder' in name:
		elderList.append(scene.getCharacter(name))
	if 'brad' in name:
		bradList.append(scene.getCharacter(name))

# Add Locomotion script
scene.run('Locomotion.py')

# Whether character has reached its target
utahReached = True
doctorReached = True
bradReached = True
elderReached  = True

# Paths for characters
utahPath = [SrVec(-1000, 1000, 0), SrVec(-200, -1000, 0), scene.getPawn('pawn1'), scene.getPawn('pawn0')]
doctorPath = [SrVec(1000, 1000, 0), SrVec(200, -1000, 0), SrVec(200, 1000, 0), SrVec(1000, -1000, 0)]
bradPath = list(utahPath)
bradPath.reverse()
elderPath = list(doctorPath)
elderPath.reverse()
doctorCur = elderCur = 0
pathAmt = len(doctorPath)

# Enable collision
collisionManager = getScene().getCollisionManager()
collisionManager.setStringAttribute('collisionResolutionType', 'default')
#collisionManager.setBoolAttribute('singleChrCapsuleMode', True)
collisionManager.setEnable(True)

class LocomotionDemo(SBScript):
	def update(self, time):
		global utahReached, doctorReached, bradReached, elderReached, doctorCur, elderCur
		# Once utah completes path, do again
		if utahReached:
			for utah in utahList:
				followPath(utah.getName(), utahPath)
			utahReached = False
		# Once doctor reaches 1 waypoint, move to next
		if doctorReached:
			for doctor in doctorList:
				move(doctor.getName(), doctorPath[doctorCur], random.uniform(1, 5))
			doctorCur = doctorCur + 1
			# If reaches max path, reset
			if doctorCur >= pathAmt:
				doctorCur = 0
			doctorReached = False
		# Once brad completes path, do again
		if bradReached:
			for brad in bradList:
				followPath(brad.getName(), bradPath)
			bradReached = False
		# Once elder reaches 1 waypoint, move to next
		if elderReached:
			for elder in elderList:
				move(elder.getName(), elderPath[elderCur], random.uniform(1, 5))
			elderCur = elderCur + 1
			if elderCur >= pathAmt:
				elderCur = 0
			elderReached = False
			
# Run the update script
scene.removeScript('locomotiondemo')
locomotiondemo = LocomotionDemo()
scene.addScript('locomotiondemo', locomotiondemo)

# Locomotion handler to check if characters have arrived
class LocomotionHandler(EventHandler):
	def executeAction(self, ev):
		global utahReached, doctorReached, bradReached, elderReached
		params = ev.getParameters()
		if 'success' in params:
			if 'utah0' in params:
				utahReached = True
			if 'doctor0' in params:
				doctorReached = True
			if 'brad0' in params:
				bradReached = True
			if 'elder0' in params:
				elderReached = True

locomotionHdl = LocomotionHandler()
evtMgr = scene.getEventManager()
evtMgr.addEventHandler('locomotion', locomotionHdl)