import math
import random
print "|--------------------------------------------|"
print "|         Starting Ogre Crowd Demo           |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'mesh/Ogre')
scene.addAssetPath('script', 'behaviorsets')
scene.addAssetPath('script', 'scripts')
scene.loadAssets()

# Set scene parameters and camera
scene.setScale(0.1)
#scene.getPawn('camera').setPosition(SrVec(0, -5, 0))

# Set joint map for Sinbad
print 'Setting up joint map for Brad'
scene.run('ogre-sinbad-map.py')
sinbadSkName = 'Sinbad.skeleton.xml'
jointMapManager = scene.getJointMapManager()
sinbadMap = jointMapManager.getJointMap('Sinbad.skeleton.xml')
ogreSk = scene.getSkeleton(sinbadSkName)
sinbadMap.applySkeleton(ogreSk)

steerManager = scene.getSteerManager()
print 'Setting up Sinbads'
amount = 20
row = 0; column = 0; 
offsetX = 0; offsetZ = 0;
sinbadList = []
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
	# Retarget character
	if i == 0 :
		scene.run('BehaviorSetMaleLocomotion.py')
		setupBehaviorSet()
	retargetBehaviorSet(sinbadName)	
	sinbadList.append(sinbad)
	scene.command("char %s viewer deformableGPU" % sinbadName)	
	# Play default animation
	bml.execBML(sinbadName, '<body posture="ChrUtah_Idle001"/>')
	
steerManager.setEnable(False)
steerManager.setEnable(True)

# Set up list of Brads

print 'Configuring scene parameters and camera'
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 80.98, 53.44)
camera.setCenter(1.0, 1.7, -39.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(500)
camera.setNearPlane(0.1)
camera.setAspectRatio(1.02)

sim.start()
sim.resume()

# Paths for characters
sinbadPath = [SrVec(-120, -120, 0), SrVec(120, 120, 0), SrVec(120, -120, 0), SrVec(-120, 120, 0)]
sinbadCur = 0
pathAmt = len(sinbadPath)
sinbadReached = True

class CrowdDemo(SBScript):
	def update(self, time):
		global sinbadReached, sinbadCur
		# Once utah completes path, do again
		if sinbadReached:
			for sinbad in sinbadList:
				# Move character
				bml.execBML(sinbad.getName(), '<locomotion speed="' +  str(random.uniform(1.2, 5.0)) + '" target="' +\
											vec2str(sinbadPath[sinbadCur]) + '" type="basic"/>')
			sinbadCur = sinbadCur + 1
			# If reaches max path, reset
			if sinbadCur >= pathAmt:
				sinbadCur = 0
			sinbadReached = False
		
# Run the update script
scene.removeScript('crowddemo')
crowddemo = CrowdDemo()
scene.addScript('crowddemo', crowddemo)

reachCount = 0
# Locomotion handler to check if characters have arrived
class LocomotionHandler(SBEventHandler):
	def executeAction(self, ev):
		global sinbadReached, reachCount
		params = ev.getParameters()
		if 'success' in params:
			if 'sinbad' in params:
				reachCount = reachCount + 1
				if reachCount >= 6:
					sinbadReached = True	
					reachCount = 0

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
