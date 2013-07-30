print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/bradrachel.py  |"
print "|--------------------------------------------|"
def createRetargetInstance(srcSkelName, tgtSkelName):
	endJoints = StringVec();
	#endJoints.append('l_ankle')
	endJoints.append('l_forefoot')
	endJoints.append('l_toe')
	endJoints.append('l_wrist')
	#endJoints.append('r_ankle')		
	endJoints.append('r_forefoot')	
	endJoints.append('r_toe')	
	endJoints.append('r_wrist')

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
	# replace retarget each animation with just a simple retarget instance
	retargetManager = scene.getRetargetManager()
        retarget = retargetManager.getRetarget(srcSkelName,tgtSkelName)
	if retarget == None:
		retarget = 	retargetManager.createRetarget(srcSkelName,tgtSkelName)
		retarget.initRetarget(endJoints,relativeJoints)
		

### Load data/sbm-common assets
scene.setMediaPath("../../../../data/")
scene.addAssetPath("motion", "ChrBrad")
scene.addAssetPath("script", "scripts")
scene.addAssetPath("script", "behaviorsets")
scene.addAssetPath("mesh", "mesh")
scene.addAssetPath("audio", ".")
scene.setScale(1.0)
scene.setBoolAttribute("internalAudio", True)

# map to the SmartBody standard
scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
scene.addAssetPath("motion", "ChrBrad")
scene.loadAssets()

scene.command("skeletonmap ChrBrad.sk zebra2")
scene.command("motionmapdir ChrBrad zebra2")

scene.setScale(1.0)
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 1.71, 1.86)
camera.setCenter(0, 1, 0.01)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
cameraPos = SrVec(0, 1.6, 10)
scene.getPawn('camera').setPosition(cameraPos)

# Brad's face definition
brad = scene.createCharacter("ChrBrad", "")
bradSkeleton = scene.createSkeleton("ChrBrad.sk")
brad.setSkeleton(bradSkeleton)
#brad.setFaceDefinition(bradFace)
bradPos = SrVec(.35, 0, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(-17, 0, 0)
brad.setHPR(bradHPR)

#print 'Before create controllers'
brad.createStandardControllers()
#print 'After create controllers'

# deformable mesh
brad.setDoubleAttribute("deformableMeshScale", .01)
brad.setStringAttribute("deformableMesh", "ChrBrad")
# lip syncing diphone setup
brad.setStringAttribute("lipSyncSetName", "default")
brad.setBoolAttribute("usePhoneBigram", True)
brad.setVoice("remote")
brad.setVoiceCode("Festival_voice_kal_diphone")
# gesture map setup
brad.setStringAttribute("gestureMap", "ChrBrad")
brad.setBoolAttribute("gestureRequest.autoGestureTransition", True)
brad.setBoolAttribute("ikPostFix", True)
brad.setBoolAttribute("terrainWalk", True)
scene.command("char ChrBrad viewer deformableGPU")
scene.setDefaultCharacter("ChrBrad")

# setup mocap locomotion
scene.run('BehaviorSetMaleMocapLocomotion.py')
setupBehaviorSet()
retargetBehaviorSet('ChrBrad')

# steering
scene.run("init-steer-agents.py")
setupSteerAgent("ChrBrad", "mocap")
steerManager = scene.getSteerManager()
steerManager.setEnable(False)
scene.command('terrain load')
# start the simulation
sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBML('ChrBrad', '<saccade mode="listen"/>')
sim.resume()

steerManager.setEnable(True)





