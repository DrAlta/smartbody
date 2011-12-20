print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py  |"
print "|--------------------------------------------|"
scene.command("vhmsglog on")
scene.run("default-viewer.py")

### Load data/sbm-common assets
### Assumes current directory is: core/smartbody/sbm/bin
scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.addAssetPath("seq", "../../../../data/sbm-test/scripts")
scene.addAssetPath("mesh", "../../../../data/mesh")

scene.run("init-common-assets.py")
#0	seq init-general-parameters
scene.run("init-common-face.py")
scene.command("panim enable")

numCharacters = 50
for i in range(0, numCharacters):

	brad = scene.createCharacter("brad" + str(i), "")
	bradSkeleton = scene.createSkeleton("common.sk")
	brad.setSkeleton(bradSkeleton)
	brad.setFaceDefinition(defaultFace)
	row = i % 10
	col = int(i / 10)
	bradPos = SrVec(-135 + 100 * row, 102, 100 * col)
	
	brad.setPosition(bradPos)
	bradHPR = SrVec(-17, 0, 0)
	brad.setHPR(bradHPR)
	brad.setVoice("remote")
	brad.setVoiceCode("star")
	brad.createStandardControllers()
	#utah.setStringAttribute("mesh", "utah")

scene.setDefaultCharacter("brad0")
scene.setDefaultRecipient("brad1")

scene.run("init-param-animation.py")

#scene.run("init-steer-agents.py")

#scene.run("init-example-reach.py")
#names = scene.getCharacterNames()
#for n in range(0, len(names)):
#	reachSetup(names[n])

# start the simulation
sim.start()

for i in range(0, numCharacters):
	bml.execBML('brad' + str(i), '<body posture="ChrUtah_Idle003"/>')
	
	
for i in range(0, numCharacters):
	scene.command("panim schedule char brad" + str(i) + " state allLocomotion loop true playnow false")
	#scene.commandAt(3, "panim updatestate char brad" + str(i) + " 0 100 0")
sim.resume()



