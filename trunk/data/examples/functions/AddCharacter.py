import math

scene.run("init-common-assets.py")
scene.run("init-common-face.py")
scene.run("init-utah-face.py")
scene.run("init-param-animation.py")
scene.run("init-steer-agents.py")
scene.run("init-example-reach.py")

steerManager = scene.getSteerManager()
steerManager.setIntAttribute('gridDatabaseOptions.maxItemsPerGridCell', 14)

def addCharacter(charName, model, reach=False):
	'''Name of your character(string), Model name(string), Enable reaching
	   Available models: brad, utah, doctor, elder, billford(.dae) '''
	#assert(charName in scene.getCharacterNames()), 'already exists!'
	# Fix adding multiple
	# Create character
	char = scene.createCharacter(charName, model)
	# Set up skeleton and position for character
	if 'utah' in model:
		# Utah uses a different skeleton and has a different origin
		skeleton = scene.createSkeleton('test_utah.sk')
		char.setPosition(SrVec(0, 0, 0))
	elif 'billford' in model:
		skeleton = scene.createSkeleton('test_billford.sk')
		char.setPosition(SrVec(0, 102, 0))
	else:
		skeleton = scene.createSkeleton('common.sk')
		char.setPosition(SrVec(0, 102, 0))
	char.setSkeleton(skeleton)
	# Set up facing direction
	char.setHPR(SrVec(0, 0, 0))
	# Set up face definition
	char.setFaceDefinition(defaultFace)
	# Set up voice
	char.setVoice('remote')
	char.setVoiceCode('MicrosoftAnna')
	# Set up standard controllers
	char.createStandardControllers()
	# Assign mesh
	char.setStringAttribute('deformableMesh', model)
	# Set up steer agent
	setupSteerAgent(charName,'all')
	steerManager.setEnable(False)
	steerManager.setEnable(True)
	# Set up internal audio
	scene.setBoolAttribute("internalAudio", True)
	# Set up reach agent
	if reach:
		reachSetup(charName,"KNN","")
		
	# Start the simulation
	sim.start()
	if 'utah' in model:
		bml.execBML(charName, '<body posture="ChrUtah_Idle003"/>')
	if 'brad' in model:
		bml.execBML(charName, '<body posture="HandsAtSide_Motex"/>')
	if 'doctor' in model or 'elder' in model or 'billford' in model:
		bml.execBML(charName, '<body posture="LHandOnHip_Motex"/>')
	bml.execBML(charName, '<saccade mode="listen"/>')
	sim.resume()
	
def addMultipleCharacters(charName, model, amount, reach=False):
	''' Character name(string), Model name(string), Amount to spawn, Enable reaching '''
	index = 0; row = 0; column = 0; count = 0
	for i in range(amount):
		name = charName + str(index)
		'''
		for chrName in scene.getCharacterNames():
			if charName in chrName:
				print chrName
				count = count + 1
		name = charName + str(count)
		'''
		char = scene.createCharacter(name, model)
		posX = (-100 * (5/2)) + 100 * column
		posZ = ((-100 / math.sqrt(amount)) * (amount/2)) + 100 * row
		# Set up skeleton and position for character
		if 'utah' in model:
			# Utah uses a different skeleton and has a different origin
			skeleton = scene.createSkeleton('test_utah.sk')
			char.setPosition(SrVec(posX, 0, posZ))
		else:
			skeleton = scene.createSkeleton('common.sk')
			char.setPosition(SrVec(posX, 102, posZ))
		char.setSkeleton(skeleton)
		# Set up facing direction
		char.setHPR(SrVec(0, 0, 0))
		# Set up face definition
		char.setFaceDefinition(defaultFace)
		# Set up voice
		char.setVoice('remote')
		char.setVoiceCode('MicrosoftAnna')
		# Set up standard controllers
		char.createStandardControllers()
		# Assign mesh
		char.setStringAttribute('deformableMesh', model)
		index = index + 1
		column = column + 1
		if column >= 5:
			column = 0
			row = row + 1
	# Set up steering for each agent
	for name in scene.getCharacterNames():
		setupSteerAgent(name, 'all')
	steerManager.setEnable(False)
	steerManager.setEnable(True)
	# Set up reach if enabled
	if reach:
		for name in scene.getCharacterNames():
			reachSetup(name,'KNN','')
	# Start the simulation
	sim.start()
	for name in scene.getCharacterNames():
		if 'utah' in name:
			bml.execBML(name, '<body posture="ChrUtah_Idle003"/>')
		if 'brad' in name:
			bml.execBML(name, '<body posture="HandsAtSide_Motex"/>')
		if 'doctor' in name or 'elder' in name:
			bml.execBML(name, '<body posture="LHandOnHip_Motex"/>')
	bml.execBML(name, '<saccade mode="listen"/>')
	sim.resume()
	
