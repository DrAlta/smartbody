def catchingSetup(origSkelName, skeletonName, baseJoint, prefix, statePreFix):
	blendManager = scene.getBlendManager()

	blendCatching = blendManager.createMotionBlendBase(prefix + "Catch", skeletonName, 3)
	blendCatching.setBlendSkeleton(skeletonName)

	originalMotions = StringVec()
	originalMotions.append("Center.bvh")
	originalMotions.append("Center2.bvh")
	originalMotions.append("Far_Center_Up.bvh")
	originalMotions.append("Far_Left.bvh")
	originalMotions.append("Far_Left2.bvh")
	originalMotions.append("Far_Low_Left.bvh")
	originalMotions.append("Far_Lower_Right.bvh")
	originalMotions.append("Far_Lower_Right2.bvh")
	originalMotions.append("Far_Right.bvh")
	originalMotions.append("Left.bvh")
	originalMotions.append("Low_Center.bvh")
	originalMotions.append("Low_Left.bvh")
	originalMotions.append("Low_Right.bvh")
	originalMotions.append("Low_Right2.bvh")
	originalMotions.append("Right.bvh")
	originalMotions.append("Up_Center.bvh")
	originalMotions.append("Up_Left.bvh")
	originalMotions.append("Up_Left_Center.bvh")
	originalMotions.append("Up_Right.bvh")
	originalMotions.append("Up_Right_Far.bvh")
	# and the mirrored motions
	originalMotions.append("Center.bvh" + "Lt")
	originalMotions.append("Center2.bvh" + "Lt")
	originalMotions.append("Far_Center_Up.bvh" + "Lt")
	originalMotions.append("Far_Left.bvh" + "Lt")
	originalMotions.append("Far_Left2.bvh" + "Lt")
	originalMotions.append("Far_Low_Left.bvh" + "Lt")
	originalMotions.append("Far_Lower_Right.bvh" + "Lt")
	originalMotions.append("Far_Lower_Right2.bvh" + "Lt")
	originalMotions.append("Far_Right.bvh" + "Lt")
	originalMotions.append("Left.bvh" + "Lt")
	originalMotions.append("Low_Center.bvh" + "Lt")
	originalMotions.append("Low_Left.bvh" + "Lt")
	originalMotions.append("Low_Right.bvh" + "Lt")
	originalMotions.append("Low_Right2.bvh" + "Lt")
	originalMotions.append("Right.bvh" + "Lt")
	originalMotions.append("Up_Center.bvh" + "Lt")
	originalMotions.append("Up_Left.bvh" + "Lt")
	originalMotions.append("Up_Left_Center.bvh" + "Lt")
	originalMotions.append("Up_Right.bvh" + "Lt")
	originalMotions.append("Up_Right_Far.bvh" + "Lt")
	
	motions = StringVec()
	assetManager = scene.getAssetManager()
	for i in range(0, len(originalMotions)):
		motions.append(prefix + originalMotions[i])
		sbMotion = assetManager.getMotion(originalMotions[i])
		if sbMotion != None:
			sbMotion.setMotionSkeletonName(origSkelName)
	
	para = DoubleVec();
	for i in range(0,3):
		para.append(0)
	para.append(0)
	for i in range(0, len(motions)):
		blendCatching.addMotion(motions[i], para)		
		
	points0 = DoubleVec()
	points0.append(0) # Center 0
	points0.append(0) # Center2 0
	points0.append(0) # Far_Center_Up 0
	points0.append(0) # Far_Left 0
	points0.append(0) # Far_Left2 0
	points0.append(0) # Far_Low_Left 0
	points0.append(0) # Far_Lower_Right 0
	points0.append(0) # Far_Lower_Right2 0
	points0.append(0) # Far_Right 0
	points0.append(0) # Left 0
	points0.append(0) # Low_Center 0
	points0.append(0) # Low_Left 0
	points0.append(0) # Low_Right 0
	points0.append(0) # Low_Right2 0
	points0.append(0) # Right 0
	points0.append(0) # Up_Center 0
	points0.append(0) # Up_Left 0
	points0.append(0) # Low_Center 0
	points0.append(0) # Up_Left_Center 0
	points0.append(0) # Up_Right 0
	
	points0.append(0) # CenterLt 0
	points0.append(0) # Center2Lt 0
	points0.append(0) # Far_Center_UpLt 0
	points0.append(0) # Far_LeftLt 0
	points0.append(0) # Far_Left2Lt 0
	points0.append(0) # Far_Low_LeftLt 0
	points0.append(0) # Far_Lower_RightLt 0
	points0.append(0) # Far_Lower_Right2Lt 0
	points0.append(0) # Far_RightLt 0
	points0.append(0) # LeftLt 0
	points0.append(0) # Low_CenterLt 0
	points0.append(0) # Low_LeftLt 0
	points0.append(0) # Low_RightLt 0
	points0.append(0) # Low_Right2Lt 0
	points0.append(0) # RightLt 0
	points0.append(0) # Up_CenterLt 0
	points0.append(0) # Up_LeftLt 0
	points0.append(0) # Low_CenterLt 0
	points0.append(0) # Up_Left_CenterLt 0
	points0.append(0) # Up_RightLt 0
	blendCatching.addCorrespondencePoints(motions, points0)
	
	
	points1 = DoubleVec()
	points1.append(3.597) # Center 1
	points1.append(2.475) # Center2 1
	points1.append(2.277) # Far_Center_Up 1
	points1.append(1.848) # Far_Left 1
	points1.append(3.069) # Far_Left2 1
	points1.append(2.343) # Far_Low_Left 1
	points1.append(3.102) # Far_Lower_Right 1
	points1.append(3.399) # Far_Lower_Right2 1
	points1.append(1.551) # Far_Right 1
	points1.append(1.617) # Left 1
	points1.append(1.881) # Low_Center 1
	points1.append(2.772) # Low_Left 1
	points1.append(2.673) # Low_Right 1
	points1.append(2.574) # Low_Right2 1
	points1.append(5.148) # Right 1
	points1.append(2.013) # Up_Center 1
	points1.append(2.376) # Up_Left 1
	points1.append(1.947) # Low_Center 1
	points1.append(3.168) # Up_Left_Center 1
	points1.append(2.112) # Up_Right 1
	
	points1.append(3.597) # CenterLt 1
	points1.append(2.475) # Center2Lt 1
	points1.append(2.277) # Far_Center_UpLt 1
	points1.append(1.848) # Far_LeftLt 1
	points1.append(3.069) # Far_Left2Lt 1
	points1.append(2.343) # Far_Low_LeftLt 1
	points1.append(3.102) # Far_Lower_RightLt 1
	points1.append(3.399) # Far_Lower_Right2Lt 1
	points1.append(1.551) # Far_RightLt 1
	points1.append(1.617) # LeftLt 1
	points1.append(1.881) # Low_CenterLt 1
	points1.append(2.772) # Low_LeftLt 1
	points1.append(2.673) # Low_RightLt 1
	points1.append(2.574) # Low_Right2Lt 1
	points1.append(5.148) # RightLt 1
	points1.append(2.013) # Up_CenterLt 1
	points1.append(2.376) # Up_LeftLt 1
	points1.append(1.947) # Low_CenterLt 1
	points1.append(3.168) # Up_Left_CenterLt 1
	points1.append(2.112) # Up_RightLt 1
	blendCatching.addCorrespondencePoints(motions, points1)
	
	blendCatching.buildBlendBase("catch", "Inverse" ,True);	
	
	

