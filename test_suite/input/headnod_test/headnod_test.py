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
camera.setEye(.78, 1.36, 1.0)
camera.setCenter(.4, 1.36, 0.33)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

scene.command("char ChrBrad viewer deformableGPU")

sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<head type="NOD" id="b1"/>')
scene.commandAt(2.7, 'snapshot')
bml.execBMLAt(6, 'ChrBrad', '<head type="NOD" amount="0.4" repeats="1.0"/>')
scene.commandAt(6.3, 'snapshot')
bml.execBMLAt(9, 'ChrBrad', '<head type="NOD" amount="0.6" repeats="2.0"/>')
scene.commandAt(9.2, 'snapshot')
scene.commandAt(9.7, 'snapshot')
bml.execBMLAt(14, 'ChrBrad', '<head type="NOD" amount="1.0" repeats="1.5"/>')
scene.commandAt(14.1, 'snapshot')
scene.commandAt(14.6, 'snapshot')

scene.commandAt(15, 'quit')