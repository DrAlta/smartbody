print "Init Steer Agent"

steerManager = scene.getSteerManager()

steerManager.setSteerUnit("centimeter")
numCharacters = scene.getNumCharacters()
charNames = scene.getCharacterNames()
for i in range(0, numCharacters):
	steerAgent = steerManager.createSteerAgent(charNames[i])
	sbCharacter = scene.getCharacter(charNames[i])
	sbCharacter.setSteerAgent(steerAgent)

steerManager.start()
