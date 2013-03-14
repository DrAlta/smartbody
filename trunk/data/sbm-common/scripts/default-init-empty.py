print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py  |"
print "|--------------------------------------------|"

def characterSetup(charName):
	sbChar = scene.getCharacter(charName)
	# steering
	steerManager = scene.getSteerManager()
	setupSteerAgent(charName, 'all')	
	steerManager.setEnable(True)
	
	# reaching
	reachSetup(charName,"KNN","")
	
	# jumping
	scene.run('BehaviorSetJumping.py')
	setupBehaviorSet()
	retargetBehaviorSet(charName, sbChar.getSkeleton().getName())
	
	# gesture
	scene.run('BehaviorSetGestures.py')
	setupBehaviorSet()
	retargetBehaviorSet(charName, sbChar.getSkeleton().getName())
	
	# punching
	scene.run('BehaviorSetPunching.py')
	setupBehaviorSet()
	retargetBehaviorSet(charName, sbChar.getSkeleton().getName())
	
	# kicking
	scene.run('BehaviorSetKicking.py')
	setupBehaviorSet()
	retargetBehaviorSet(charName, sbChar.getSkeleton().getName())



scene.run("default-viewer.py")

### Load data/sbm-common assets
scene.addAssetPath("seq", "sbm-common/scripts")
scene.addAssetPath("seq", "behaviorsets")

scene.run("init-common-assets.py")
scene.run("init-common-face.py")

scene.run("init-param-animation.py")

scene.run("init-steer-agents.py")
scene.run("init-example-reach.py")

scene.setBoolAttribute("internalAudio", True)

# start the simulation
sim.start()
scene.vhmsg("vrAllCall")
