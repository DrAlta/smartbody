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

scene.run('bradrachel.py')
camera.setEye(0.33, 1.64, 0.39)
camera.setCenter(0.44, 1.57, -0.02)

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