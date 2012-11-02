from random import choice

print "|--------------------------------------------|"
print "|        Starting Speech/Face Demo           |"
print "|--------------------------------------------|"

# Add asset paths
scene.setMediaPath('../../../../data/')
scene.addAssetPath("script", 'examples')
scene.addAssetPath("script", 'examples/functions')
scene.addAssetPath('seq', 'sbm-common/scripts')
scene.addAssetPath('seq', 'sbm-test/scripts')
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('mesh', 'retarget/mesh')
scene.addAssetPath('audio', 'Resources/audio')

scene.run('bradrachel.py')
brad.setVoiceCode('MicrosoftAnna')
camera.setEye(-0.03, 2.54, 1.41)
camera.setCenter(0.1, 1.92, -0.47)

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

rachelSentences = []
rachelSentences.append('My name is Rachel')
rachelSentences.append('Hello Brad, how are you?')
rachelSentences.append('Have you heard the news?')
rachelSentences.append('What are some of your hobbies?')
rachelSentences.append('Its dangerous to go alone, take this')

bradLastSentence = ''
rachelLastSentence = ''

currentTurn = 'ChrBrad'

def speakRandomSentence():
	global bradLastSentence, rachelLastSentence, currentTurn
	
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