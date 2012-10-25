import time
print "|--------------------------------------------|"
print "|         Starting Locomotion Demo           |"
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
camera.setEye(0, 1315.49, 1673.26)
camera.setCenter(0, 1189.21, 1519.14)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(10000)
camera.setNearPlane(1)
camera.setAspectRatio(1.02632)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad')
setPos('brad', SrVec(-35, 102, 0))
addCharacter('elder', 'elder')
setPos('elder', SrVec(-135, 102, 0))

# Add pawns
addPawn('pawn0', 'sphere')
scene.getPawn('pawn0').setPosition(SrVec(800, 10, 800))
addPawn('pawn1', 'box')
scene.getPawn('pawn1').setPosition(SrVec(-800, 10, -800))

# Add Locomotion script
scene.run('Locomotion.py')

# Whether character has reached its target
bradReached = True
elderReached = True

# Point A and B and current waypoint for elder
elderWaypoints = [SrVec(-500, -500, 0), SrVec(-500, 500, 0)]
cur = 0

# Mixed vector list for pathfinding function
vecList = [scene.getPawn('pawn1'), SrVec(-500, 0, 0), scene.getPawn('pawn0'), SrVec(1000, -500, 0), SrVec(-700, -700, 0), SrVec(0, 500, 0)]

class Locomotion(SBScript):
	def update(self, time):
		global bradReached, elderReached, cur
		if bradReached:
			followPath('brad', vecList)
			bradReached = False
		if elderReached:
			move('elder', elderWaypoints[cur])
			if cur == 0: cur = 1
			elif cur == 1: cur = 0
			elderReached = False
			
# Run the update script
scene.removeScript('locomotion')
locomotion = Locomotion()
scene.addScript('locomotion', locomotion)

class LocomotionHandler(EventHandler):
	def executeAction(self, ev):
		global bradReached, elderReached
		params = ev.getParameters()
		if 'success' in params:
			if 'brad' in params:
				bradReached = True
			if 'elder' in params:
				elderReached = True

locomotionHdl = LocomotionHandler()
evtMgr = scene.getEventManager()
evtMgr.addEventHandler('locomotion', locomotionHdl)