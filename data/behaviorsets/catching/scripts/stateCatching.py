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
	points0.append(0) # Up_Left_Center 0
	points0.append(0) # Up_Right 0
	points0.append(0) # Up_Right_Far 0
	
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
	points0.append(0) # Up_Left_CenterLt 0
	points0.append(0) # Up_Right 0
	points0.append(0) # Up_Right_Far 0
	blendCatching.addCorrespondencePoints(motions, points0)
	
	points1 = DoubleVec()
	points1.append(1.18) # Center 1
	points1.append(1.05) # Center2 1
	points1.append(.83) # Far_Center_Up 1
	points1.append(.91) # Far_Left 1
	points1.append(1.16) # Far_Left2 1
	points1.append(.68) # Far_Low_Left 1
	points1.append(1.32) # Far_Lower_Right 1
	points1.append(1.60) # Far_Lower_Right2 1
	points1.append(.89) # Far_Right 1
	points1.append(1.14) # Left 1
	points1.append(.92) # Low_Center 1
	points1.append(1.11) # Low_Left 1
	points1.append(1.40) # Low_Right 1
	points1.append(1.41) # Low_Right2 1
	points1.append(1.53) # Right 1
	points1.append(.83) # Up_Center 1
	points1.append(1.30) # Up_Left 1
	points1.append(.99) # Up_Left_Center 1
	points1.append(1.0) # Up_Right 1
	points1.append(.96) # Up_Right_Far 1
	
	points1.append(1.18) # CenterLt 1
	points1.append(1.05) # Center2Lt 1
	points1.append(.83) # Far_Center_UpLt 1
	points1.append(.91) # Far_LeftLt 1
	points1.append(1.16) # Far_Left2Lt 1
	points1.append(.68) # Far_Low_LeftLt 1
	points1.append(1.32) # Far_Lower_RightLt 1
	points1.append(1.60) # Far_Lower_Right2Lt 1
	points1.append(.89) # Far_RightLt 1
	points1.append(1.14) # LeftLt 1
	points1.append(.92) # Low_CenterLt 1
	points1.append(1.11) # Low_LeftLt 1
	points1.append(1.40) # Low_RightLt 1
	points1.append(1.41) # Low_Right2Lt 1
	points1.append(1.53) # RightLt 1
	points1.append(.83) # Up_CenterLt 1
	points1.append(1.30) # Up_LeftLt 1
	points1.append(.99) # Up_Left_CenterLt 1
	points1.append(1.0) # Up_RightLt 1
	points1.append(.96) # Up_Right_Far 1
	
	points2 = DoubleVec()
	points2.append(3.597) # Center 2
	points2.append(2.475) # Center2 2
	points2.append(2.277) # Far_Center_Up 2
	points2.append(1.848) # Far_Left 2
	points2.append(3.069) # Far_Left2 2
	points2.append(2.343) # Far_Low_Left 2
	points2.append(3.102) # Far_Lower_Right 2
	points2.append(3.399) # Far_Lower_Right2 2
	points2.append(1.551) # Far_Right 2
	points2.append(1.617) # Left 2
	points2.append(1.881) # Low_Center 2
	points2.append(2.772) # Low_Left 2
	points2.append(2.673) # Low_Right 2
	points2.append(2.574) # Low_Right2 2
	points2.append(5.148) # Right 2
	points2.append(2.013) # Up_Center 2
	points2.append(2.376) # Up_Left 2
	points2.append(1.947) # Up_Left_Center 2
	points2.append(3.168) # Up_Right 2
	points2.append(2.112) # Up_Right_Far 2
	
	points2.append(3.597) # CenterLt 2
	points2.append(2.475) # Center2Lt 2
	points2.append(2.277) # Far_Center_UpLt 2
	points2.append(1.848) # Far_LeftLt 2
	points2.append(3.069) # Far_Left2Lt 2
	points2.append(2.343) # Far_Low_LeftLt 2
	points2.append(3.102) # Far_Lower_RightLt 2
	points2.append(3.399) # Far_Lower_Right2Lt 2
	points2.append(1.551) # Far_RightLt 2
	points2.append(1.617) # LeftLt 2
	points2.append(1.881) # Low_CenterLt 2
	points2.append(2.772) # Low_LeftLt 2
	points2.append(2.673) # Low_RightLt 2
	points2.append(2.574) # Low_Right2Lt 2
	points2.append(5.148) # RightLt 2
	points2.append(2.013) # Up_CenterLt 2
	points2.append(2.376) # Up_LeftLt 2
	points2.append(1.947) # Up_Left_CenterLt 2
	points2.append(3.168) # Up_RightLt 2
	points2.append(2.112) # Up_Right_FarLt 2
	blendCatching.addCorrespondencePoints(motions, points2)
	
	# generate the 3-dimensional parameters for the blend based
	# on the location of the catching point
	
	skeleton = scene.getSkeleton(skeletonName)
	rhand = skeleton.getJointByName("r_wrist")
	lhand = skeleton.getJointByName("l_wrist")
	for r in range(0, 9):
		m = scene.getMotion(originalMotions[r])
		m.connect(skeleton)
		rpos = m.getJointPosition(rhand, points1[r])
		params = DoubleVec()
		params.append(rpos.getData(0))
		params.append(rpos.getData(1))
		params.append(rpos.getData(2))
		blendCatching.setParameter(originalMotions[r], params)	
#		print originalMotions[r] + " " + str(params[0]) + " " + str(params[1]) + " " +str(params[2])
		
	for l in range(10, 19):
		m = scene.getMotion(originalMotions[l])
		m.connect(skeleton)
		lpos = m.getJointPosition(lhand, points1[l])
		params = DoubleVec()
		params.append(lpos.getData(0))
		params.append(lpos.getData(1))
		params.append(lpos.getData(2))
		blendCatching.setParameter(originalMotions[l], params)
#		print originalMotions[l] + " " + str(params[0]) + " " + str(params[1]) + " " +str(params[2])
	
	#blendCatching.buildBlendBase("catch", "RBF" ,True);	
	#blendCatching.buildBlendBase("catch", "Inverse" ,True);	
	blendCatching.buildBlendBase("catch", "KNN" ,True);	
	
	

