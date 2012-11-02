import random


print "|--------------------------------------------|"
print "|           Starting Crowd Demo              |"
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
camera.setEye(0, 2259.11, 1870.06)
camera.setCenter(0, 2099.85, 1750.36)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addMultipleCharacters('utah', 'utah', 40)

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Set up list of Utahs and Doctors
utahList = []
for name in scene.getCharacterNames():
	if 'utah' in name:
		utahList.append(scene.getCharacter(name))

# Add Locomotion script
scene.run('Locomotion.py')

# Whether character has reached its target
utahReached = True

# Paths for characters
utahPath = [SrVec(-1000, -1000, 0), SrVec(1000, 1000, 0), SrVec(1000, -1000, 0), SrVec(-1000, 1000, 0)]
utahCur = 0
pathAmt = len(utahPath)

class CrowdDemo(SBScript):
	def update(self, time):
		global utahReached, utahCur
		# Once utah completes path, do again
		if utahReached:
			for utah in utahList:
				move(utah.getName(), utahPath[utahCur], random.uniform(1.2, 5))
			utahCur = utahCur + 1
			# If reaches max path, reset
			if utahCur >= pathAmt:
				utahCur = 0
			utahReached = False
		
# Run the update script
scene.removeScript('crowddemo')
crowddemo = CrowdDemo()
scene.addScript('crowddemo', crowddemo)

reachCount = 0
# Locomotion handler to check if characters have arrived
class LocomotionHandler(EventHandler):
	def executeAction(self, ev):
		global utahReached, reachCount
		params = ev.getParameters()
		if 'success' in params:
			if 'utah' in params:
				reachCount = reachCount + 1
				if reachCount >= 15:
					utahReached = True	
					reachCount = 0

locomotionHdl = LocomotionHandler()
evtMgr = scene.getEventManager()
evtMgr.addEventHandler('locomotion', locomotionHdl)