def addMultipleCharacters2(charName, model, amount, reach=False):
	''' Character name(string), Model name(string), Amount to spawn, Enable reaching '''
	index = 0
	for i in range(amount):
		name = charName + str(index)
		char = scene.createCharacter(name, model)
		posX = (-100 * (amount/2)) + 100 * index
		# Set up skeleton and position for character
		if 'utah' in model:
			# Utah uses a different skeleton and has a different origin
			skeleton = scene.createSkeleton('test_utah.sk')
			char.setPosition(SrVec(posX, 0, 0))
		else:
			skeleton = scene.createSkeleton('common.sk')
			char.setPosition(SrVec(posX, 102, 0))
		char.setSkeleton(skeleton)
		# Set up facing direction
		char.setHPR(SrVec(0, 0, 0))
		# Set up face definition
		char.setFaceDefinition(defaultFace)
		# Set up voice
		char.setVoice('remote')
		char.setVoiceCode('MicrosoftAnna')
		# Set up standard controllers
		char.createStandardControllers()
		# Assign mesh
		char.setStringAttribute('deformableMesh', model)
		index = index + 1
		# Row ++ index reset
		
	numCharacters = scene.getNumCharacters()
	charNames = scene.getCharacterNames()
	for i in range(0, numCharacters):
		setupSteerAgent(charNames[i],'all')	
	steerManager.setEnable(False)
	steerManager.setEnable(True)

	if reach:
		names = scene.getCharacterNames()
		for n in range(0, len(names)):
			reachSetup(names[n],"KNN","")
			
	# Start the simulation
	sim.start()
	for name in scene.getCharacterNames():
		if 'utah' in name:
			bml.execBML(name, '<body posture="ChrUtah_Idle003"/>')
		if 'brad' in name:
			bml.execBML(name, '<body posture="HandsAtSide_Motex"/>')
		if 'doctor' in name or 'elder' in name or 'billford' in name:
			bml.execBML(name, '<body posture="LHandOnHip_Motex"/>')
		bml.execBML(name, '<saccade mode="listen"/>')

	sim.resume()
	
def addPawn(name, collisionShape, size=SrVec(10, 10, 10)):
	''' Name of pawn(string), Shape of pawn(string), Size of pawn(SrVec)
		Available shapes: sphere, box, capsule
		Default SrVec(10, 10, 10) '''
	pawn = scene.createPawn(name)
	pawn.setStringAttribute('collisionShape', collisionShape)
	pawn.getAttribute('collisionShapeScale').setValue(size)
	
def setPos(name, target):
	scene.getCharacter(name).setPosition(target)
	
def setPawnPos(name, target):
	scene.getPawn(name).setPosition(target)
	
def setFacing(name, amount):
	scene.getCharacter(name).setHPR(SrVec(amount, 0, 0))
	
def printVector(vec, text=''):
	print '%s x: %s y: %s z: %s' % (text, vec.getData(0), vec.getData(1), vec.getData(2))
