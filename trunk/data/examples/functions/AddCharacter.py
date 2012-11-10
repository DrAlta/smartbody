import math

scene.run("init-common-assets.py")
scene.run("init-common-face.py")
scene.run("init-utah-face.py")
scene.run("init-param-animation.py")
scene.run("init-steer-agents.py")
scene.run("init-example-reach.py")

scene.addAssetPath('motion', '../../../../data/ChrBrad')
scene.addAssetPath('motion', '../../../../data/ChrRachel')
scene.run('init-diphoneDefault.py')

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
	
def addMultipleCharacters(charName, model, amount, reach=False, offsetX=0, offsetZ=0):
	''' Character name(string), Model name(string), Amount to spawn, Enable reaching, Offset X, Offset Z '''
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
			char.setPosition(SrVec(posX + offsetX, 0, posZ + offsetZ))
		else:
			skeleton = scene.createSkeleton('common.sk')
			char.setPosition(SrVec(posX + offsetX, 102, posZ + offsetZ))
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
	
def addNewBrad(charName, posX=0, posZ=0):
	scene.loadAssetsFromPath('../../../../data/ChrBrad')
	scene.run('zebra2-map.py')
	zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
	# Brad skeleton
	bradSkeleton = scene.getSkeleton('ChrBrad.sk')
	zebra2Map.applySkeleton(bradSkeleton)
	zebra2Map.applyMotionRecurse('../../../../data/ChrBrad')
	# Brad's face definition
	bradFace = scene.createFaceDefinition(charName)
	bradFace.setFaceNeutral('ChrBrad@face_neutral')
	
	bradFace.setAU(1,  "left",  "ChrBrad@001_inner_brow_raiser_lf")
	bradFace.setAU(1,  "right", "ChrBrad@001_inner_brow_raiser_rt")
	bradFace.setAU(2,  "left",  "ChrBrad@002_outer_brow_raiser_lf")
	bradFace.setAU(2,  "right", "ChrBrad@002_outer_brow_raiser_rt")
	bradFace.setAU(4,  "left",  "ChrBrad@004_brow_lowerer_lf")
	bradFace.setAU(4,  "right", "ChrBrad@004_brow_lowerer_rt")
	bradFace.setAU(5,  "both",  "ChrBrad@005_upper_lid_raiser")
	bradFace.setAU(6,  "both",  "ChrBrad@006_cheek_raiser")
	bradFace.setAU(7,  "both",  "ChrBrad@007_lid_tightener")
	bradFace.setAU(10, "both",  "ChrBrad@010_upper_lip_raiser")
	bradFace.setAU(12, "left",  "ChrBrad@012_lip_corner_puller_lf")
	bradFace.setAU(12, "right", "ChrBrad@012_lip_corner_puller_rt")
	bradFace.setAU(25, "both",  "ChrBrad@025_lips_part")
	bradFace.setAU(26, "both",  "ChrBrad@026_jaw_drop")
	bradFace.setAU(45, "left",  "ChrBrad@045_blink_lf")
	bradFace.setAU(45, "right", "ChrBrad@045_blink_rt")

	bradFace.setViseme("open",    "ChrBrad@open")
	bradFace.setViseme("W",       "ChrBrad@W")
	bradFace.setViseme("ShCh",    "ChrBrad@ShCh")
	bradFace.setViseme("PBM",     "ChrBrad@PBM")
	bradFace.setViseme("FV",      "ChrBrad@FV")
	bradFace.setViseme("wide",    "ChrBrad@wide")
	bradFace.setViseme("tBack",   "ChrBrad@tBack")
	bradFace.setViseme("tRoof",   "ChrBrad@tRoof")
	bradFace.setViseme("tTeeth",  "ChrBrad@tTeeth")

	brad = scene.createCharacter(charName, '')
	bradSkeleton = scene.createSkeleton('ChrBrad.sk')
	brad.setSkeleton(bradSkeleton)
	brad.setFaceDefinition(bradFace)
	bradPos = SrVec(posX, 0, posZ)
	brad.setPosition(bradPos)
	#bradHPR = SrVec(-17, 0, 0)
	#brad.setHPR(bradHPR)
	brad.createStandardControllers()
	# Deformable mesh
	brad.setDoubleAttribute('deformableMeshScale', .01)
	brad.setStringAttribute('deformableMesh', 'ChrBrad')
	# Lip syncing diphone setup
	brad.setStringAttribute('diphoneSetName', 'default')
	brad.setBoolAttribute('useDiphone', True)
	brad.setVoice('remote')
	brad.setVoiceCode('Festival_voice_kal_diphone')
	# Gesture map setup
	#brad.setStringAttribute('gestureMap', charName)
	#brad.setBoolAttribute('bmlRequest.autoGestureTransition', True)
	# start the simulation
	sim.start()

	bml.execBML(charName, '<body posture="ChrBrad@Idle01"/>')
	bml.execBML(charName, '<saccade mode="listen"/>')

	sim.resume()

