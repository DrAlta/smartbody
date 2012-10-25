# Initialize and run NVBG
print "|--------------------------------------------|"
print "|         Starting NVBG Speech               |"
print "|   HAVE YOU ENABLED WINDOW > SPEECH RELAY?' |"
print "|--------------------------------------------|"

scene.run('initSmartBodyNVBG.py')
scene.run('runNVBG.py')

# Dictionary to store each character's NVBG
chrNVBG = {}
for name in scene.getCharacterNames():
	chrNVBG[name] = scene.getCharacter(name).getNvbg().nvbg
	
def speak(chrName, text='give me something to say'):
	''' Character name(string), Text to say(string)'''
	chrNVBG[chrName].speak(text);