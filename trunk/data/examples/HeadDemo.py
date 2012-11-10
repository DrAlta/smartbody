import random
print "|--------------------------------------------|"
print "|            Starting Head Demo              |"
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

# Add Character script
scene.run('AddCharacter.py')

# Set scene parameters to fit new Brad
scene.setScale(1.0)
scene.run("default-viewer.py")
camera = getCamera()
camera.setEye(0.0013, 1.66, 0.3756)
camera.setCenter(0.1031, 1.58, -0.0099)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

addNewBrad('ChrBrad')
scene.getCharacter('ChrBrad').setHPR(SrVec(-17, 0, 0))

scene.command("char ChrBrad viewer deformableGPU")

last = 0
canTime = True
delay = 5
class HeadDemo(SBScript):
	def update(self, time):
		global last, canTime
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		# When time's up do action
		if canTime:
			last = time
			canTime = False
			randomHead()

headList = ['NOD', 'SHAKE', 'TOSS', 'WIGGLE', 'WAGGLE']
def randomHead():
	headChoice = random.choice(headList)
	print headChoice
	repeats = random.uniform(0.1, 2)
	velocity = random.uniform(0.1, 1)
	amount = random.uniform(0.1, 1)
	bml.execBML('*', '<head type="' + headChoice + '" amount="' + str(amount) + '" repeats="' + str(repeats) + '" velocity="' + str(velocity) + '"/>')
			
# Run the update script
scene.removeScript('headdemo')
headdemo = HeadDemo()
scene.addScript('headdemo', headdemo)