print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py  |"
print "|--------------------------------------------|"

def characterSetup(charName):
	# steering
	steerManager = scene.getSteerManager()
	setupSteerAgent(charName, 'all')	
	steerManager.setEnable(True)
	
	# reaching
	reachSetup(charName,"KNN","")



scene.run("default-viewer.py")

### Load data/sbm-common assets
scene.addAssetPath("seq", "sbm-common/scripts")

scene.run("init-common-assets.py")
scene.run("init-common-face.py")

scene.run("init-param-animation.py")

scene.run("init-steer-agents.py")
scene.run("init-example-reach.py")

scene.setBoolAttribute("internalAudio", True)

# start the simulation
sim.start()
scene.vhmsg("vrAllCall")
