print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py   |"
print "|--------------------------------------------|"


### Load data/sbm-common assets
scene.addAssetPath("script", "sbm-common/scripts")
scene.addAssetPath("motion", "sbm-common/common-sk")
scene.loadAssets()

scene.setBoolAttribute("internalAudio", True)

scene.run("init-common-face.py")

doctor = scene.createCharacter("doctor", "")
doctorSkeleton = scene.createSkeleton("common.sk")
doctor.setSkeleton(doctorSkeleton)
doctor.setFaceDefinition(defaultFace)
doctorPos = SrVec(-35, 107, 0)
doctor.setPosition(doctorPos)
doctorHPR = SrVec(0, 0, 0)
doctor.setHPR(doctorHPR)
doctor.createStandardControllers()

brad = scene.createCharacter("brad", "")
bradSkeleton = scene.createSkeleton("common.sk")
brad.setSkeleton(bradSkeleton)
brad.setFaceDefinition(defaultFace)
bradPos = SrVec(35, 107, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(0, 0, 0)
brad.setHPR(bradHPR)
brad.createStandardControllers()

# start the simulation
sim.start()
bml.execBML('doctor', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('doctor', '<saccade mode="listen"/>')
bml.execBML('brad', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('brad', '<saccade mode="listen"/>')
sim.resume()

