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
camera.setEye(0.88, 1.48, 1.30)
camera.setCenter(0.28, 1.07, 0.12)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

scene.command("char ChrBrad viewer deformableGPU")

sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<gaze sbm:target-pos="0 1.70 1.00"/>')
scene.commandAt(3, 'snapshot')
bml.execBMLAt(4, 'ChrBrad', '<gaze sbm:target-pos="1.00 2.00 -.40"/>')
camera.setEye(1.60, 1.50, 0)
scene.commandAt(5, 'snapshot')
bml.execBMLAt(6, 'ChrBrad', '<gaze sbm:target-pos="1.00 1.70 -.40"/>')
scene.commandAt(7, 'snapshot')
bml.execBMLAt(8, 'ChrBrad', '<gaze sbm:target-pos="1.00 1.00 -.40"/>')
scene.commandAt(9, 'snapshot')

scene.commandAt(10, 'quit')