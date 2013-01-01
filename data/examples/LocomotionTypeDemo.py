import random
print "|--------------------------------------------|"
print "|       Starting Locomotion Type Demo        |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath('script', 'sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('mesh', 'retarget/mesh')
scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'ChrRachel')
scene.addAssetPath('motion', 'retarget\motion')
scene.addAssetPath('motion', 'sbm-common/common-sk')
scene.loadAssets()

# Set scene parameters and camera
print 'Configuring scene parameters and camera'
scene.setScale(1.0)
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 2.87, 11.67)
camera.setCenter(0, 2.14, 9.81)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, -5, 0))

# Set joint map for Brad
print 'Setting up joint map for Brad'
scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse('ChrBrad')

# Retarget setup
scene.run('motion-retarget.py')
# Animation setup
scene.run('init-param-animation.py')

# Set up 3 Brads
print 'Adding characters into scene'
posX = -200
for i in range(3):
	baseName = 'ChrBrad%s' % i
	brad = scene.createCharacter(baseName, '')
	bradSkeleton = scene.createSkeleton('ChrBrad.sk')
	brad.setSkeleton(bradSkeleton)
	# Set position
	bradPos = SrVec((posX + (i * 200))/100, 0, 0)
	brad.setPosition(bradPos)
	# Set up standard controllers
	brad.createStandardControllers()
	# Set deformable mesh
	brad.setDoubleAttribute('deformableMeshScale', .01)
	brad.setStringAttribute('deformableMesh', 'ChrBrad')
	# Play idle animation
	bml.execBML(baseName, '<body posture="ChrBrad@Idle01"/>')
	# Retarget character
	retargetCharacter(baseName, 'ChrBrad.sk', False)

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Whether character has reached its target
brad1Reached = brad2Reached = brad3Reached = True

# Paths for characters
brad1Path = [SrVec(-2, 8, 0), SrVec(-2, -8, 0)]
brad2Path = [SrVec(0, 8, 0), SrVec(0, -8, 0)]
brad3Path = [SrVec(2, 8, 0), SrVec(2, -8, 0)]

brad1Cur = brad2Cur = brad3Cur = 0
pathAmt = 2

class LocomotionDemo(SBScript):
	def update(self, time):
		global brad1Reached, brad2Reached, brad3Reached, brad1Cur, brad2Cur, brad3Cur
		# When brad has reached, make him move to the next location
		if brad1Reached:
			bml.execBML('ChrBrad0', '<locomotion manner="walk" target="' + vec2str(brad1Path[brad1Cur]) + '" type="basic"/>')
			brad1Cur = brad1Cur + 1
			if brad1Cur >= pathAmt:
				brad1Cur = 0
			brad1Reached = False
		if brad2Reached:
			bml.execBML('ChrBrad1', '<locomotion manner="jog" target="' + vec2str(brad2Path[brad2Cur]) + '" type="basic"/>')
			brad2Cur = brad2Cur + 1
			if brad2Cur >= pathAmt:
				brad2Cur = 0
			brad2Reached = False
		if brad3Reached:
			bml.execBML('ChrBrad2', '<locomotion manner="run" target="' + vec2str(brad3Path[brad3Cur]) + '" type="basic"/>')
			brad3Cur = brad3Cur + 1
			if brad3Cur >= pathAmt:
				brad3Cur = 0
			brad3Reached = False
			
# Run the update script
scene.removeScript('locomotiondemo')
locomotiondemo = LocomotionDemo()
scene.addScript('locomotiondemo', locomotiondemo)

# Locomotion handler to check if characters have arrived
class LocomotionHandler(EventHandler):
	def executeAction(self, ev):
		global brad1Reached, brad2Reached, brad3Reached
		params = ev.getParameters()
		if 'success' in params:
			if 'ChrBrad0' in params:
				brad1Reached = True
			if 'ChrBrad1' in params:
				brad2Reached = True
			if 'ChrBrad2' in params:
				brad3Reached = True

locomotionHdl = LocomotionHandler()
evtMgr = scene.getEventManager()
evtMgr.addEventHandler('locomotion', locomotionHdl)

def vec2str(vec):
	''' Converts SrVec to string '''
	x = vec.getData(0)
	y = vec.getData(1)
	z = vec.getData(2)
	if -0.0001 < x < 0.0001: x = 0
	if -0.0001 < y < 0.0001: y = 0
	if -0.0001 < z < 0.0001: z = 0
	return "" + str(x) + " " + str(y) + " " + str(z) + ""