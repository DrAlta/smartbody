print "|--------------------------------------------|"
print "|        Starting Retargetting Demo          |"
print "|--------------------------------------------|"

		
def setMotionNameSkeleton(motionName, skelName):
	motion = scene.getMotion(motionName)
	if motion != None:
		motion.scale(100)
		motion.setMotionSkeletonName(skelName)
	
def createRetargetInstance(srcSkelName, tgtSkelName):
	# replace retarget each animation with just a simple retarget instance
	
	# these joints and their children are not retargeted
	endJoints = StringVec();
	endJoints.append('l_forefoot')
	endJoints.append('l_toe')
	endJoints.append('l_wrist')
	endJoints.append('r_forefoot')	
	endJoints.append('r_toe')	
	endJoints.append('r_wrist')

	# these joints are skipped during skeleton alignment
	relativeJoints = StringVec();
	relativeJoints.append('spine1')
	relativeJoints.append('spine2')
	relativeJoints.append('spine3')
	relativeJoints.append('spine4')
	relativeJoints.append('spine5')
	relativeJoints.append('r_sternoclavicular')
	relativeJoints.append('l_sternoclavicular')
	relativeJoints.append('r_acromioclavicular')
	relativeJoints.append('l_acromioclavicular')	
	
	retargetManager = scene.getRetargetManager()
        retarget = retargetManager.getRetarget(srcSkelName,tgtSkelName)
	if retarget == None:
		retarget = 	retargetManager.createRetarget(srcSkelName,tgtSkelName)
		retarget.initRetarget(endJoints,relativeJoints)
		
# Add asset paths
scene.addAssetPath('script', 'sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'sbm-common/common-sk')
scene.loadAssets()

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 188.56, 241.41)
camera.setCenter(0, 114.56, 56.41)
scene.getPawn('camera').setPosition(SrVec(0, -5, 0))

# Apply zebra joint map for ChrBrad
scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
bradSkeleton = scene.getSkeleton("ChrBrad.sk")
bradSkeleton.rescale(100)
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse("ChrBrad")

# set skeleton names for source motions, so the on-line retargeting will know how to apply retargeting
setMotionNameSkeleton("ChrBrad@Idle01", "ChrBrad.sk")
setMotionNameSkeleton("ChrBrad@Guitar01", "ChrBrad.sk")
setMotionNameSkeleton("ChrBrad@PushUps01", "ChrBrad.sk")

# Create on-line retarget instance from ChrBrad.sk to common.sk
createRetargetInstance("ChrBrad.sk", "common.sk")

# Add Brad ( target character )
target = scene.createCharacter('target', '')
targetSkeleton = scene.createSkeleton('common.sk')
target.setSkeleton(targetSkeleton)
targetPos = SrVec(-80, 102, 0)
target.setPosition(targetPos)
target.createStandardControllers()
target.setDoubleAttribute('deformableMeshScale', 1)
target.setStringAttribute('deformableMesh', 'brad')
bml.execBML('target', '<body posture="ChrBrad@Idle01"/>')

# Add ChrBrad (source character )
source = scene.createCharacter('source', '')
sourceSkeleton = scene.createSkeleton('ChrBrad.sk')
source.setSkeleton(sourceSkeleton)
sourcePos = SrVec(80, 0, 0)
source.setPosition(sourcePos)
source.createStandardControllers()
# Deformable mesh
source.setDoubleAttribute('deformableMeshScale', 1)
source.setStringAttribute('deformableMesh', 'ChrBrad')
bml.execBML('source', '<body posture="ChrBrad@Idle01"/>')

# Turn on GPU deformable geometry
scene.command("char target viewer deformableGPU")
scene.command("char source viewer deformableGPU")


# Retarget motion
	
last = 0
canTime = True
delay = 10
class RetargettingDemo(SBScript):
	def update(self, time):
		global canTime, last, output
		diff = time - last
		if diff >= delay:
			canTime = True
			diff = 0
		if canTime:
			last = time
			canTime = False
			# Play non retargetted and retargetted add delay
			bml.execBML('target', '<animation name="ChrBrad@Guitar01"/>')
			bml.execBML('source', '<animation name="ChrBrad@Guitar01"/>')
			
scene.removeScript('retargettingdemo')
retargettingdemo = RetargettingDemo()
scene.addScript('retargettingdemo', retargettingdemo)
