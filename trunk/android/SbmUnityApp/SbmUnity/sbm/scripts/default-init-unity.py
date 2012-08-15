print "|--------------------------------------------|"
print "|  default-init-unity.py                     |"
print "|--------------------------------------------|"

scene.command('time simfps 0')
scene.run("init-common-face.py")

# Init billford character
billford = scene.createCharacter("billford", "art/BillFord/ChrBillFordPrefab")
billfordSkeleton = scene.createSkeleton("test_billford_unity.sk")
billford.setSkeleton(billfordSkeleton)
billford.setFaceDefinition(defaultFace)
billfordPos = SrVec(0, 0, 0)
billford.setPosition(billfordPos)
billfordHPR = SrVec(-17, 0, 0)
billford.setHPR(billfordHPR)
#billford.setVoice("remote")
#billford.setVoiceCode("Festival_voice_rab_diphone")
billford.createStandardControllers()

# Setup parameterize animation and steering

scene.run("init-param-animation.py")
scene.run("init-steer-agents.py")
steerManager = scene.getSteerManager()
numCharacters = scene.getNumCharacters()
charNames = scene.getCharacterNames()
for i in range(0, numCharacters):
	setupSteerAgent(charNames[i],'all')	
steerManager.setEnable(True)

scene.run("init-example-reach.py")
names = scene.getCharacterNames()
for n in range(0, len(names)):
	reachSetup(names[n],"RBF","")

sim.start()
bml.execBML('billford', '<body posture="ChrUtah_IdleHandsAtSide001"/>')