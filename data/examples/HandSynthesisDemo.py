print "|--------------------------------------------|"
print "|        Setup Hand Demo 			        |"
print "|--------------------------------------------|"

		
def setMotionNameSkeleton(motionName, skelName):
	motion = scene.getMotion(motionName)
	if motion != None:		
		#motion.scale(1)
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
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'ChrMaarten')
scene.addAssetPath('motion', 'Hand')
scene.addAssetPath('script', 'behaviorsets')
scene.addAssetPath('script', 'scripts')
scene.loadAssets()

# Runs the default viewer for camera
scene.setScale(1)

# Apply hand joint map
# Set up joint map for Bradh  
print 'Setting up joint map and configuring Brad\'s skeleton'
scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse('ChrBrad')

# setting up ipy retargetting
print 'Setting up joint map and configuring Brad\'s skeleton'
scene.run('ipisoft-map.py')
ipisoftMap = scene.getJointMapManager().getJointMap('ipisoft')


# retargetting the motion (iPy motion to smartbody skeleton)
m = scene.getMotion("Adil_Gesture1.bvh")
ipisoftMap.applyMotion(m)
ipiSk = scene.getSkeleton("Adil_Gesture1.bvh")
ipisoftMap.applySkeleton(ipiSk)
setMotionNameSkeleton('Adil_Gesture1.bvh', 'Adil_Gesture1.bvh')
createRetargetInstance('Adil_Gesture1.bvh','ChrBrad.sk')


# run behavior set to load motions
scene.run('BehaviorSetGestures.py')
setupBehaviorSet()
retargetBehaviorSet('ChrBrad')

# create a new hand configuration
configManager = scene.getHandConfigurationManager()
handConfig = configManager.createHandConfiguration("test_config")
handConfig.addMotion("ChrBrad@Idle01_NegativeBt01")
#configManager.printHandConfiguration("test_config")

#create another hand configuration
handConfig2 = configManager.createHandConfiguration("new_config")
handConfig2.addMotion("ChrBrad@Idle01_YouPointLf01")
handConfig2.addMotion("ChrBrad@Idle01_BeatHighBt01")
handConfig2.addMotion("ChrBrad@Idle01_IndicateRightBt01")
#configManager.printHandConfiguration("new_config")

# create a third detailed hand configuration
print 'Setting up all_config'
handConfig3 = configManager.createHandConfiguration("all_config")
handConfig3.addMotion("ChrBrad@Idle01_YouPointLf01")
handConfig3.addMotion("ChrBrad@Idle01_BeatHighBt01")
handConfig3.addMotion("ChrBrad@Idle01_ArmStretch01")
handConfig3.addMotion("ChrBrad@Idle01_BeatHighBt02")
handConfig3.addMotion("ChrBrad@Idle01_BeatHighLf01")
handConfig3.addMotion("ChrBrad@Idle01_BeatLowBt01")
handConfig3.addMotion("ChrBrad@Idle01_BeatLowLf01")
handConfig3.addMotion("ChrBrad@Idle01_BeatLowLf02")
handConfig3.addMotion("ChrBrad@Idle01_BeatMidBt01")
handConfig3.addMotion("ChrBrad@Idle01_BeatMidLf01")
handConfig3.addMotion("ChrBrad@Idle01_ChopBoth01")
handConfig3.addMotion("ChrBrad@Idle01_ChopBt01")
handConfig3.addMotion("ChrBrad@Idle01_ChopLf01")
handConfig3.addMotion("ChrBrad@Idle01_Contemplate01")
handConfig3.addMotion("ChrBrad@Idle01_ExampleLf01")
handConfig3.addMotion("ChrBrad@Idle01_HoweverLf01")
handConfig3.addMotion("ChrBrad@Idle01_IndicateLeftBt01")
handConfig3.addMotion("ChrBrad@Idle01_IndicateLeftLf01")
handConfig3.addMotion("ChrBrad@Idle01_IndicateRightBt01")
handConfig3.addMotion("ChrBrad@Idle01_IndicateRightRt01")
handConfig3.addMotion("ChrBrad@Idle01_MeLf01")
handConfig3.addMotion("ChrBrad@Idle01_NegativeBt01")
handConfig3.addMotion("ChrBrad@Idle01_NegativeBt02")
handConfig3.addMotion("ChrBrad@Idle01_NegativeLf01")
handConfig3.addMotion("ChrBrad@Idle01_NegativeRt01")
handConfig3.addMotion("ChrBrad@Idle01_OfferBoth01")
handConfig3.addMotion("ChrBrad@Idle01_OfferLf01")
handConfig3.addMotion("ChrBrad@Idle01_PleaBt02")
handConfig3.addMotion("ChrBrad@Idle01_PointLf01")
handConfig3.addMotion("ChrBrad@Idle01_ReceiveLf01")
handConfig3.addMotion("ChrBrad@Idle01_ReceiveRt01")
handConfig3.addMotion("ChrBrad@Idle01_SafeLf01")
handConfig3.addMotion("ChrBrad@Idle01_SafeRt01")
handConfig3.addMotion("ChrBrad@Idle01_ScratchChest01")
handConfig3.addMotion("ChrBrad@Idle01_ScratchHeadLf01")
handConfig3.addMotion("ChrBrad@Idle01_YouLf01")

# use this to print hand configurations
#configManager.printHandConfiguration("all_config")

# Add ChrBrad (source character )
source = scene.createCharacter('source', '')
sourceSkeleton = scene.createSkeleton('ChrBrad.sk')
source.setSkeleton(sourceSkeleton)
sourcePos = SrVec(0, 0, 0)
source.setPosition(sourcePos)
source.createStandardControllers()

# Deformable mesh
source.setDoubleAttribute('deformableMeshScale', 0.01)

# enable these for mesh
#source.setStringAttribute('deformableMesh', 'ChrMaarten.dae')
#source.setStringAttribute("displayType", "GPUmesh")

# configure camera
print 'Configuring scene parameters and camera'
camera = getCamera()
camera.setEye(0, 7.98, 17.44)
camera.setCenter(1.0, 1.7, -39.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(1000)
camera.setNearPlane(0.1)
camera.setAspectRatio(1.02)