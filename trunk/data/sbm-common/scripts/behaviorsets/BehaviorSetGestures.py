scene.run("BehaviorSetCommon.py")

def setupBehaviorSet():
	print "Setting up behavior set for gestures..."
	scene.loadAssetsFromPath("../../../../data/behaviorsets/Gestures/skeletons")
	scene.loadAssetsFromPath("../../../../data/behaviorsets/Gestures/motions")
	scene.addAssetPath("script", "../../../../data/behaviorsets/Gestures/scripts")
	# map the zebra2 skeleton
	scene.run("zebra2-map.py")
	zebra2Map = scene.getJointMapManager().getJointMap("zebra2")
	bradSkeleton = scene.getSkeleton("ChrBrad.sk")
	zebra2Map.applySkeleton(bradSkeleton)
	
	gestureMotions = StringVec()
	gestureMotions.append("ChrBrad@Idle01")
	gestureMotions.append("ChrBrad@Idle01_ArmStretch01")
	gestureMotions.append("ChrBrad@Idle01_BeatHighBt01")
	gestureMotions.append("ChrBrad@Idle01_BeatHighLf01")
	gestureMotions.append("ChrBrad@Idle01_BeatLowBt01")
	gestureMotions.append("ChrBrad@Idle01_BeatLowLf01")
	gestureMotions.append("ChrBrad@Idle01_BeatLowLf02")
	gestureMotions.append("ChrBrad@Idle01_BeatLowRt01")
	gestureMotions.append("ChrBrad@Idle01_BeatMidBt01")
	gestureMotions.append("ChrBrad@Idle01_BeatMidLf01")	
	gestureMotions.append("ChrBrad@Idle01_ChopBoth01")
	gestureMotions.append("ChrBrad@Idle01_ChopLf01")
	gestureMotions.append("ChrBrad@Idle01_Contemplate01")
	gestureMotions.append("ChrBrad@Idle01_ExampleLf01")
	gestureMotions.append("ChrBrad@Idle01_HoweverLf01")
	gestureMotions.append("ChrBrad@Idle01_IndicateLeftBt01")
	gestureMotions.append("ChrBrad@Idle01_IndicateLeftLf01")
	gestureMotions.append("ChrBrad@Idle01_IndicateRightBt01")
	gestureMotions.append("ChrBrad@Idle01_IndicateRightRt01")
	gestureMotions.append("ChrBrad@Idle01_MeLf01")
	gestureMotions.append("ChrBrad@Idle01_NegativeBt01")
	gestureMotions.append("ChrBrad@Idle01_NegativeLf01")
	gestureMotions.append("ChrBrad@Idle01_OfferBoth01")
	gestureMotions.append("ChrBrad@Idle01_OfferLf01")
	gestureMotions.append("ChrBrad@Idle01_PleaBt02")
	gestureMotions.append("ChrBrad@Idle01_PointLf01")
	gestureMotions.append("ChrBrad@Idle01_ReceiveLf01")
	gestureMotions.append("ChrBrad@Idle01_SafeRt01")
	gestureMotions.append("ChrBrad@Idle01_ScratchChest01")
	gestureMotions.append("ChrBrad@Idle01_ScratchHeadLf01")
	gestureMotions.append("ChrBrad@Idle01_ScratchTempleLf01")
	gestureMotions.append("ChrBrad@Idle01_ShoulderStretch01")
	gestureMotions.append("ChrBrad@Idle01_Shrug01")
	gestureMotions.append("ChrBrad@Idle01_SqueezeHand01")
	gestureMotions.append("ChrBrad@Idle01_Think01")
	gestureMotions.append("ChrBrad@Idle01_WeightShift01")
	gestureMotions.append("ChrBrad@Idle01_WeightShift02")
	gestureMotions.append("ChrBrad@Idle01_YouLf01")
	
	for i in range(0, len(gestureMotions)):
		motion = scene.getMotion(gestureMotions[i])
		zebra2Map.applyMotion(motion)


def retargetBehaviorSet(charName, skelName):
	gestureMotions = StringVec()
	gestureMotions.append("ChrBrad@Idle01")
	gestureMotions.append("ChrBrad@Idle01_ArmStretch01")
	gestureMotions.append("ChrBrad@Idle01_BeatHighBt01")
	gestureMotions.append("ChrBrad@Idle01_BeatHighLf01")
	gestureMotions.append("ChrBrad@Idle01_BeatLowBt01")
	gestureMotions.append("ChrBrad@Idle01_BeatLowLf01")
	gestureMotions.append("ChrBrad@Idle01_BeatLowLf02")
	gestureMotions.append("ChrBrad@Idle01_BeatLowRt01")
	gestureMotions.append("ChrBrad@Idle01_BeatMidBt01")
	gestureMotions.append("ChrBrad@Idle01_BeatMidLf01")	
	gestureMotions.append("ChrBrad@Idle01_ChopBoth01")
	gestureMotions.append("ChrBrad@Idle01_ChopLf01")
	gestureMotions.append("ChrBrad@Idle01_Contemplate01")
	gestureMotions.append("ChrBrad@Idle01_ExampleLf01")
	gestureMotions.append("ChrBrad@Idle01_HoweverLf01")
	gestureMotions.append("ChrBrad@Idle01_IndicateLeftBt01")
	gestureMotions.append("ChrBrad@Idle01_IndicateLeftLf01")
	gestureMotions.append("ChrBrad@Idle01_IndicateRightBt01")
	gestureMotions.append("ChrBrad@Idle01_IndicateRightRt01")
	gestureMotions.append("ChrBrad@Idle01_MeLf01")
	gestureMotions.append("ChrBrad@Idle01_NegativeBt01")
	gestureMotions.append("ChrBrad@Idle01_NegativeLf01")
	gestureMotions.append("ChrBrad@Idle01_OfferBoth01")
	gestureMotions.append("ChrBrad@Idle01_OfferLf01")
	gestureMotions.append("ChrBrad@Idle01_PleaBt02")
	gestureMotions.append("ChrBrad@Idle01_PointLf01")
	gestureMotions.append("ChrBrad@Idle01_ReceiveLf01")
	gestureMotions.append("ChrBrad@Idle01_SafeRt01")
	gestureMotions.append("ChrBrad@Idle01_ScratchChest01")
	gestureMotions.append("ChrBrad@Idle01_ScratchHeadLf01")
	gestureMotions.append("ChrBrad@Idle01_ScratchTempleLf01")
	gestureMotions.append("ChrBrad@Idle01_ShoulderStretch01")
	gestureMotions.append("ChrBrad@Idle01_Shrug01")
	gestureMotions.append("ChrBrad@Idle01_SqueezeHand01")
	gestureMotions.append("ChrBrad@Idle01_Think01")
	gestureMotions.append("ChrBrad@Idle01_WeightShift01")
	gestureMotions.append("ChrBrad@Idle01_WeightShift02")
	gestureMotions.append("ChrBrad@Idle01_YouLf01")
	
	outDir = '../../../../data/retarget/motion/' + skelName + '/';
	print 'outDir = ' + outDir ;
	if not os.path.exists(outDir):
		os.makedirs(outDir)

	# retarget gestures
	for n in range(0, len(gestureMotions)):
		curMotion = scene.getMotion(gestureMotions[n])
		if curMotion is not None:
			retargetMotion(gestureMotions[n], 'ChrBrad.sk', skelName, outDir + 'Gestures/');
		else:
			print "Cannot find motion " + gestureMotions[n] + ", it will be excluded from the gesture setup..."

		
