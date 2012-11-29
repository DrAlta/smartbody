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

scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.4, 3.62, 9.4)
camera.setCenter(.4, 3.3, 8.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")

scene.run("init-steer-agents.py")
steerManager = scene.getSteerManager()
setupSteerAgent('ChrBrad','ChrBrad.sk')	
steerManager.setEnable(False)
brad.setBoolAttribute('steering.pathFollowingMode', True)
steerManager.setEnable(True)

sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<locomotion manner="walk" target="3.0 3.0 0"/>')
scene.commandAt(4, 'snapshot')
bml.execBMLAt(6, 'ChrBrad', '<locomotion manner="jog" target="-3.0 -3.0 0"/>')
scene.commandAt(10, 'snapshot')
bml.execBMLAt(12, 'ChrBrad', '<locomotion manner="run" target="-3.0 3.0 0"/>')
scene.commandAt(14, 'snapshot')

scene.commandAt(15, 'quit')