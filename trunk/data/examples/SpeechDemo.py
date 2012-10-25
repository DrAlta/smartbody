from random import choice

print "|--------------------------------------------|"
print "|        Starting Speech/Face Demo           |"
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
camera.setEye(4, 155.79, 86.42)
camera.setCenter(4, 155.79, 40.63)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(10000)
camera.setNearPlane(1)
camera.setAspectRatio(1.02632)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(0, 0, 0))

# Add Speech script 
scene.run('Speech.py')

# Add Facial Movements script
scene.run('FacialMovements.py')

scene.run('Gestures.py')
initGestureMap()

# Update to repeat reaches
last = 0
canTime = True
delay = 5
class SpeechDemo(SBScript):
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
			speakRandomSentence()
			
# Run the update script
scene.removeScript('speechdemo')
speechdemo = SpeechDemo()
scene.addScript('speechdemo', speechdemo)
	
sentenceList = []
sentenceList.append('I am very very angry at you right now')
sentenceList.append('I am so happy that I am not a real boy')
sentenceList.append('I am so sad and you should feel bad')
sentenceList.append('How can you say that about my weight?')
def speakRandomSentence():
	randomSentence = choice(sentenceList)
	if 'angry' in randomSentence: angry('utah', 5)
	if 'happy' in randomSentence: happy('utah', 5)
	if 'sad' in randomSentence:	sad('utah', 5)
	if 'weight' in randomSentence: shock('utah', 5)
	speak('utah', randomSentence)
	
	