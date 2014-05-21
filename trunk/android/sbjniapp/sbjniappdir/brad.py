print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/brad.py  |"
print "|--------------------------------------------|"


### Load data/sbm-common assets
scene.setMediaPath('/sdcard/sbjniappdir/')
scene.addAssetPath("script", "./")
scene.addAssetPath('motion', './')
scene.addAssetPath("mesh", "mesh")
scene.loadAssets()
scene.setScale(1.0)

# set the default camera 
camera = scene.getActiveCamera()
camera.setEye(0, 2.0478, 4.69259)
camera.setCenter(0.012, 1.60887, 2.75628)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(0.4)
camera.setFarPlane(100)
camera.setNearPlane(0.1)

# map to the SmartBody standard
scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
bradMotion = scene.getMotion('ChrBrad@Idle01_BeatHighBt01')
zebra2Map.applyMotion(bradMotion)

brad = scene.createCharacter("ChrBrad", "")
bradSkeleton = scene.createSkeleton("ChrBrad.sk")
brad.setSkeleton(bradSkeleton)
bradHPR = SrVec(0, 0, 0)
brad.setHPR(bradHPR)
bradPos = SrVec(0.0, 0.0, 0.0)
brad.setPosition(bradPos)
brad.createStandardControllers()
# deformable mesh
brad.setStringAttribute("deformableMesh", "ChrBradFlash.dae")

# start the simulation
sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01_BeatHighBt01"/>')
bml.execBML('ChrBrad', '<saccade mode="listen"/>')
sim.resume()

bml.execBML('ChrBrad', '<gaze sbm:handle="flash" sbm:target-pos="0 0 0"/>')
bml.execBML('ChrBrad', '<gaze sbm:handle="flash" sbm:fade-out="0.2"/>')


