from random import choice
print "|--------------------------------------------|"
print "|        Starting Speech/Face Demo           |"
print "|--------------------------------------------|"

# Add asset paths
#scene.setMediaPath('')
scene.addAssetPath("script", '../../../../data/examples')
scene.addAssetPath("script", '../../../../data/examples/functions')
scene.addAssetPath('seq', '../../../../data/sbm-common/scripts')
scene.addAssetPath('seq', '../../../../data/sbm-test/scripts')
scene.addAssetPath('mesh', '../../../../data/mesh')
scene.addAssetPath('mesh', '../../../../data/retarget/mesh')
scene.addAssetPath('audio', '../../../../data/Resources/audio')

# Add Character script
scene.run('AddCharacter.py')

# Set scene parameters to fit new Brad and Rachel
scene.setScale(1.0)
scene.setBoolAttribute("internalAudio", True)
scene.run("default-viewer.py")
camera = getCamera()
camera.setEye(0.03, 1.59, 1.42)
camera.setCenter(0.11, 0.88, -0.43)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

addNewBrad('ChrBrad', .35, 0)
addNewRachel('ChrRachel', -.35, 0)
scene.getCharacter('ChrBrad').setHPR(SrVec(-17, 0, 0))
scene.getCharacter('ChrRachel').setHPR(SrVec(17, 0, 0))

scene.getCharacter('ChrBrad').setVoiceCode('MicrosoftAnna')

scene.command('char ChrBrad viewer deformableGPU')
scene.command('char ChrRachel viewer deformableGPU')

# Add Speech script 
scene.run('Speech.py')

# Add Facial Movements script
scene.run('FacialMovements.py')

# Add Gestures script
scene.run('Gestures.py')
initGestureMap()

# Add Saccade script
scene.run('Saccade.py')
playSaccade('ChrBrad', 'talk')
playSaccade('ChrRachel', 'talk')

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
			playRandomGesture()
			speakRandomSentence()
			
# Run the update script
scene.removeScript('speechdemo')
speechdemo = SpeechDemo()
scene.addScript('speechdemo', speechdemo)
	
# Sentences
bradSentences = []
bradSentences.append('My name is Brad')
bradSentences.append('Hello Rachel, how are you today?')
bradSentences.append('What is your favorite color?')
bradSentences.append('Stay a while, and listen')
bradSentences.append('But our princess is in another castle')
bradLastSentence = ''

rachelSentences = []
rachelSentences.append('My name is Rachel')
rachelSentences.append('Hello Brad, how are you?')
rachelSentences.append('Have you heard the news?')
rachelSentences.append('What are some of your hobbies?')
rachelSentences.append('Its dangerous to go alone, take this')
rachelLastSentence = ''

# Current turn
currentTurn = 'ChrBrad'

def speakRandomSentence():
	global bradLastSentence, rachelLastSentence, currentTurn
	# If Brad's turn
	if currentTurn == 'ChrBrad':
		randomSentence = choice(bradSentences)
		for i in range(5):
			if randomSentence == bradLastSentence:
				randomSentence = choice(sentenceList)
			else:
				break
		lastSentence = randomSentence
		speak(currentTurn, randomSentence)
		currentTurn = 'ChrRachel'
	# If Rachel's turn
	elif currentTurn == 'ChrRachel':
		randomSentence = choice(rachelSentences)
		for i in range(5):
			if randomSentence == rachelLastSentence:
				randomSentence = choice(sentenceList)
			else:
				break
		lastSentence = randomSentence
		speak(currentTurn, randomSentence)
		currentTurn = 'ChrBrad'

# List of gesture types
gestureList = ['YOU', 'ME', 'LEFT', 'RIGHT', 'NEGATION', 'CONTRAST', 
			   'ASSUMPTION', 'RHETORICAL', 'INCLUSIVITY', 'QUESTION', 
			   'OBLIGATION', 'GREETING', 'CONTEMPLATE']
def playRandomGesture():
	# Randomly choose and play a gesture
	randomGesture = choice(gestureList)
	playGesture(currentTurn, '', '', randomGesture)