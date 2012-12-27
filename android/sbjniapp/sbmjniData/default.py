scene.addAssetPath('ME', '/sdcard/sbmjniData/')
print ">>> Loading common motions and poses..."
scene.loadAssets()

doctor = scene.createCharacter("doctor", "SasoBase.SasoDoctorPerez")
doctorSkeleton = scene.createSkeleton("common.sk")
doctor.setSkeleton(doctorSkeleton)
doctorPos = SrVec(35, 102, 0)
doctor.setPosition(doctorPos)
doctorHPR = SrVec(-17, 0, 0)
doctor.setHPR(doctorHPR)
doctor.createStandardControllers()


sim.start()
bml.execBML('doctor', '<body posture="HandsAtSide_Arms_Sweep"/>')
