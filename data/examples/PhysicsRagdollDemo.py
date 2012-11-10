import random
print "|--------------------------------------------|"
print "|      Starting Physics Ragdoll Demo         |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath("script","../../../../data/examples")
scene.addAssetPath("script","../../../../data/examples/functions")
scene.addAssetPath("script","../../../../data/sbm-common/scripts")
scene.addAssetPath('seq', '../../../../data/sbm-common/scripts')
scene.addAssetPath('seq', '../../../../data/sbm-test/scripts')
scene.addAssetPath('mesh', '../../../../data/mesh')
scene.addAssetPath('mesh', '../../../../data/retarget/mesh')
scene.addAssetPath('audio', '../../../../data/Resources/audio')

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(-9, 255, 417)
camera.setCenter(-9, 182, 232)

# Set simulation fps
#scene.getSimulationManager().setSimFps(60

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad')
setPos('brad', SrVec(0, 200, 0))

addCharacter('brad2', 'brad')
setPos('brad2', SrVec(100, 150, 0))
#addCharacter('brad3', 'brad')
#setPos('brad3', SrVec(-100, 150, -100))

bradList = []
for name in scene.getCharacterNames():
	if name == 'brad': continue
	bradList.append(scene.getCharacter(name))

# Add pawns in scene
addPawn('constraint1', 'sphere', SrVec(10, 10, 10))
scene.getPawn('constraint1').setPosition(SrVec(0, 240, 0))
addPawn('phy1', 'sphere')
scene.getPawn('phy1').setPosition(SrVec(75, 150, 0))

# Targets
addPawn('target1', 'box', SrVec(10, 10, 10))
addPawn('target2', 'sphere', SrVec(15, 15, 15))
addPawn('target3', 'box', SrVec(10, 10, 10))
addPawn('target4', 'box', SrVec(10, 10, 10))
addPawn('target5', 'box', SrVec(10, 10, 10))
setPawnPos('target1', SrVec(100, 150, 125))
setPawnPos('target2', SrVec(-75, 50, 100))
setPawnPos('target3', SrVec(-75, 100, -50))
setPawnPos('target4', SrVec(100, 30, -100))
setPawnPos('target5', SrVec(-100, 30, 90))

addPawn('wall1', 'box', SrVec(200, 50, 10))
addPawn('wall2', 'box', SrVec(200, 50, 10))
addPawn('wall3', 'box', SrVec(10, 50, 200))
addPawn('wall4', 'box', SrVec(10, 50, 200))
setPawnPos('wall1', SrVec(0, 0, 211))
setPawnPos('wall2', SrVec(0, 0, -211))
setPawnPos('wall3', SrVec(200, 0, 0))
setPawnPos('wall4', SrVec(-200, 0, 0))

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Add Gaze script
scene.run('Gaze.py')

# Add Physics script
scene.run('Physics.py')

# Set up physics and constraints
setupCharacterPhysics('brad')
constrainChr('brad', 'l_wrist', 'constraint1')
setupCharacterPhysics('brad2')
#setupCharacterPhysics('brad3')
''' MORE THAN 2 RAGDOLLS SEEM TO AFFECT FRAMERATE '''
# Setup pawn physics
setupPawnPhysics('phy1')
setupPawnPhysics('target1')
setupPawnPhysics('target2')
setupPawnPhysics('target3')
setupPawnPhysics('target4')
setupPawnPhysics('target5')
setupPawnPhysics('wall1')
setupPawnPhysics('wall2')
setupPawnPhysics('wall3')
setupPawnPhysics('wall4')

# Physics manager and forces
phyManager = scene.getPhysicsManager()
forceX = forceZ = 150
bradRagdoll = False

# Enable collision
collisionManager = getScene().getCollisionManager()
collisionManager.setStringAttribute('collisionResolutionType', 'default')
collisionManager.setEnable(True)

curZ = 0
curX = 0
amountZ = amountX = -1
speed = 0.05
last = 0
canTime = True
delay = 10
started = False
class PhysicsDemo(SBScript):
	def update(self, time):
		global canTime, last, started, amountZ, curZ, amountX, curX
		global forceX, forceZ, bradRagdoll
		if canTime:
			last = time
			canTime = False
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		# Trigger once
		if canTime and not started:
			started = True
			togglePhysics('brad')
			togglePawnPhysics('target2')
			# Do gaze for elder
			bml.execBML('brad', '<body posture="Walk"/>')
		# If time's up, do action
		if canTime:
			forceX = random.uniform(-200, 200)
			forceZ = random.uniform(-200, 200)
			phyManager.applyForceToPawn('target2', SrVec(forceX, 0, forceZ))
			randX = random.uniform(-50000, 50000)
			randY = random.uniform(-50000, 50000)
			randZ = random.uniform(-50000, 50000)
			phyManager.applyForceToCharacter('brad', 'spine1', SrVec(randX, randY, randZ))
			if not bradRagdoll:
				bradRagdoll = True
				# Brad 2
				for brad in bradList:
					togglePhysics(brad.getName(), 'on')
			elif bradRagdoll:
				bradRagdoll = False
				for brad in bradList:
					togglePhysics(brad.getName(), 'off')
					# Randomize position and rotation
					randX = random.uniform(-150, 150)
					randY = random.uniform(102, 200)
					randZ = random.uniform(0, 150)
					randH = random.uniform(-180, 180)
					randP = random.uniform(-180, 180)
					randR = random.uniform(-180, 180)
					scene.getCharacter(brad.getName()).setPosition(SrVec(randX, randY, randZ))
					scene.getCharacter(brad.getName()).setHPR(SrVec(randH, randP, randR))
		# Physical pawn
		setPawnPos('phy1', SrVec(curX, 150, curZ))
		curX = curX + speed * amountX
		curZ = curZ + speed * amountZ
		if curX < -30: amountX = 1
		if curX > 30: amountX = -1
		if curZ < 5: amountZ = 1
		if curZ > 20: amountZ = -1
	
class CollisionHandler(EventHandler):
	def executeAction(self, ev):
		params = ev.getParameters()
		list = params.split()
		#print list
		# Brad collision
		if len(list) > 1:
			if list[1] == 'brad':
				if len(list) > 2:
					if list[2] == 'phy1':
						#print '%s collided with %s' % (list[1], list[2])
						gaze('brad', 'phy1', 300)
						stopGaze('brad', 2, 300)
				
collisionHdl = CollisionHandler()
evtMgr = scene.getEventManager()
evtMgr.addEventHandler('collision', collisionHdl)
			
# Run the update script
scene.removeScript('physicsdemo')
physicsdemo = PhysicsDemo()
scene.addScript('physicsdemo', physicsdemo)