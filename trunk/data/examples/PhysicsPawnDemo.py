import random

print "|--------------------------------------------|"
print "|        Starting Physics Pawn Demo          |"
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
camera.setEye(0, 409.62, 733.74)
camera.setCenter(0, 335.62, 548.74)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(10000)
camera.setNearPlane(1)
camera.setAspectRatio(1.02632)

#scene.getSimulationManager().setSimFps(60)

# Add Character script
scene.run('AddCharacter.py')
numPawns = 50
for i in range(numPawns):
	baseName = 'phy%s' % i
	shapeList = ['sphere', 'box', 'capsule']
	size = random.randrange(5, 30)
	addPawn(baseName, random.choice(shapeList), SrVec(size, size, size))

pawnList = []
for name in scene.getPawnNames():
	if 'phy' in name:
		pawnList.append(scene.getPawn(name))

# Add Physics script
scene.run('Physics.py')

# Setup pawn physics
for pawn in pawnList:
	setupPawnPhysics(pawn.getName())

last = 0
canTime = True
delay = 3
started = False
class PhysicsPawnDemo(SBScript):
	def update(self, time):
		global canTime, last, started
		if canTime:
			last = time
			canTime = False
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		if canTime and not started:
			#for pawn in pawnList:
				#togglePawnPhysics(pawn.getName())
			started = True
		if canTime:
			for pawn in pawnList:
				togglePawnPhysics(pawn.getName())
			randomizePos()

size = 150
def randomizePos():
	for pawn in pawnList:
		x = random.uniform(-size, size) + 1
		y = random.uniform(50, size * 3) + 1
		z = random.uniform(-size, size) + 1
		h = random.uniform(-180, 180) + 1
		p = random.uniform(-180, 180) + 1
		r = random.uniform(-180, 180) + 1
		pawn.setPosition(SrVec(x, y, z))
		pawn.setHPR(SrVec(h, p, r))

randomizePos()
			
# Run the update script
scene.removeScript('physicspawndemo')
physicspawndemo = PhysicsPawnDemo()
scene.addScript('physicspawndemo', physicspawndemo)