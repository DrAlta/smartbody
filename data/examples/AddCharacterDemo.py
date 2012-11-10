print "|--------------------------------------------|"
print "|         Starting Character Demo            |"
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
camera.setEye(0, 367.58, 574.66)
camera.setCenter(0, 275, 395)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(75, 0, 250))

# .dae
addCharacter('brad', 'brad')
setPos('brad', SrVec(-75, 102, 250))

# Multiple
addMultipleCharacters('elder', 'elder', 25)

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))
	
last = 0
canTime = True
delay = 3
class AddCharacterDemo(SBScript):
	def update(self, time):
		global canTime, last
		if canTime:
			last = time
			canTime = False
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		# If time's up, do action
		if canTime:
			bml.execBML('*', '<animation name="HandsAtSide_RArm_GestureYou"/>')
		
scene.removeScript('addcharacterdemo')
addcharacterdemo = AddCharacterDemo()
scene.addScript('addcharacterdemo', addcharacterdemo)