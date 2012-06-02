print "|-------------------------------------------------|"
print "|  data/sbm-common/scripts/default-init-empty.py  |"
print "|-------------------------------------------------|"

def createStandardCharacter(charName, skelName, meshName, position):
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
	retargetCharacter(charName,skelName)

scene.run("default-viewer.py")
scene.setIntAttribute("colladaTrimFrames", 2)
### Load data/sbm-common assets
scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.addAssetPath("seq", "../../../../data/sbm-test/scripts")
scene.addAssetPath("mesh", "../../../../data/mesh")

scene.run("init-common-assets.py")
scene.run("init-common-face.py")
scene.run("mixamo-map.py")

jointMapManager = scene.getJointMapManager()
jointMap = jointMapManager.getJointMap('mixamo')	

playerSkelName = 'player.dae'
playerSkel = scene.getSkeleton(playerSkelName)
jointMap.applySkeleton(playerSkel)
	
createStandardCharacter('player', playerSkelName, 'player', SrVec(0, 0, 100))

steerManager = scene.getSteerManager()
steerManager.setBoolAttribute("useEnvironmentCollisions",False)
steerManager.setEnable(False)
steerManager.setEnable(True)
scene.setBoolAttribute("internalAudio", True)

# start the simulation
sim.start()
bml.execBML('player', '<body posture="'+ playerSkelName +'HandsAtSide_Motex"/>')
sim.resume()


	

	





