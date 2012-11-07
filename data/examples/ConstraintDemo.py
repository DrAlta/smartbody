print "|--------------------------------------------|"
print "|          Starting Constraint Demo          |"
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
camera.setEye(0, 345.99, 502.63)
camera.setCenter(0, 246.80, 329.83)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah0', 'utah', False)
setPos('utah0', SrVec(55, 0, 0))
addCharacter('utah1', 'utah', False)
setPos('utah1', SrVec(145, 0, 0))
# Characters that need reach 
addCharacter('utah2', 'utah', True)
setPos('utah2', SrVec(-145, 0, 0))
addCharacter('utah3', 'utah', True)
setPos('utah3', SrVec(-55, 0, 0))

# Add pawns in scene
addPawn('bradPawn', 'sphere', SrVec(0, 0, 0))
setPawnPos('bradPawn', SrVec(101.28, 111.45, 17.64))
addPawn('utahPawn', 'sphere', SrVec(0, 0, 0))
setPawnPos('utahPawn', SrVec(176.66, 107.13, 36.86))
addPawn('gazePawn1', 'sphere')
setPawnPos('gazePawn1', SrVec(-200, 175, 150))
addPawn('touchPawn1', 'sphere', SrVec(5, 5, 5))
setPawnPos('touchPawn1', SrVec(-178, 155, 43))
addPawn('touchPawn2', 'sphere', SrVec(5, 5, 5))
setPawnPos('touchPawn2', SrVec(-88, 155, 43))

# Set camera position
setPawnPos('camera', SrVec(0, 50, 0))

# Add reach and gaze script
scene.run('Reach.py')
scene.run('Gaze.py')

# Get utah2 and utah3 to touch the pawns
touch('utah2', 'touchPawn1', False, False)
touch('utah3', 'touchPawn2', False, False)

# Constraint joints at target locations
bml.execBMLAt(2, 'utah2', '<sbm:constraint effector="r_wrist" sbm:effector-root="r_sternoclavicular" sbm:handle="cutah2" target="touchPawn1" sbm:fade-in="0.5"/>')
bml.execBMLAt(2, 'utah1', '<sbm:constraint effector="l_wrist" sbm:effector-root="l_sternoclavicular" sbm:handle="cutah" target="utahPawn" sbm:fade-in="0.5"/>')
bml.execBMLAt(2, 'utah1', '<sbm:constraint effector="r_wrist" sbm:effector-root="r_sternoclavicular" sbm:handle="cutah" target="bradPawn" sbm:fade-in="0.5"/>')

# Make all character gaze at pawn
bml.execBMLAt(5, '*', '<gaze target="gazePawn1"/>')

# Variables to move pawn
gazeX = -200
dir = 1
speed = 0.2

class ConstraintDemo(SBScript):
	def update(self, time):
		global gazeX, dir, speed
		# Change direction when hit border
		if gazeX > 200:
			dir = -1
		elif gazeX < -200:
			dir = 1
		gazeX = gazeX + speed * dir
		setPawnPos('gazePawn1', SrVec(gazeX, 175, 150))
		
# Run the update script
scene.removeScript('constraintdemo')
constraintdemo = ConstraintDemo()
scene.addScript('constraintdemo', constraintdemo)