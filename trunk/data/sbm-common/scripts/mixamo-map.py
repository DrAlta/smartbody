# Mapping from Mixamo skeleton to SmartBody skeleton
# Before loading character, run this;
#   seq mixamo-map.seq
# then map any existing skeletons:
#   skeletonmap <skeleton> mixamo
# also map any motions to mixamo:
#   motionmap <motion> mixamo

jointMapManager = scene.getJointMapManager()

mixamoMap = jointMapManager.createJointMap("mixamo")

mixamoMap.setMapping("Hips", "base")
mixamoMap.setMapping("Neck", "spine4")
mixamoMap.setMapping("Neck1", "spine5")
mixamoMap.setMapping("Head", "skullbase")
mixamoMap.setMapping("LeftShoulder", "l_acromioclavicular")
mixamoMap.setMapping("LeftArm", "l_shoulder")
mixamoMap.setMapping("LeftForeArm", "l_elbow")
mixamoMap.setMapping("LeftHand", "l_wrist")
mixamoMap.setMapping("LeftHandThumb1", "l_thumb1")
mixamoMap.setMapping("LeftHandThumb2", "l_thumb2")
mixamoMap.setMapping("LeftHandThumb3", "l_thumb4")
mixamoMap.setMapping("LeftHandIndex1", "l_index1")
mixamoMap.setMapping("LeftHandIndex2", "l_index2")
mixamoMap.setMapping("LeftHandIndex3", "l_index4")
mixamoMap.setMapping("LeftHandMiddle1", "l_middle1")
mixamoMap.setMapping("LeftHandMiddle2", "l_middle2")
mixamoMap.setMapping("LeftHandMiddle4", "l_middle4")
mixamoMap.setMapping("LeftHandRing1", "l_ring1")
mixamoMap.setMapping("LeftHandRing2", "l_ring2")
mixamoMap.setMapping("LeftHandRing4", "l_ring4")
mixamoMap.setMapping("LeftHandPinky1", "l_pinky1")
mixamoMap.setMapping("LeftHandPinky2", "l_pinky2")
mixamoMap.setMapping("LeftHandPinky3", "l_pinky4")
mixamoMap.setMapping("RightShoulder", "r_acromioclavicular")
mixamoMap.setMapping("RightArm", "r_shoulder")
mixamoMap.setMapping("RightHand", "r_wrist")
mixamoMap.setMapping("RightHandThumb1", "r_thumb1")
mixamoMap.setMapping("RightHandThumb2", "r_thumb2")
mixamoMap.setMapping("RightHandThumb3", "r_thumb4")
mixamoMap.setMapping("RightHandIndex1", "r_index1")
mixamoMap.setMapping("RightHandIndex2", "r_index2")
mixamoMap.setMapping("RightHandIndex3", "r_index4")
mixamoMap.setMapping("RightHandMiddle1", "r_middle1")
mixamoMap.setMapping("RightHandMiddle2", "r_middle2")
mixamoMap.setMapping("RightHandMiddle4", "r_middle4")
mixamoMap.setMapping("RightHandRing1", "r_ring1")
mixamoMap.setMapping("RightHandRing2", "r_ring2")
mixamoMap.setMapping("RightHandRing4", "r_ring4")
mixamoMap.setMapping("RightHandPinky1", "r_pinky1")
mixamoMap.setMapping("RightHandPinky2", "r_pinky2")
mixamoMap.setMapping("RightHandPinky3", "r_pinky4")
mixamoMap.setMapping("LeftUpLeg", "l_hip")
mixamoMap.setMapping("LeftLeg", "l_knee")
mixamoMap.setMapping("LeftFoot", "l_ankle")
mixamoMap.setMapping("RightUpLeg", "r_hip")
mixamoMap.setMapping("RightLeg", "r_knee")
mixamoMap.setMapping("RightFoot", "l_ankle")
mixamoMap.setMapping("RightToeBase", "r_forefoot")
mixamoMap.setMapping("Spine2", "spine3")
mixamoMap.setMapping("Spine1", "spine2")
mixamoMap.setMapping("Spine", "spine1")




