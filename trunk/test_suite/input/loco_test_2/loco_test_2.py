scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh','mesh')

scene.getSimulationManager().setSimFps(60)

scene.run("motion-retarget.py")
scene.run("init-param-animation.py")

scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'ChrRachel')
scene.addAssetPath('motion', 'retarget\motion')
scene.addAssetPath('motion', 'sbm-common/common-sk')
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
retargetCharacter('ChrBrad', 'ChrBrad.sk')

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
retargetCharacter('ChrRachel', 'ChrRachel.sk')

scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.4, 3.62, 9.4)
camera.setCenter(.4, 3.3, 8.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.96897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")
scene.command("char ChrRachel viewer deformableGPU")

scene.run("init-steer-agents.py")
steerManager = scene.getSteerManager()
setupSteerAgent('ChrRachel','ChrRachel.sk')
setupSteerAgent('ChrBrad','ChrBrad.sk')
steerManager.setEnable(False)
brad.setBoolAttribute('steering.pathFollowingMode', True)
rachel.setBoolAttribute('steering.pathFollowingMode', True)
steerManager.setEnable(True)

sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBML('ChrRachel', '<body posture="ChrRachel_ChrBrad@Idle01"/>')
'''FOLLOW DOESN'T WORK ON NEW CHARACTERS'''
bml.execBMLAt(2, 'ChrRachel', '<locomotion manner="run" target="3 3"/>')
bml.execBMLAt(4, 'ChrBrad', '<locomotion manner="run" target="ChrRachel"/>')
scene.commandAt(4.5, 'snapshot')

bml.execBMLAt(6, 'ChrRachel', '<locomotion manner="jog" target="-3 3"/>')
bml.execBMLAt(8, 'ChrBrad', '<locomotion manner="run" target="ChrRachel"/>')
scene.commandAt(8.5, 'snapshot')

bml.execBMLAt(10, 'ChrRachel', '<locomotion manner="walk" target="-3 -3"/>')
bml.execBMLAt(12, 'ChrBrad', '<locomotion manner="run" target="ChrRachel"/>')
scene.commandAt(14, 'snapshot')

scene.commandAt(15, 'quit')