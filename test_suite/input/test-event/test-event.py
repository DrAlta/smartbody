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
camera.setEye(.0, 1.2, 1.5)
camera.setCenter(.4, 1.2, 0)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")

pawn1 = scene.createPawn('pawn1')
pawn1.setPosition(SrVec(3.0, 1.2, -2.0))

sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bradBML1 = "<gaze target=\"pawn1\"/>"
bradBML2 = "<gaze sbm:fade-out=\"1.0\"/>"
bradName = "ChrBrad"
bml.execBMLAt(2, 'ChrBrad', '<animation id="m1" name="ChrBrad@Idle01_MeLf01"/><sbm:event stroke="m1:stroke" message="sbm python bml.execBML(bradName, bradBML1)"/>')
scene.commandAt(3, 'snapshot')
scene.commandAt(3.5, 'snapshot')
bml.execBMLAt(5, 'ChrBrad', '<animation id="m2" name="ChrBrad@Idle01_YouLf01"/><sbm:event stroke="m2:stroke" message="sbm python bml.execBML(bradName, bradBML2)"/>')
scene.commandAt(6, 'snapshot')

scene.commandAt(7, 'quit')