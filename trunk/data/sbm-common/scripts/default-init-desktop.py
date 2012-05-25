print "|---------------------------------------------------|"
print "|  data/sbm-common/scripts/default-init-desktop.py  |"
print "|---------------------------------------------------|"


#scene.run("default-viewer.py")
camera = getCamera()
camera.setEye(0, 166, 185)
camera.setCenter(0, 92, 0)

cameraPawn = scene.createPawn("camera")
cameraPos = SrVec(0, 166, 185)
cameraPawn.setPosition(cameraPos)


### Load data/sbm-common assets
scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.addAssetPath("seq", "../../../../data/sbm-test/scripts")
scene.addAssetPath("mesh", "../../../../data/mesh")

scene.run("init-common-assets.py")
scene.run("init-common-face.py")


'''
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
'''

utah = scene.createCharacter("utah", "")
utahSkeleton = scene.createSkeleton("test_utah.sk")
utah.setSkeleton(utahSkeleton)
utah.setFaceDefinition(defaultFace)
utahPos = SrVec(0, 0, 0)
utah.setPosition(utahPos)
utahHPR = SrVec(-17, 0, 0)
utah.setHPR(utahHPR)
utah.setVoice("remote")
utah.setVoiceCode("Festival_voice_rab_diphone")
utah.createStandardControllers()
utah.setStringAttribute("deformableMesh", "utah")

#scene.setDefaultCharacter("doctor")
#scene.setDefaultRecipient("elder")

scene.run("init-param-animation.py")
scene.run("init-steer-agents.py")

scene.setBoolAttribute("internalAudio", True)

# add the reaching database
# scene.run("init-example-reach.py")
# names = scene.getCharacterNames()
# for n in range(0, len(names)):
	# reachSetup(names[n])

# start the simulation
sim.start()

#bml.execBML('doctor', '<body posture="LHandOnHip_Motex"/>')
#bml.execBML('elder', '<body posture="LHandOnHip_Motex"/>')
#bml.execBML('brad', '<body posture="HandsAtSide_Motex"/>')
bml.execBML('utah', '<body posture="ChrUtah_Idle003"/>')

#bml.execBML('doctor', '<saccade mode="listen"/>')
#bml.execBML('elder', '<saccade mode="listen"/>')
#bml.execBML('brad', '<saccade mode="listen"/>')
bml.execBML('utah', '<saccade mode="listen"/>')

sim.resume()


'''
class CollisionHandler(EventHandler):
	def executeAction(self, ev):
		params = ev.getParameters()
		# uncomment the following line when debugging
		#print params
		tok = params.split('/')
		#numTokens = len(tok)
		#print tok[0] +' and '+ tok[1] + ' has collision';			
		#bml.execBML(tok[0],'<gaze target="'+tok[1]+'"/>');
		#bml.execBML(tok[1],'<gaze target="'+tok[0]+'"/>');
				
		

f = CollisionHandler()
em = scene.getEventManager()
em.addEventHandler("collision", f)
'''






