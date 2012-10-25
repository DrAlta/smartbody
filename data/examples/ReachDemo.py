import time

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
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(10000)
camera.setNearPlane(1)
camera.setAspectRatio(1.02632)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad', True)
setPos('brad', SrVec(-35, 102, 0))
addCharacter('elder', 'elder', True)
setPos('elder', SrVec(-135, 102, 0))
addCharacter('utah', 'utah', True)
setPos('utah', SrVec(135, 0, 0))

# Add pawns in scene
addPawn('bradPawn', 'sphere')
scene.getPawn('bradPawn').setPosition(SrVec(-70, 40, -30))
addPawn('elderPawn', 'sphere')
scene.getPawn('elderPawn').setPosition(SrVec(-175, 220, 65))
addPawn('utahPawn', 'sphere')
scene.getPawn('utahPawn').setPosition(SrVec(135, 200, -200))

#printVector(scene.getPawn('bradPawn').getPosition())

# Add Physics script
scene.run('Reach.py')
scene.run('Gaze.py')

# Update for non locomotion
# Locomotion eventhandler for locomotion ones

area1PickUp = False
area1PutDown = False
area2PickUp = False
area2PutDown = False
moving = False

# Updates to repeat reaches
last = time.time()
canTime = True
delay = 5
class Reaches(SBScript):
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
			self.checkUtah()
			
	def checkUtah(self):
		global moving
		if not area1PickUp and not moving:
			pickUp('utah', 'utahPawn', True)
			gaze('utah', 'utahPawn')
			moving = True
		if not area1PutDown and area1PickUp and not moving:
			putDown('utah', '135 200 200')
			#gaze('utah', 'utahPawn')
			moving = True
		if not area2PickUp and area1PutDown and not moving:
			pickUp('utah', 'utahPawn', True)
			gaze('utah', 'utahPawn')
			moving = True
		if not area2PutDown and area2PickUp and not moving:
			putDown('utah', '135 200 -200')
			#gaze('utah', 'utahPawn')
			moving = True
			
# Run the update script
scene.removeScript('reaches')
reaches = Reaches()
scene.addScript('reaches', reaches)

class ReachingHandler(EventHandler):
	def executeAction(self, ev):
		params = ev.getParameters()
		global area1PickUp, area1PutDown, area2PickUp, area2PutDown, moving
		if 'utah' in params:
			if not area1PickUp and 'pawn-attached' in params:
				area1PickUp = True
				moving = False
				#print 'picked up area 1'
			# If area 1 done
			elif area1PutDown and not area2PickUp and 'pawn-attached' in params:
				area2PickUp = True
				moving = False
				#print 'picked up area 2'
			if not area1PutDown and 'pawn-released' in params:
				area1PutDown = True
				moving = False
				#print 'put down area 1'
			elif area2PickUp and not area2PutDown and 'pawn-released' in params:
				area2PutDown = True
				#print 'put down area 2'
				# Reset all
				area1PickUp = False
				area1PutDown = False
				area2PickUp = False
				area2PutDown = False
				moving = False
		
		reachingHdl = ReachingHandler()
		
evtMgr = scene.getEventManager()
reachingHdl = ReachingHandler()
evtMgr.addEventHandler('reachNotifier', reachingHdl)