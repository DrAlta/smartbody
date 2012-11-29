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
pawn3.setPosition(SrVec(.4, 1.0, .40))
pawn4 = scene.createPawn('pawn4')
pawn4.setPosition(SrVec(.0, 1.2, -.22))

# Create physics for the pawns
for name in scene.getPawnNames():
	if 'pawn' in name:
		pawn = scene.getPawn(name)
		pawn.setStringAttribute('collisionShape', 'box')
		pawn.getAttribute('collisionShapeScale').setValue(SrVec(0.05, 0.05, 0.05))
		pawn.getAttribute('createPhysics').setValue()
		#pawn.setBoolAttribute('enablePhysics', True)
		
scene.getPhysicsManager().getPhysicsEngine().setBoolAttribute('enable', True)

lastTime = 0
canTime = True
class Timer(SBScript):
	def update(self, time):
		global canTime, lastTime
		if canTime:
			lastTime = time
			canTime = False
		diff = time - lastTime
		if diff > 2:
			canTime = True
			diff = 0
		if canTime:
			pawn1.setBoolAttribute('enablePhysics', True)
			pawn2.setBoolAttribute('enablePhysics', True)
			pawn3.setBoolAttribute('enablePhysics', True)
			pawn4.setBoolAttribute('enablePhysics', True)
			
scene.removeScript('timer')
timer = Timer()
scene.addScript('timer', timer)
	
sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

scene.commandAt(1, 'snapshot')
bml.execBMLAt(2, 'ChrBrad', '<sbm:reach sbm:action="pick-up" target="pawn1" sbm:fade-in="1.0"/>')
bml.execBMLAt(5, 'ChrBrad', '<sbm:reach sbm:action="pick-up" sbm:reach-finish="true"/>')
scene.commandAt(5.5, 'snapshot')
bml.execBMLAt(7, 'ChrBrad', '<sbm:reach sbm:action="put-down" sbm:target-pos=".4, 1.4, .4" sbm:reach-finish="true"/>')
scene.commandAt(9, 'snapshot')

bml.execBMLAt(10, 'ChrBrad', '<sbm:reach sbm:action="pick-up" target="pawn2" sbm:fade-in="1.0"/>')
bml.execBMLAt(13, 'ChrBrad', '<sbm:reach sbm:action="pick-up" sbm:reach-finish="true"/>')
scene.commandAt(13.5, 'snapshot')
bml.execBMLAt(16, 'ChrBrad', '<sbm:reach sbm:action="put-down" sbm:target-pos=".0, 1.4, -.2" sbm:reach-finish="true"/>')
scene.commandAt(21, 'snapshot')

scene.commandAt(22, 'quit')