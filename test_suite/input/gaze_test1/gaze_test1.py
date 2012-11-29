scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.getSimulationManager().setSimFps(60)

scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath('motion','ChrBrad')
scene.loadAssets()

bradSkeleton = scene.getSkeleton("ChrBrad.sk")
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse("ChrBrad")

brad = scene.createCharacter('ChrBrad', 'ChrBrad')
bradSkeleton = scene.createSkeleton('ChrBrad.sk')
brad.setSkeleton(bradSkeleton)
brad.createStandardControllers()
brad.setDoubleAttribute('deformableMeshScale', .01)
brad.setStringAttribute('deformableMesh', 'ChrBrad')

scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.1, 1.53, 1.35)
camera.setCenter(.1, 1.25, 0.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")

sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<gaze angle="60" direction="POLAR" sbm:joint-range="NECK" target="camera"/>')
scene.commandAt(3, 'snapshot')
bml.execBMLAt(4, 'ChrBrad', '<gaze angle="-60" direction="POLAR" sbm:joint-range="NECK" target="camera"/>')
scene.commandAt(5, 'snapshot')
bml.execBMLAt(6, 'ChrBrad', '<gaze angle="40" direction="POLAR" sbm:joint-range="CHEST" target="camera"/>')
bml.execBMLAt(6, 'ChrBrad', '<gaze angle="10" direction="UPRIGHT" sbm:joint-range="NECK" target="camera"/>')
scene.commandAt(7, 'snapshot')
bml.execBMLAt(8, 'ChrBrad', '<gaze angle="-80" direction="POLAR" sbm:joint-range="CHEST" target="camera"/>')
bml.execBMLAt(8, 'ChrBrad', '<gaze angle="20" direction="POLAR" sbm:joint-range="NECK" target="camera"/>')
scene.commandAt(9, 'snapshot')
bml.execBMLAt(10, 'ChrBrad', '<gaze sbm:joint-range="CHEST" target="camera"/>')
bml.execBMLAt(10, 'ChrBrad', '<gaze angle="-20" direction="DOWNLEFT" sbm:joint-range="NECK" target="camera"/>')
scene.commandAt(11, 'snapshot')
bml.execBMLAt(12, 'ChrBrad', '<gaze angle="-25" direction="DOWNLEFT" sbm:joint-range="NECK" target="camera"/>')
scene.commandAt(13, 'snapshot')

scene.commandAt(14, 'quit')