def addNewRachel(charName, posX=0, posZ=0):
	scene.loadAssetsFromPath('../../../../data/ChrRachel')
	scene.run('zebra2-map.py')
	zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
	# Rachel skeleton
	rachelSkeleton = scene.getSkeleton('ChrRachel.sk')
	zebra2Map.applySkeleton(rachelSkeleton)
	zebra2Map.applyMotionRecurse('../../../../data/ChrRachel')
	# Rachel's face definition
	rachelFace = scene.createFaceDefinition(charName)
	rachelFace.setFaceNeutral('ChrRachel@face_neutral')
	
	rachelFace.setAU(1,  "left",  "ChrRachel@001_inner_brow_raiser_lf")
	rachelFace.setAU(1,  "right", "ChrRachel@001_inner_brow_raiser_rt")
	rachelFace.setAU(2,  "left",  "ChrRachel@002_outer_brow_raiser_lf")
	rachelFace.setAU(2,  "right", "ChrRachel@002_outer_brow_raiser_rt")
	rachelFace.setAU(4,  "left",  "ChrRachel@004_brow_lowerer_lf")
	rachelFace.setAU(4,  "right", "ChrRachel@004_brow_lowerer_rt")
	rachelFace.setAU(5,  "both",  "ChrRachel@005_upper_lid_raiser")
	rachelFace.setAU(6,  "both",  "ChrRachel@006_cheek_raiser")
	rachelFace.setAU(7,  "both",  "ChrRachel@007_lid_tightener")
	rachelFace.setAU(10, "both",  "ChrRachel@010_upper_lip_raiser")
	rachelFace.setAU(12, "left",  "ChrRachel@012_lip_corner_puller_lf")
	rachelFace.setAU(12, "right", "ChrRachel@012_lip_corner_puller_rt")
	rachelFace.setAU(25, "both",  "ChrRachel@025_lips_part")
	rachelFace.setAU(26, "both",  "ChrRachel@026_jaw_drop")
	rachelFace.setAU(45, "left",  "ChrRachel@045_blink_lf")
	rachelFace.setAU(45, "right", "ChrRachel@045_blink_rt")

	rachelFace.setViseme("open",    "ChrRachel@open")
	rachelFace.setViseme("W",       "ChrRachel@W")
	rachelFace.setViseme("ShCh",    "ChrRachel@ShCh")
	rachelFace.setViseme("PBM",     "ChrRachel@PBM")
	rachelFace.setViseme("FV",      "ChrRachel@FV")
	rachelFace.setViseme("wide",    "ChrRachel@wide")
	rachelFace.setViseme("tBack",   "ChrRachel@tBack")
	rachelFace.setViseme("tRoof",   "ChrRachel@tRoof")
	rachelFace.setViseme("tTeeth",  "ChrRachel@tTeeth")
	
	rachel = scene.createCharacter(charName, '')
	rachelSkeleton = scene.createSkeleton('ChrRachel.sk')
	rachel.setSkeleton(rachelSkeleton)
	rachel.setFaceDefinition(rachelFace)
	rachelPos = SrVec(posX, 0, posZ)
	rachel.setPosition(rachelPos)
	rachelHPR = SrVec(0, 0, 0)
	rachel.setHPR(rachelHPR)
	rachel.createStandardControllers()
	# Deformable mesh
	rachel.setDoubleAttribute('deformableMeshScale', .01)
	rachel.setStringAttribute('deformableMesh', 'ChrRachel')
	# Lip syncing diphone setup
	rachel.setStringAttribute('diphoneSetName', 'default')
	rachel.setBoolAttribute('useDiphone', True)
	rachel.setVoice('remote')
	rachel.setVoiceCode('MicrosoftAnna')
	# Gesture map setup
	#rachel.setStringAttribute('gestureMap', charName)
	#rachel.setBoolAttribute('bmlRequest.autoGestureTransition', True)
	# Start the simulation
	sim.start()
	bml.execBML(charName, '<body posture="ChrRachel_ChrBrad@Idle01"/>')
	bml.execBML(charName, '<saccade mode="listen"/>')
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
