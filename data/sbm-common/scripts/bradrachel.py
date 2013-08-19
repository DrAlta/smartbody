print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/bradrachel.py  |"
print "|--------------------------------------------|"

scene.run("motion-retarget.py")
### Load data/sbm-common assets
scene.addAssetPath("script", "sbm-common/scripts")
scene.addAssetPath("mesh", "mesh")
scene.addAssetPath("audio", ".")

scene.setScale(1.0)
scene.setBoolAttribute("internalAudio", True)


scene.run("default-viewer.py")


cameraPawn = scene.getPawn("camera")
cameraPos = SrVec(0, 1.6, 10)
cameraPawn.setPosition(cameraPos)

camera = getCamera()
camera.setEye(-0.35, 1.63, 2.3)
camera.setCenter(0.012, 1.18, 0.40)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)


# map to the SmartBody standard
scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath("motion", "ChrRachel")
scene.addAssetPath("motion", "ChrBrad")
scene.addAssetPath("motion", "sbm-common/common-sk")
scene.loadAssets()

scene.run("gestureMap.py")

# established lip syncing data set
scene.run("init-diphoneDefault.py")

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
bradPos = SrVec(.35, 0, 0)
brad.setPosition(bradPos)
bradHPR = SrVec(-17, 0, 0)
brad.setHPR(bradHPR)
brad.createStandardControllers()
# deformable mesh
brad.setDoubleAttribute("deformableMeshScale", .01)
brad.setStringAttribute("deformableMesh", "ChrBrad")
brad.setStringAttribute("gestureMap", "ChrBrad")
# lip syncing diphone setup
brad.setStringAttribute("lipSyncSetName", "default")
brad.setBoolAttribute("usePhoneBigram", True)
#brad.setVoice("remote")
#brad.setVoiceCode("Festival_voice_kal_diphone")
brad.setVoice("audiofile")
brad.setVoiceCode("Sounds")


"""
############################# Rachel
rachelSkeleton = scene.getSkeleton("ChrRachel.sk")
zebra2Map.applySkeleton(rachelSkeleton)
zebra2Map.applyMotionRecurse("ChrRachel")

# Rachel's face definition
rachelFace = scene.createFaceDefinition("ChrRachel")
rachelFace.setFaceNeutral("ChrRachel@face_neutral")

rachelFace.setAU(1,  "left",  "ChrRachel@001_inner_brow_raiser_lf")
rachelFace.setAU(1,  "right", "ChrRachel@001_inner_brow_raiser_rt")
rachelFace.setAU(2,  "left",  "ChrRachel@002_outer_brow_raiser_lf")
rachelFace.setAU(2,  "right", "ChrRachel@002_outer_brow_raiser_rt")
rachelFace.setAU(4,  "left",  "ChrRachel@004_brow_lowerer_lf")
rachelFace.setAU(4,  "right", "ChrRachel@004_brow_lowerer_rt")
rachelFace.setAU(5,  "both",  "ChrRachel@005_upper_lid_raiser")
rachelFace.setAU(6,  "both",  "ChrRachel@006_cheek_raiser")
rachelFace.setAU(7,  "both",  "ChrRachel@007_lid_tightener")
rachelFace.setAU(10, "both",  "ChrRachel@010_upper_lip_raiser")
rachelFace.setAU(12, "left",  "ChrRachel@012_lip_corner_puller_lf")
rachelFace.setAU(12, "right", "ChrRachel@012_lip_corner_puller_rt")
rachelFace.setAU(25, "both",  "ChrRachel@025_lips_part")
rachelFace.setAU(26, "both",  "ChrRachel@026_jaw_drop")
rachelFace.setAU(45, "left",  "ChrRachel@045_blink_lf")
rachelFace.setAU(45, "right", "ChrRachel@045_blink_rt")

rachelFace.setViseme("open",    "ChrRachel@open")
rachelFace.setViseme("W",       "ChrRachel@W")
rachelFace.setViseme("ShCh",    "ChrRachel@ShCh")
rachelFace.setViseme("PBM",     "ChrRachel@PBM")
rachelFace.setViseme("FV",      "ChrRachel@FV")
rachelFace.setViseme("wide",    "ChrRachel@wide")
rachelFace.setViseme("tBack",   "ChrRachel@tBack")
rachelFace.setViseme("tRoof",   "ChrRachel@tRoof")
rachelFace.setViseme("tTeeth",  "ChrRachel@tTeeth")

rachel = scene.createCharacter("ChrRachel", "")
rachelSkeleton = scene.createSkeleton("ChrRachel.sk")
rachel.setSkeleton(rachelSkeleton)
rachel.setFaceDefinition(rachelFace)
rachelPos = SrVec(-.35, 0, 0)
rachel.setPosition(rachelPos)
rachelHPR = SrVec(17, 0, 0)
rachel.setHPR(rachelHPR)
rachel.createStandardControllers()
# deformable mesh
rachel.setDoubleAttribute("deformableMeshScale", .01)
rachel.setStringAttribute("deformableMesh", "ChrRachel")
# lip syncing diphone setup
rachel.setStringAttribute("lipSyncSetName", "default")
rachel.setBoolAttribute("usePhoneBigram", True)
#rachel.setVoice("remote")
#rachel.setVoiceCode("MicrosoftAnna")
rachel.setVoice("audiofile")
rachel.setVoiceCode("Sounds")
"""
scene.setDefaultCharacter("ChrBrad")
scene.setDefaultRecipient("ChrRachel")

#scene.run("runNVBG.py")
# start the simulation
sim.start()

bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBML('ChrBrad', '<saccade mode="listen"/>')

"""
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')
bml.execBML('ChrRachel', '<body posture="ChrRachel_ChrBrad@Idle01" start="1"/>')

bml.execBML('ChrBrad', '<saccade mode="listen"/>')
bml.execBML('ChrRachel', '<saccade mode="listen"/>')

bml.execBML('ChrBrad', '<gaze sbm:handle="brad" target="camera"/>')
bml.execBML('ChrRachel', '<gaze sbm:handle="rachel" target="camera"/>')
"""

sim.resume()
