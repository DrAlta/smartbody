print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/bradrachel.py  |"
print "|--------------------------------------------|"


### Load data/sbm-common assets
scene.addAssetPath("script", "sbm-common/scripts")
scene.addAssetPath("mesh", "mesh")
scene.addAssetPath("audio", ".")

scene.setScale(1.0)
scene.setBoolAttribute("internalAudio", True)

camera = scene.getActiveCamera()
camera.setEye(0, 2.0478, 4.69259)
camera.setCenter(0.012, 1.60887, 2.75628)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(0.4)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

# map to the SmartBody standard
scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath("motion", "ChrBrad")
scene.loadAssets()

# establish lip syncing data set
scene.run("init-diphoneDefault.py")

scene.run("gestureMap.py")

############################# Brad
bradSkeleton = scene.getSkeleton("ChrBrad.sk")
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse("ChrBrad")

# Brad's face definition
bradFace = scene.createFaceDefinition("ChrBrad")
bradFace.setFaceNeutral("ChrBrad@face_neutral")

bradFace.setAU(1,  "left",  "ChrBrad@001_inner_brow_raiser_lf")
bradFace.setAU(1,  "right", "ChrBrad@001_inner_brow_raiser_rt")
bradFace.setAU(2,  "left",  "ChrBrad@002_outer_brow_raiser_lf")
bradFace.setAU(2,  "right", "ChrBrad@002_outer_brow_raiser_rt")
bradFace.setAU(4,  "left",  "ChrBrad@004_brow_lowerer_lf")
bradFace.setAU(4,  "right", "ChrBrad@004_brow_lowerer_rt")
bradFace.setAU(5,  "both",  "ChrBrad@005_upper_lid_raiser")
bradFace.setAU(6,  "both",  "ChrBrad@006_cheek_raiser")
bradFace.setAU(7,  "both",  "ChrBrad@007_lid_tightener")
bradFace.setAU(10, "both",  "ChrBrad@010_upper_lip_raiser")
bradFace.setAU(12, "left",  "ChrBrad@012_lip_corner_puller_lf")
bradFace.setAU(12, "right", "ChrBrad@012_lip_corner_puller_rt")
bradFace.setAU(25, "both",  "ChrBrad@025_lips_part")
bradFace.setAU(26, "both",  "ChrBrad@026_jaw_drop")
bradFace.setAU(45, "left",  "ChrBrad@045_blink_lf")
bradFace.setAU(45, "right", "ChrBrad@045_blink_rt")

bradFace.setViseme("open",    "ChrBrad@open")
bradFace.setViseme("W",       "ChrBrad@W")
bradFace.setViseme("ShCh",    "ChrBrad@ShCh")
bradFace.setViseme("PBM",     "ChrBrad@PBM")
bradFace.setViseme("FV",      "ChrBrad@FV")
bradFace.setViseme("wide",    "ChrBrad@wide")
bradFace.setViseme("tBack",   "ChrBrad@tBack")
bradFace.setViseme("tRoof",   "ChrBrad@tRoof")
bradFace.setViseme("tTeeth",  "ChrBrad@tTeeth")

brad = scene.createCharacter("ChrBrad", "")
bradSkeleton = scene.createSkeleton("ChrBrad.sk")
brad.setSkeleton(bradSkeleton)
brad.setFaceDefinition(bradFace)
bradPos = SrVec(0, 0, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(0, 0, 0)
brad.setHPR(bradHPR)
brad.createStandardControllers()
# deformable mesh
brad.setStringAttribute("deformableMesh", "ChrBrad")
# lip syncing diphone setup
brad.setStringAttribute("diphoneSetName", "default")
brad.setBoolAttribute("useDiphone", True)
brad.setVoice("audiofile")
brad.setVoiceCode("Sounds")
# gesture map setup
brad.setStringAttribute("gestureMap", "ChrBrad")
brad.setStringAttribute("gestureMapMeek", "ChrBradSad")
brad.setStringAttribute("gestureMapEmphatic", "ChrBradEmphatic")
brad.setBoolAttribute("gestureRequest.autoGestureTransition", True)

# start the simulation
sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBML('ChrBrad', '<saccade mode="listen"/>')
sim.resume()

bml.execBML('ChrBrad', '<gaze sbm:handle="flash" sbm:target-pos="0 0 0"/>')
bml.execBML('ChrBrad', '<gaze sbm:handle="flash" sbm:fade-out="0.2"/>')


