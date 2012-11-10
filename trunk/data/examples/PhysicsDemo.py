import random
print "|--------------------------------------------|"
print "|          Starting Physics Demo             |"
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
scene.getSimulationManager().setSimFps(60)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad')
setPos('brad', SrVec(-150, 200, 20))
addCharacter('elder', 'elder')
setPos('elder', SrVec(-75, 102, 0))
addCharacter('doctor', 'doctor', True)
setPos('doctor', SrVec(75, 102, 0))
setFacing('doctor', 90)
addCharacter('brad2', 'brad', True)
setPos('brad2', SrVec(135, 102, 0))
setFacing('brad2', -90)

# Add pawns in scene
addPawn('constraint1', 'sphere', SrVec(1, 1, 1))
scene.getPawn('constraint1').setPosition(SrVec(-150, 240, 20))
addPawn('phy1', 'sphere')
scene.getPawn('phy1').setPosition(SrVec(-75, 150, 20))

# Targets
addPawn('target1', 'sphere', SrVec(0, 0, 0))
addPawn('target2', 'sphere', SrVec(0, 0, 0))
addPawn('target3', 'sphere', SrVec(0, 0, 0))
addPawn('target4', 'sphere', SrVec(0, 0, 0))
setPawnPos('target1', SrVec(75, 150, 10))
setPawnPos('target2', SrVec(75, 150, -10))
setPawnPos('target3', SrVec(135, 150, 10))
setPawnPos('target4', SrVec(135, 150, -10))

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
# Doctor physics and constraints
setupCharacterPhysics('doctor')
constrainChr('doctor', 'spine1')
constrainChr('doctor', 'r_wrist')
constrainChr('doctor', 'l_wrist')
constrainChr('doctor', 'r_ankle')
constrainChr('doctor', 'l_ankle')
# Brad2 physics and constraints
setupCharacterPhysics('brad2')
constrainChr('brad2', 'spine1')
constrainChr('brad2', 'r_wrist')
constrainChr('brad2', 'l_wrist')
constrainChr('brad2', 'r_ankle')
constrainChr('brad2', 'l_ankle')
# Elder physics and contraints
setupCharacterPhysics('elder')
constrainChr('elder', 'spine1')
constrainChr('elder', 'l_wrist')
constrainChr('elder', 'r_wrist')
constrainChr('elder', 'l_ankle')
constrainChr('elder', 'r_ankle')
# Setup pawn physics
setupPawnPhysics('phy1')

bradX = -150
bradCur = -1
curZ = 20
curX = -75
amountZ = amountX = -1
speed = 0.2
last = 0
canTime = True
delay = 6
started = False
class PhysicsDemo(SBScript):
	def update(self, time):
		global canTime, last, started
		global amountZ, curZ, amountX, curX, bradX, bradCur
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
			togglePhysics('doctor')
			togglePhysics('elder')
			togglePawnPhysics('phy1')
			togglePawnPhysics('phy1')
			setPawnPos('phy1', SrVec(-75, 150, 20))
			# Do gaze for elder
			bml.execBML('brad', '<body posture="Walk"/>')
		# If time's up, do action
		if canTime:
			bml.execBML('brad', '<head repeats="5" velocity="0.75" type="SHAKE"/>')
			boxingLogic()
		# Elder pawn
		setPawnPos('phy1', SrVec(curX, 150, curZ))
		curX = curX + speed * amountX
		curZ = curZ + speed * amountZ
		if curX < -90: amountX = 1
		if curX > -60: amountX = -1
		if curZ < 9: amountZ = 1
		if curZ > 20: amountZ = -1
		# Brad pawn
		setPawnPos('constraint1', SrVec(bradX, 240, 20))
		bradX = bradX + speed * bradCur
		if bradX < -170: bradCur = 1
		if bradX > -130: bradCur = -1
	
# Current turn
currentTurn = 'brad2'		
def boxingLogic():
	global currentTurn
	# Brad's turn, toggle physics and play reach
	if currentTurn == 'brad2':
		togglePhysics('brad2', 'off')
		togglePhysics('doctor', 'on')
		randNum = random.randrange(0, 2)
		randDodge = random.randrange(0, 3)
		if randNum == 0:
			bml.execBML('brad2', '<sbm:reach sbm:action="touch" sbm:reach-finish="true" sbm:reach-type="left" target="target1"/>')
		elif randNum == 1:
			bml.execBML('brad2', '<sbm:reach sbm:action="touch" sbm:reach-finish="true" sbm:reach-type="right" target="target2"/>')
		if randDodge == 2:
			togglePhysics('doctor', 'off')
			bml.execBML('doctor', '<animation name="ChrUtah_Relax001_CrouchProtectHead_right"/>')
		currentTurn = 'doctor'
	# Doctor's turn, toggle physics and play reach
	elif currentTurn == 'doctor':
		togglePhysics('doctor', 'off')
		togglePhysics('brad2', 'on')
		randNum = random.randrange(0, 2)
		randDodge = random.randrange(0, 3)
		if randNum == 0:
			bml.execBML('doctor', '<sbm:reach sbm:action="touch" sbm:reach-finish="true" sbm:reach-type="right" target="target3"/>')
		elif randNum == 1:
			bml.execBML('doctor', '<sbm:reach sbm:action="touch" sbm:reach-finish="true" sbm:reach-type="left" target="target4"/>')
		if randDodge == 2:
			togglePhysics('brad2', 'off')
			bml.execBML('brad2', '<animation name="ChrUtah_Relax001_CrouchProtectHead_right"/>')
		currentTurn = 'brad2'
		
class CollisionHandler(EventHandler):
	def executeAction(self, ev):
		params = ev.getParameters()
		list = params.split()
		# Elder collision
		if list[1] == 'elder':
			if list[2] == 'phy1':
				#print '%s collided with %s' % (list[1], list[2])
				gaze('elder', 'phy1', 300)
				stopGaze('elder', 2, 300)
		# Doctor doesn't get collision events for some reason
		if list[1] == 'brad2':
			# Brad hits doctor
			if 'wrist' in list[2] and currentTurn == 'doctor':
				target = list[3] + ' ' + list[4] + ' ' + list[5]
				gaze('doctor', target, 400)
				stopGaze('doctor', 1.5, 300)
			# Doctor hits brad
			elif '_' in list[2] and currentTurn == 'brad2':
				target = list[3] + ' ' + list[4] + ' ' + list[5]
				gaze('brad2', target, 400)
				stopGaze('brad2', 1.5, 300)
				
collisionHdl = CollisionHandler()
evtMgr = scene.getEventManager()
evtMgr.addEventHandler('collision', collisionHdl)
			
# Run the update script
scene.removeScript('physicsdemo')
physicsdemo = PhysicsDemo()
scene.addScript('physicsdemo', physicsdemo)