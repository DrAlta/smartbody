print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py  |"
print "|--------------------------------------------|"

scene.run("default-viewer.py")

### Load data/sbm-common assets
scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.addAssetPath("seq", "../../../../data/sbm-test/scripts")
scene.addAssetPath("mesh", "../../../../data/mesh")
scene.addAssetPath("mesh", "../../../../data/retarget/mesh")

scene.run("init-common-assets.py")
scene.run("init-common-face.py")
scene.run("init-utah-face.py")

doctor = scene.createCharacter("doctor", "SasoBase.SasoDoctorPerez")
doctorSkeleton = scene.createSkeleton("common.sk")
doctor.setSkeleton(doctorSkeleton)
doctor.setFaceDefinition(defaultFace)
doctorPos = SrVec(35, 102, 0)
doctor.setPosition(doctorPos)
doctorHPR = SrVec(-17, 0, 0)
doctor.setHPR(doctorHPR)
doctor.setVoice("remote")
doctor.setVoiceCode("Festival_voice_rab_diphone")
doctor.createStandardControllers()
doctor.setStringAttribute("deformableMesh", "doctor")

elder = scene.createCharacter("elder", "SasoBase.Mayor")
elderSkeleton = scene.createSkeleton("common.sk")
elder.setSkeleton(elderSkeleton)
elder.setFaceDefinition(defaultFace)
elderPos = SrVec(-35, 102, 0)
elder.setPosition(elderPos)
elderHPR = SrVec(17, 0, 0)
elder.setHPR(elderHPR)
elder.setVoice("remote")
elder.setVoiceCode("Festival_voice_rab_diphone")
elder.createStandardControllers()
elder.setStringAttribute("deformableMesh", "elder")

brad = scene.createCharacter("brad", "")
bradSkeleton = scene.createSkeleton("common.sk")
brad.setSkeleton(bradSkeleton)
brad.setFaceDefinition(defaultFace)
bradPos = SrVec(135, 102, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(-17, 0, 0)
brad.setHPR(bradHPR)
brad.setVoice("remote")
brad.setVoiceCode("Festival_voice_rab_diphone")
brad.createStandardControllers()
brad.setStringAttribute("deformableMesh", "brad")

utah = scene.createCharacter("utah", "")
utahSkeleton = scene.createSkeleton("test_utah.sk")
utah.setSkeleton(utahSkeleton)
utah.setFaceDefinition(utahFace)
utahPos = SrVec(-135, 0, 0)
utah.setPosition(utahPos)
utahHPR = SrVec(-17, 0, 0)
utah.setHPR(utahHPR)
utah.setVoice("remote")
utah.setVoiceCode("Festival_voice_rab_diphone")
utah.createStandardControllers()
utah.setStringAttribute("deformableMesh", "utah")

scene.setDefaultCharacter("doctor")
scene.setDefaultRecipient("elder")

scene.run("init-param-animation.py")

scene.run("init-steer-agents.py")
steerManager = scene.getSteerManager()

numCharacters = scene.getNumCharacters()
charNames = scene.getCharacterNames()
for i in range(0, numCharacters):
	setupSteerAgent(charNames[i],'all')	
steerManager.setEnable(True)

scene.setBoolAttribute("internalAudio", True)

# add the reaching database
scene.run("init-example-reach.py")
names = scene.getCharacterNames()
for n in range(0, len(names)):
	reachSetup(names[n],"KNN","")

# start the simulation
sim.start()

bml.execBML('doctor', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('elder', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('brad', '<body posture="HandsAtSide_Motex"/>')
bml.execBML('utah', '<body posture="ChrUtah_Idle003"/>')

bml.execBML('doctor', '<saccade mode="listen"/>')
bml.execBML('elder', '<saccade mode="listen"/>')
bml.execBML('brad', '<saccade mode="listen"/>')
bml.execBML('utah', '<saccade mode="listen"/>')

sim.resume()





