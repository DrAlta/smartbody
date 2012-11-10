import random
print "|--------------------------------------------|"
print "|            Starting Reach Demo             |"
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
camera.setEye(0, 345.99, 502.63)
camera.setCenter(0, 246.80, 329.83)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad', True)
setPos('brad', SrVec(-35, 102, -100))
addCharacter('elder', 'elder', True)
setPos('elder', SrVec(-135, 102, -100))
addCharacter('utah', 'utah', True)
setPos('utah', SrVec(135, 0, 0))

# Multliple passing
addCharacter('doctor1', 'doctor', True)
setPos('doctor1', SrVec(-135, 102, 100))
setFacing('doctor1', 90)
addCharacter('doctor2', 'doctor', True)
setPos('doctor2', SrVec(-35, 102, 100))
setFacing('doctor2', -90)

# Add pawns in scene
addPawn('bradPawn', 'sphere')
setPawnPos('bradPawn', SrVec(0, 40, -130))
addPawn('elderPawn', 'sphere')
setPawnPos('elderPawn', SrVec(-175, 200, -35))
addPawn('utahPawn', 'sphere')
setPawnPos('utahPawn', SrVec(135, 200, -200))
addPawn('doctorPawn', 'sphere', SrVec(5, 5, 5))
setPawnPos('doctorPawn', SrVec(-85, 200, 100))

# Set camera position
setPawnPos('camera', SrVec(0, 50, 0))

# Turn on GPU deformable geomtery for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Add Physics script
scene.run('Reach.py')
scene.run('Gaze.py')

area1PickUp = False
area1PutDown = False
area2PickUp = False
area2PutDown = False
moving = False

# Updates to repeat reaches
last = 0
canTime = True
delay = 5

lastDoctor = 0
canTimeDoctor = True
delayDoctor = 1
class ReachDemo(SBScript):
	def update(self, time):
		global canTime, last
		if canTime:
			last = time
			canTime = False
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		
		# If time up, do actions
		if canTime:
			# Elder actions
			point('elder', 'elderPawn')
			gaze('elder', 'elderPawn')
			# Brad actions
			touch('brad', 'bradPawn')
			gaze('brad', 'bradPawn')
			# Utah actions
			checkUtah()
			
		global lastDoctor, canTimeDoctor
		if canTimeDoctor:
			lastDoctor = time
			canTimeDoctor = False
		diffDoctor = time - lastDoctor
		if diffDoctor >= delayDoctor:
			diffDoctor = 0
			canTimeDoctor = True
		if canTimeDoctor:
			checkDoctor()
			#<gesture name="a" start="6"/><locomotion target="0 0"/>
			
def checkUtah():
	global moving
	if not area1PickUp and not moving:
		pickUp('utah', 'utahPawn', True)
		gaze('utah', 'utahPawn')
		moving = True
	if not area1PutDown and area1PickUp and not moving:
		putDown('utah', '135 200 200', True)
		moving = True
	if not area2PickUp and area1PutDown and not moving:
		pickUp('utah', 'utahPawn', True)
		gaze('utah', 'utahPawn')
		moving = True
	if not area2PutDown and area2PickUp and not moving:
		putDown('utah', '135 200 -200', True)
		moving = True
		
currentTurn = 'doctor1'
pawnAttached = False
reaching = False
puttingDown = False
def checkDoctor():
	global currentTurn, pawnAttached
	if not pawnAttached and not reaching:
		pickUp(currentTurn, 'doctorPawn', finish=False)
	elif pawnAttached and not puttingDown:
		randX = random.uniform(-105, -65)
		randY = random.randrange(100, 200)
		randZ = random.randrange(80, 120)
		putDown(currentTurn, '%s %s %s' % (randX, randY, randZ))
	gaze('doctor1', 'doctorPawn')
	gaze('doctor2', 'doctorPawn')
		
# Run the update script
scene.removeScript('reachdemo')
reachdemo = ReachDemo()
scene.addScript('reachdemo', reachdemo)

class ReachingHandler(EventHandler):
	def executeAction(self, ev):
		params = ev.getParameters()
		global area1PickUp, area1PutDown, area2PickUp, area2PutDown, moving
		if 'utah' in params:
			if not area1PickUp and 'pawn-attached' in params:
				area1PickUp = True
				moving = False
			# If area 1 done
			elif area1PutDown and not area2PickUp and 'pawn-attached' in params:
				area2PickUp = True
				moving = False
			if not area1PutDown and 'pawn-released' in params:
				area1PutDown = True
				moving = False
			elif area2PickUp and not area2PutDown and 'pawn-released' in params:
				area2PutDown = True
				# Reset all
				area1PickUp = False
				area1PutDown = False
				area2PickUp = False
				area2PutDown = False
				moving = False
		
		global pawnAttached, currentTurn, reaching, puttingDown
		# Check which doctor's turn it is
		if currentTurn in params:
			if 'pawn-attached' in params:
				pawnAttached = True
				reaching = False
			if 'pawn-released' in params:
				pawnAttached = False
				if currentTurn == 'doctor1':
					currentTurn = 'doctor2'
				elif currentTurn == 'doctor2':
					currentTurn = 'doctor1'
				puttingDown = False
				
		reachingHdl = ReachingHandler()
		
evtMgr = scene.getEventManager()
reachingHdl = ReachingHandler()
evtMgr.addEventHandler('reachNotifier', reachingHdl)