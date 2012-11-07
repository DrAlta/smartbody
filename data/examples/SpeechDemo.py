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
camera.setEye(1, 144.70, 147.41)
camera.setCenter(1, 137.54, 102.19)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(0, 0, 0))

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Add Speech script 
scene.run('Speech.py')

# Add Facial Movements script
scene.run('FacialMovements.py')

# Add Gestures script
scene.run('Gestures.py')
initGestureMap()

# Add Saccade script
scene.run('Saccade.py')
playSaccade('utah', 'talk')

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
	
# Keywords may overlap
sentenceList = []
sentenceList.append('You have made me very angry')
sentenceList.append('I am so happy that I am not a real boy')
sentenceList.append('Why must you hurt my feelings when I am sad')
sentenceList.append('How can you say that about my weight?')
sentenceList.append('Free kittens are on the left side of the road')
sentenceList.append('Sweet death is on the right side of the road')
lastSentence = ''

def speakRandomSentence():
	global lastSentence
	randomSentence = choice(sentenceList)
	for i in range(5):
		if randomSentence == lastSentence:
			randomSentence = choice(sentenceList)
		else:
			break
	if 'angry' in randomSentence: 
		angry('utah')
		playGesture('utah', 'SWEEPRIGHT', 'RIGHT_HAND')
	if 'happy' in randomSentence: 
		happy('utah')
		playGesture('utah', 'OFFER', 'RIGHT_HAND')
	if 'sad' in randomSentence:	
		sad('utah')
		playGesture('utah', 'YOU', 'RIGHT_HAND')
	if 'weight' in randomSentence: 
		shock('utah')
		playGesture('utah', 'WHY', 'BOTH_HANDS')
	if 'left' in randomSentence:
		happy('utah')
		playGesture('utah', 'LEFT', 'LEFT_HAND')
	if 'right' in randomSentence:
		sad('utah')
		playGesture('utah', 'RIGHT', 'RIGHT_HAND')
	
	lastSentence = randomSentence
	speak('utah', randomSentence)
	
	