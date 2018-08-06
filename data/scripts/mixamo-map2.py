# Mapping from Mixamo skeleton to SmartBody skeleton

jointMapManager = scene.getJointMapManager()
mixamoMap = jointMapManager.getJointMap("mixamorig")

if mixamoMap == None:
	mixamoMap = jointMapManager.createJointMap("mixamorig")
	mixamoMap.setMapping("mixamorig_RightEye", "eyeball_right")
	mixamoMap.setMapping("mixamorig_LeftEye", "eyeball_left")

	mixamoMap.setMapping("mixamorig_Hips", "base")
	mixamoMap.setMapping("mixamorig_Spine", "spine1")
	mixamoMap.setMapping("mixamorig_Spine1", "spine2")
	mixamoMap.setMapping("mixamorig_Spine2", "spine3")
	mixamoMap.setMapping("mixamorig_Neck", "spine4")
	mixamoMap.setMapping("mixamorig_Head", "spine5")
	mixamoMap.setMapping("mixamorig_LeftShoulder", "l_sternoclavicular")
	mixamoMap.setMapping("mixamorig_LeftArm", "l_shoulder")
	mixamoMap.setMapping("mixamorig_LeftForeArm", "l_elbow")
	mixamoMap.setMapping("mixamorig_LeftHand", "l_wrist")
	mixamoMap.setMapping("mixamorig_LeftHandThumb1", "l_thumb1")
	mixamoMap.setMapping("mixamorig_LeftHandThumb2", "l_thumb2")
	mixamoMap.setMapping("mixamorig_LeftHandThumb3", "l_thumb3")
	mixamoMap.setMapping("mixamorig_LeftHandThumb4", "l_thumb4")
	mixamoMap.setMapping("mixamorig_LeftHandIndex1", "l_index1")
	mixamoMap.setMapping("mixamorig_LeftHandIndex2", "l_index2")
	mixamoMap.setMapping("mixamorig_LeftHandIndex3", "l_index3")
	mixamoMap.setMapping("mixamorig_LeftHandIndex4", "l_index4")
	mixamoMap.setMapping("mixamorig_LeftHandMiddle1", "l_middle1")
	mixamoMap.setMapping("mixamorig_LeftHandMiddle2", "l_middle2")
	mixamoMap.setMapping("mixamorig_LeftHandMiddle3", "l_middle3")
	mixamoMap.setMapping("mixamorig_LeftHandMiddle4", "l_middle4")
	mixamoMap.setMapping("mixamorig_LeftHandRing1", "l_ring1")
	mixamoMap.setMapping("mixamorig_LeftHandRing2", "l_ring2")
	mixamoMap.setMapping("mixamorig_LeftHandRing3", "l_ring3")
	mixamoMap.setMapping("mixamorig_LeftHandRing4", "l_ring4")
	mixamoMap.setMapping("mixamorig_LeftHandPinky1", "l_pinky1")
	mixamoMap.setMapping("mixamorig_LeftHandPinky2", "l_pinky2")
	mixamoMap.setMapping("mixamorig_LeftHandPinky3", "l_pinky3")
	mixamoMap.setMapping("mixamorig_LeftHandPinky4", "l_pinky4")
	mixamoMap.setMapping("mixamorig_RightShoulder", "r_sternoclavicular")
	mixamoMap.setMapping("mixamorig_RightArm", "r_shoulder")
	mixamoMap.setMapping("mixamorig_RightForeArm", "r_elbow")
	mixamoMap.setMapping("mixamorig_RightHand", "r_wrist")
	mixamoMap.setMapping("mixamorig_RightHandThumb1", "r_thumb1")
	mixamoMap.setMapping("mixamorig_RightHandThumb2", "r_thumb2")
	mixamoMap.setMapping("mixamorig_RightHandThumb3", "r_thumb3")
	mixamoMap.setMapping("mixamorig_RightHandThumb4", "r_thumb4")
	mixamoMap.setMapping("mixamorig_RightHandIndex1", "r_index1")
	mixamoMap.setMapping("mixamorig_RightHandIndex2", "r_index2")
	mixamoMap.setMapping("mixamorig_RightHandIndex3", "r_index3")
	mixamoMap.setMapping("mixamorig_RightHandIndex4", "r_index4")
	mixamoMap.setMapping("mixamorig_RightHandMiddle1", "r_middle1")
	mixamoMap.setMapping("mixamorig_RightHandMiddle2", "r_middle2")
	mixamoMap.setMapping("mixamorig_RightHandMiddle3", "r_middle3")
	mixamoMap.setMapping("mixamorig_RightHandMiddle4", "r_middle4")
	mixamoMap.setMapping("mixamorig_RightHandRing1", "r_ring1")
	mixamoMap.setMapping("mixamorig_RightHandRing2", "r_ring2")
	mixamoMap.setMapping("mixamorig_RightHandRing3", "r_ring3")
	mixamoMap.setMapping("mixamorig_RightHandRing4", "r_ring4")
	mixamoMap.setMapping("mixamorig_RightHandPinky1", "r_pinky1")
	mixamoMap.setMapping("mixamorig_RightHandPinky2", "r_pinky2")
	mixamoMap.setMapping("mixamorig_RightHandPinky3", "r_pinky3")
	mixamoMap.setMapping("mixamorig_RightHandPinky4", "r_pinky4")
	mixamoMap.setMapping("mixamorig_LeftUpLeg", "l_hip")
	mixamoMap.setMapping("mixamorig_LeftLeg", "l_knee")
	mixamoMap.setMapping("mixamorig_LeftFoot", "l_ankle")
	mixamoMap.setMapping("mixamorig_LeftToeBase", "l_forefoot")
	mixamoMap.setMapping("mixamorig_LeftFootToeBase_End", "l_toe")
	mixamoMap.setMapping("mixamorig_LeftToe_End", "l_toe")
	mixamoMap.setMapping("mixamorig_RightUpLeg", "r_hip")
	mixamoMap.setMapping("mixamorig_RightLeg", "r_knee")
	mixamoMap.setMapping("mixamorig_RightFoot", "r_ankle")
	mixamoMap.setMapping("mixamorig_RightToeBase", "r_forefoot")
	mixamoMap.setMapping("mixamorig_RightFootToeBase_End", "r_toe")
	mixamoMap.setMapping("mixamorig_RightToe_End", "r_toe")