print "|--------------------------------------------|"
print "|             Starting Gaze Demo             |"
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
camera.setEye(0, 272, 375)
camera.setCenter(0, 173, 202)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(55, 0, 0))
addCharacter('brad', 'brad')
setPos('brad', SrVec(145, 102, 0))
# Characters that need reach 
addCharacter('elder', 'elder')
setPos('elder', SrVec(-145, 102, 0))
addCharacter('doctor', 'doctor')
setPos('doctor', SrVec(-55, 102, 0))

# Add pawns in scene
addPawn('gazePawn1', 'sphere')
setPawnPos('gazePawn1', SrVec(-200, 200, 200))
# Set camera position
setPawnPos('camera', SrVec(0, 50, 0))

# Add reach and gaze script
scene.run('Gaze.py')

# Make all character gaze at pawn
bml.execBML('utah', '<gaze sbm:joint-range="EYES NECK" target="gazePawn1"/>')
bml.execBML('brad', '<gaze sbm:joint-range="EYES CHEST" sbm:joint-speed="1200" target="gazePawn1"/>')
bml.execBML('elder', '<gaze sbm:joint-range="HEAD CHEST" sbm:joint-speed="800" sbm:priority-joint="HEAD" target="gazePawn1"/>')
bml.execBML('doctor', '<gaze sbm:joint-range="EYES BACK" sbm:joint-speed="1500" target="gazePawn1"/>')

# Variables to move pawn
gazeX = -200
gazeZ = 200
dirX = 1
dirZ = 1
speed = 0.1

class GazeDemo(SBScript):
	def update(self, time):
		global gazeX, gazeZ, dirX, dirZ, speed
		# Change direction when hit border
		if gazeX > 200:
			dirX = -1
		elif gazeX < -200:
			dirX = 1
		if gazeZ > 200:
			dirZ = -1
		elif gazeZ < -100:
			dirZ = 1
		gazeX = gazeX + speed * dirX
		gazeZ = gazeZ + speed * dirZ
		setPawnPos('gazePawn1', SrVec(gazeX, 200, gazeZ))
		
# Run the update script
scene.removeScript('gazedemo')
gazedemo = GazeDemo()
scene.addScript('gazedemo', gazedemo)