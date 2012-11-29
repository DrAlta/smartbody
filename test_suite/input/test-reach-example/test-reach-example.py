scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.getSimulationManager().setSimFps(60)

scene.run("motion-retarget.py")
scene.run("init-param-animation.py")

scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'retarget\motion')
scene.addAssetPath('motion', 'sbm-common/common-sk')
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
retargetCharacter('ChrBrad', 'ChrBrad.sk')

# Setting camera parameters
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.21, 1.67, 2.52)
camera.setCenter(.13, 1.35, 1.0)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, -1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")

pawn1 = scene.createPawn('pawn1')
pawn1.setPosition(SrVec(.2, 1.5, .4))
pawn2 = scene.createPawn('pawn2')
pawn2.setPosition(SrVec(.0, 1.2, .45))
pawn3 = scene.createPawn('pawn3')
pawn3.setPosition(SrVec(.2, .0, .45))
pawn4 = scene.createPawn('pawn4')
pawn4.setPosition(SrVec(-.1, .7, .45))

sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

scene.commandAt(1, 'snapshot')
bml.execBMLAt(1, 'ChrBrad', '<sbm:reach sbm:handle="cbrad" sbm:action="touch"/>')
bml.execBMLAt(2, 'ChrBrad', '<sbm:reach sbm:handle="cbrad" target="pawn1" sbm:fade-in="1.0"/>')
scene.commandAt(3, 'snapshot')
bml.execBMLAt(4, 'ChrBrad', '<sbm:reach sbm:handle="cbrad" target="pawn2"/>')
scene.commandAt(5, 'snapshot')
bml.execBMLAt(6, 'ChrBrad', '<sbm:reach sbm:handle="cbrad" target="pawn3" sbm:reach-finish="true"/>')
scene.commandAt(7.2, 'snapshot')
bml.execBMLAt(9, 'ChrBrad', '<sbm:reach sbm:handle="cbrad" target="pawn4" sbm:reach-finish="true"/>')
scene.commandAt(10, 'snapshot')

scene.commandAt(11, 'quit')