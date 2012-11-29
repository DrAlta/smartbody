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

# Setting camera parameters
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.13, 1.46, 2.2)
camera.setCenter(.4, .7, 0)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

pawn1 = scene.createPawn('pawn1')
pawn1.setPosition(SrVec(.3, 1.0, .2))
pawn2 = scene.createPawn('pawn2')
pawn2.setPosition(SrVec(-.16, 1.0, .13))
pawn3 = scene.createPawn('pawn3')
pawn3.setPosition(SrVec(-2.0, 1.2, 1.0))
pawn4 = scene.createPawn('pawn4')
pawn4.setPosition(SrVec(3.0, 1.2, -2.0))

scene.command("char ChrBrad viewer deformableGPU")

sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBMLAt(2, 'ChrBrad', '<animation name="ChrBrad@Idle01_ChopBoth01"/>')
scene.commandAt(3.45, 'snapshot')

bml.execBMLAt(2, 'ChrBrad', '<gaze target="pawn3"/>')
scene.commandAt(4, 'snapshot')
bml.execBMLAt(8, 'ChrBrad', '<animation name="ChrBrad@Idle01_ChopBoth01"/>')
bml.execBMLAt(9, 'ChrBrad', '<sbm:constraint effector="l_wrist" sbm:effector-root="l_sternoclavicular" sbm:handle="cbrad" target="pawn1"/>')
bml.execBMLAt(9, 'ChrBrad', '<sbm:constraint effector="r_wrist" sbm:effector-root="r_sternoclavicular" sbm:handle="cbrad" target="pawn2"/>')
bml.execBMLAt(9, 'ChrBrad', '<sbm:constraint sbm:handle="cbrad" sbm:fade-in="0.5"/>')
bml.execBMLAt(9.2, 'ChrBrad', '<gaze target="pawn4"/>')
scene.commandAt(9.5, 'snapshot')
scene.commandAt(10, 'snapshot')

scene.commandAt(11, 'quit')