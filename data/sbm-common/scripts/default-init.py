print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py  |"
print "|--------------------------------------------|"

### Load data/sbm-common assets
### Assumes current directory is: core/smartbody/sbm/bin
scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.addAssetPath("seq", "../../../../data/sbm-test/scripts")
scene.addAssetPath("mesh", "../../../../data/mesh")

scene.run("init-common-assets.py")
#0	seq init-general-parameters
scene.run("init-common-face.py")
#0	panim enable

doctor = scene.createCharacter("doctor", "SasoBase.SasoDoctorPerez")
doctorSkeleton = scene.createSkeleton("common.sk")
doctor.setSkeleton(doctorSkeleton)
doctorPos = SrVec(35, 102, 0)
doctor.setPosition(doctorPos)
doctorHPR = SrVec(-17, 0, 0)
doctor.setHPR(doctorHPR)
doctor.setVoice("remote")
doctor.setVoiceCode("M021")
doctor.createStandardControllers()
doctor.setStringAttribute("mesh", "doctor")

elder = scene.createCharacter("elder", "SasoBase.Mayor")
elderSkeleton = scene.createSkeleton("common.sk")
elder.setSkeleton(elderSkeleton)
elderPos = SrVec(-35, 102, 0)
elder.setPosition(elderPos)
elderHPR = SrVec(17, 0, 0)
elder.setHPR(elderHPR)
elder.setVoice("remote")
elder.setVoiceCode("M009")
elder.createStandardControllers()
elder.setStringAttribute("mesh", "elder")

brad = scene.createCharacter("brad", "")
bradSkeleton = scene.createSkeleton("common.sk")
brad.setSkeleton(bradSkeleton)
bradPos = SrVec(135, 102, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(-17, 0, 0)
brad.setHPR(bradHPR)
brad.setVoice("remote")
brad.setVoiceCode("star")
brad.createStandardControllers()
brad.setStringAttribute("mesh", "brad")

utah = scene.createCharacter("utah", "")
utahSkeleton = scene.createSkeleton("test_utah.sk")
utah.setSkeleton(utahSkeleton)
utahPos = SrVec(-135, 0, 0)
utah.setPosition(utahPos)
utahHPR = SrVec(-17, 0, 0)
utah.setHPR(utahHPR)
utah.setVoice("remote")
utah.setVoiceCode("star")
utah.createStandardControllers()
utah.setStringAttribute("mesh", "utah")

scene.setDefaultCharacter("doctor")
scene.setDefaultRecipient("elder")

scene.run("default-viewer.py")

# start the simulation
sim.start()

bml.execBML('doctor', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('elder', '<body posture="LHandOnHip_Motex"/>')
bml.execBML('brad', '<body posture="HandsAtSide_Motex"/>')
bml.execBML('utah', '<body posture="HandsAtSide_Motex"/>')

bml.execBML('doctor', '<saccade mode="listen"/>')
bml.execBML('elder', '<saccade mode="listen"/>')
bml.execBML('brad', '<saccade mode="listen"/>')
bml.execBML('utah', '<saccade mode="listen"/>')

#1	seq init-param-animation inline
#2	seq init-example-reach inline



#3 steer stateprefix doctor all
#3 steer stateprefix elder all
#3 steer stateprefix utah all
#3 steer stateprefix brad all

#4 steer start
#5 steer type doctor example
#5 steer type elder example
#5 steer type utah example
#5 steer type brad example