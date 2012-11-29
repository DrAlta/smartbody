scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.getSimulationManager().setSimFps(60)

scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath('motion','ChrBrad')
scene.addAssetPath('motion','ChrRachel')
scene.loadAssets()

# Brad
bradSkeleton = scene.getSkeleton("ChrBrad.sk")
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse("ChrBrad")

brad = scene.createCharacter('ChrBrad', 'ChrBrad')
bradSkeleton = scene.createSkeleton('ChrBrad.sk')
brad.setSkeleton(bradSkeleton)
brad.createStandardControllers()
brad.setPosition(SrVec(.35, 0, -.17))
# Deformable mesh
brad.setDoubleAttribute('deformableMeshScale', .01)
brad.setStringAttribute('deformableMesh', 'ChrBrad')

# Rachel
rachelSkeleton = scene.getSkeleton("ChrRachel.sk")
zebra2Map.applySkeleton(rachelSkeleton)
zebra2Map.applyMotionRecurse("ChrRachel")

rachel = scene.createCharacter('ChrRachel', 'ChrRachel')
rachelSkeleton = scene.createSkeleton('ChrRachel.sk')
rachel.setSkeleton(rachelSkeleton)
rachel.createStandardControllers()
rachel.setPosition(SrVec(-.35, 0, .17))

# Deformable mesh
rachel.setDoubleAttribute('deformableMeshScale', .01)
rachel.setStringAttribute('deformableMesh', 'ChrRachel')

scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 1.66, 1.85)
camera.setCenter(0, .92, 0)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")
scene.command("char ChrRachel viewer deformableGPU")

sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBML('ChrRachel', '<body posture="ChrRachel_ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<animation name="ChrBrad@Guitar01"/>')
bml.execBMLAt(2, 'ChrRachel', '<animation name="ChrRachel_ChrBrad@Guitar01"/>')
scene.commandAt(4, 'snapshot')
scene.commandAt(6, 'snapshot')
bml.execBMLAt(9, 'ChrBrad', '<animation name="ChrBrad@Idle01_ExampleLf01"/>')
bml.execBMLAt(9, 'ChrRachel', '<animation name="ChrRachel_ChrBrad@Idle01_ExampleLf01"/>')
scene.commandAt(10, 'snapshot')
scene.commandAt(11, 'snapshot')

scene.commandAt(12, 'quit')