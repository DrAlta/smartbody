print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/doctor.py  |"
print "|--------------------------------------------|"


### Load data/sbm-common assets
scene.addAssetPath("script", "sbm-common/scripts")
scene.addAssetPath("mesh", "mesh")
scene.addAssetPath("audio", ".")
scene.addAssetPath("motion", "sbm-common/common-sk")
scene.loadAssets()

scene.setBoolAttribute("internalAudio", True)

camera = scene.getActiveCamera()
camera.setEye(0, 20, 200)
camera.setCenter(0, 0, 0)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1)
camera.setFarPlane(1000)
camera.setNearPlane(0.1)
camera.setAspectRatio(1)


scene.run("init-common-face.py")

doctor = scene.createCharacter("doctor", "")
doctorSkeleton = scene.createSkeleton("common.sk")
doctor.setSkeleton(doctorSkeleton)
doctor.setFaceDefinition(defaultFace)
doctorPos = SrVec(0, 0, 0)
doctor.setPosition(doctorPos)
doctorHPR = SrVec(0, 0, 0)
doctor.setHPR(doctorHPR)
doctor.setVoice("audiofile")
doctor.setVoiceCode("Sounds")
doctor.createStandardControllers()
doctor.setStringAttribute("deformableMesh", "doctor")


# start the simulation
sim.start()
bml.execBML('doctor', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('doctor', '<saccade mode="listen"/>')
sim.resume()

# start the flash gaze controller
bml.execBML('doctor', '<gaze sbm:handle="flash" sbm:target-pos="0 0 0"/>')
bml.execBML('doctor', '<gaze sbm:handle="flash" sbm:fade-out="0.2"/>')
