
scene.addAssetPath('ME', '/sdcard/SbmOgre/ogre/sbm/')
print ">>> Loading common motions and poses..."
scene.loadAssets()
print ">>> Finish loading motions and poses..."
brad = scene.createCharacter("brad", "SasoBase.SasoDoctorPerez")
print ">>> Finish create character..."
bradSkeleton = scene.createSkeleton("common.sk")
print ">>> Finish create skeleton..."
brad.setSkeleton(bradSkeleton)
print ">>> Finish set skeleton..."
bradPos = SrVec(-35, 102, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(-17, 0, 0)
brad.setHPR(bradHPR)
brad.createStandardControllers()
print ">>> Finish creating standard control..."

sim.start()
bml.execBML('brad', '<body posture="HandsAtSide_Motex"/>')
#0 char brad init common.sk
#0 set character brad world_offset x -35 y 102 h -17
#0 char doctor init common.sk
#0 set character doctor world_offset x 35 y 102 h -17  
#1 bml char brad <body posture="HandsAtSide_Motex"/>
#1 bml char doctor <body posture="HandsAtSide_Motex"/>
