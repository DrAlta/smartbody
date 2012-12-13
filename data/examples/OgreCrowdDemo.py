import math
import random
print "|--------------------------------------------|"
print "|         Starting Ogre Crowd Demo           |"
print "|--------------------------------------------|"

# Add asset paths
scene.setMediaPath('../../../../data')
scene.addAssetPath('script', 'sbm-common/scripts')
scene.addAssetPath('script', 'sbm-common/scripts/behaviorsets')
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('mesh', 'retarget/mesh')
scene.addAssetPath('motion', 'Ogre')
scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'retarget/motion')
scene.addAssetPath('motion', 'sbm-common/common-sk')
scene.loadAssets()

# Set scene parameters and camera
scene.setScale(0.05)
#scene.getPawn('camera').setPosition(SrVec(0, -5, 0))

# Set joint map for Sinbad
print 'Setting up joint map for Brad'
scene.run('ogre-sinbad-map.py')
sinbadSkName = 'Sinbad.skeleton.xml'
jointMapManager = scene.getJointMapManager()
sinbadMap = jointMapManager.getJointMap('Sinbad.skeleton.xml')
ogreSk = scene.getSkeleton(sinbadSkName)
sinbadMap.applySkeleton(ogreSk)

# Behavior set setup
scene.run('behaviorsetup.py')

# Animation setup
scene.run('init-param-animation.py')
steerManager = scene.getSteerManager()

# Setting up Sinbad
# print 'Setting up Sinbad'
# sinbadName = 'sinbad'
# sinbad = scene.createCharacter(sinbadName,'')
# sinbadSk = scene.createSkeleton(sinbadSkName)
# sinbad.setSkeleton(sinbadSk)
# sinbadPos = SrVec(0,5.16, 0)
# sinbad.setPosition(sinbadPos)
# sinbad.createStandardControllers()
# sinbad.setStringAttribute('deformableMesh', 'Sinbad')
# scene.run('BehaviorSetMaleLocomotion.py')
# setupBehaviorSet()
# retargetBehaviorSet(sinbadName,sinbadSkName)
# scene.command('char sinbad viewer deformableGPU')

print 'Setting up Sinbads'
amount = 20
row = 0; column = 0; 
offsetX = 0; offsetZ = 0;
for i in range(amount):
	sinbadName = 'sinbad%s' % i
	sinbad = scene.createCharacter(sinbadName,'')
	sinbadSk = scene.createSkeleton(sinbadSkName)
	sinbad.setSkeleton(sinbadSk)
	# Set position logic
	posX = (-10 * (5/2)) + 10 * column
	posZ = ((-10 / math.sqrt(amount)) * (amount/2)) + 10 * row	
	column = column + 1
	if column >= 5:
		column = 0
		row = row + 1
	sinbadPos = SrVec((posX + offsetX), 5.16, (posZ + offsetZ))
	sinbad.setPosition(sinbadPos)
	sinbad.createStandardControllers()
	sinbad.setStringAttribute('deformableMesh', 'Sinbad')
	scene.run('BehaviorSetMaleLocomotion.py')
	setupBehaviorSet()
	retargetBehaviorSet(sinbadName,sinbadSkName)	
	scene.command("char %s viewer deformableGPU" % sinbadName)	
	# Play default animation
	bml.execBML(sinbadName, '<body posture="Sinbad.skeleton.xmlChrUtah_Idle001"/>')
	
steerManager.setEnable(False)
steerManager.setEnable(True)


print 'Configuring scene parameters and camera'
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 5.98, 13.44)
camera.setCenter(1.0, 1.7, -39.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(1.02)

sim.start()
sim.resume()
'''
steeringGroup = []
pathfindingGroup = []
# Assign groups
print 'Assigning Brads in groups'
for name in scene.getCharacterNames():
	if 'ChrBrad' in name:
		if len(steeringGroup) < amount/2:
			steeringGroup.append(scene.getCharacter(name))
		else:
			# Set pathfinding on
			scene.getCharacter(name).setBoolAttribute('steering.pathFollowingMode', True)
			pathfindingGroup.append(scene.getCharacter(name))

# Adding pawns to scene
print 'Adding pawns to scene'
target0 = scene.createPawn('target0')
target0.setPosition(SrVec(-10, 0, -10))
target1 = scene.createPawn('target1')
target1.setPosition(SrVec(-4, 0, 10))
			
group1Reached = True
group2Reached = True

# Update to repeat paths
last = 0
canTime = True
delay = 30
class LocomotionDemo(SBScript):
	def update(self, time):
		global group1Reached, group2Reached, canTime, last
		if canTime:
			last = time
			canTime = False
			group1Reached = group2Reached = True
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		# Once group 1 completes path, do again
		if group1Reached:
			for brad in steeringGroup:
				bml.execBML(brad.getName(), '<locomotion manner="run" target="-10 10 -4 -10 target1 target0"/>')
			group1Reached = False
		# Once group 2 completes path, do again
		if group2Reached:
			for brad in pathfindingGroup:
				bml.execBML(brad.getName(), '<locomotion manner="run" target="10 10 4 -10 4 10 10 -10"/>')
			group2Reached = False
			
# Run the update script
scene.removeScript('locomotiondemo')
locomotiondemo = LocomotionDemo()
scene.addScript('locomotiondemo', locomotiondemo)
'''