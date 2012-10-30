import random

print "|--------------------------------------------|"
print "|       Starting Facial Movement Demo        |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath("script","../../../../data/examples")
scene.addAssetPath("script","../../../../data/examples/functions")
scene.addAssetPath('seq', '../../../../data/sbm-common/scripts')
scene.addAssetPath('seq', '../../../../data/sbm-test/scripts')
scene.addAssetPath('mesh', '../../../../data/mesh')
scene.addAssetPath('mesh', '../../../../data/retarget/mesh')
scene.addAssetPath('audio', '../../../../data/Resources/audio')

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(4.55, 166.36, 49.51)
camera.setCenter(4.55, 166.36, 3.78)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(0, 0, 0))

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Add Facial Movements script
scene.run('FacialMovements.py')

# Add Saccade script
scene.run('Saccade.py')
playSaccade('utah', 'talk')

# Update to repeat reaches
last = 0
canTime = True
delay = 2
class FacialMovementDemo(SBScript):
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
			randomizeFace()
		
AUDict = {'1_left':0, '1_right':0, '2_left':0, '2_right':0, '4_left':0, '4_right':0, 
		  '5':0, '6':0 , '7':0, '9':0, '10':0, '12':0, '15':0, '20':0,
		  '23':0, '25':0, '26':0, '27':0, '38':0, '39':0, '45_left':0, '45_right':0}
randomFactor = 0.5
multiplier = 1

def randomizeFace():
	global multiplier
	for key in AUDict:
		if key == '20' or key == '26' or key == '27' or key == '45_left' or key == '45_right':
			multiplier = 0.25
		else:
			multiplier = 1
		rand = random.uniform(0, randomFactor * multiplier)
		AUDict[key] = rand
		bml.execBML('*', '<face amount="' + str(AUDict[key]) + '" au="' + str(key) + '" side="BOTH" type="facs"/>')
		
def printDict():
	for key,value in AUDict.items():
		print 'Key: %s Value: %s' % (key, value)
		
# Run the update script
scene.removeScript('facialmovementdemo')
facialmovementdemo = FacialMovementDemo()
scene.addScript('facialmovementdemo', facialmovementdemo)