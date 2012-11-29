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
camera.setEye(.72, 1.44, 1.0)
camera.setCenter(.35, 1.44, 0.33)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

scene.command("char ChrBrad viewer deformableGPU")

sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<head type="SHAKE" id="b1"/>')
scene.commandAt(2.3, 'snapshot')
bml.execBMLAt(3, 'ChrBrad', '<head type="SHAKE" amount="0.2" repeats="0.5"/>')
scene.commandAt(3.3, 'snapshot')
bml.execBMLAt(4, 'ChrBrad', '<head type="SHAKE" amount="0.8" repeats="2.0"/>')
scene.commandAt(4.3, 'snapshot')
scene.commandAt(4.6, 'snapshot')
bml.execBMLAt(6, 'ChrBrad', '<head type="SHAKE" amount="2.8" repeats="1.5"/>')
scene.commandAt(6.3, 'snapshot')
scene.commandAt(6.6, 'snapshot')

scene.commandAt(7, 'quit')