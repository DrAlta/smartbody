print "|-------------------------------------------------|"
print "|  data/sbm-common/scripts/drag-and-drop.py     |"
print "|-------------------------------------------------|"


def createStandardCharacter(charName, skelName, meshName, position):
	print 'create character : ' + charName + '  , skelName : ' + skelName;
	sbChar = scene.createCharacter(charName, "")
	sbSkeleton = scene.createSkeleton(skelName)		
	sbChar.setSkeleton(sbSkeleton)
	sbChar.setFaceDefinition(defaultFace)
	sbPos = position
	sbChar.setPosition(sbPos)	
	sbChar.setVoice("remote")
	sbChar.setVoiceCode("Festival_voice_rab_diphone")
	sbChar.createStandardControllers()
	sbChar.setStringAttribute("deformableMesh", meshName)	
	scene.run("motion-retarget.py")
	print 'retarget character : ' + charName + '  , skelName : ' + skelName;
	retargetCharacter(charName,skelName)

def createDragAndDropCharacter(charName, skelName, meshName, position):
	dndSkel = scene.getSkeleton(skelName)
	jointMapManager = scene.getJointMapManager()
	jointMap = jointMapManager.getJointMap(skelName)
	if (jointMap == None):
		jointMap = jointMapManager.createJointMap(skelName)
		jointMap.guessMapping(dndSkel, False)
	
	dndSkel = scene.getSkeleton(skelName)
	jointMap.applySkeleton(dndSkel)

	createStandardCharacter(charName, skelName, meshName, position)
	
	print 'drag and drop position =' + str(position.getData(0)) + ' ,' + str(position.getData(1)) + ', ' + str(position.getData(2))

	steerManager = scene.getSteerManager()
	steerManager.setBoolAttribute("useEnvironmentCollisions",False)
	steerManager.setEnable(False)
	steerManager.setEnable(True)
	scene.setBoolAttribute("internalAudio", True)

	# start the simulation
	sim.start()
	bml.execBML(charName, '<body posture="'+ skelName +'HandsAtSide_Motex"/>')
	scene.command('char ' +charName + ' viewer deformable');
	scene.run("characterTest.py")
	testRetargetCharacter(charName, skelName)


	

	





