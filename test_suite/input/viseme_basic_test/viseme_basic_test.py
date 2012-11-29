scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.getSimulationManager().setSimFps(60)

scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath('motion','ChrRachel')
scene.loadAssets()

rachelSkeleton = scene.getSkeleton("ChrRachel.sk")
zebra2Map.applySkeleton(rachelSkeleton)
zebra2Map.applyMotionRecurse("ChrRachel")

# Rachel's face definition
rachelFace = scene.createFaceDefinition('ChrRachel')
rachelFace.setFaceNeutral('ChrRachel@face_neutral')

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

rachel = scene.createCharacter('ChrRachel', '')
rachelSkeleton = scene.createSkeleton('ChrRachel.sk')
rachel.setSkeleton(rachelSkeleton)
rachel.setFaceDefinition(rachelFace)
rachel.createStandardControllers()
rachel.setDoubleAttribute('deformableMeshScale', .01)
rachel.setStringAttribute('deformableMesh', 'ChrRachel')

scene.command("char ChrRachel viewer deformableGPU")

# Setting camera parameters
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.08, 1.61, .44)
camera.setCenter(.08, 1.51, -.14)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)

scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

sim.start()
bml.execBML('ChrRachel', '<body posture="ChrRachel_ChrBrad@Idle01"/>')

scene.commandAt(2, 'char ChrRachel viseme open 1 1')
scene.commandAt(3, 'snapshot')
scene.commandAt(3, 'char ChrRachel viseme open 0 1')

scene.commandAt(4, 'char ChrRachel viseme W 1 1')
scene.commandAt(5, 'snapshot')
scene.commandAt(5, 'char ChrRachel viseme W 0 1')

scene.commandAt(6, 'char ChrRachel viseme ShCh 1 1')
scene.commandAt(7, 'snapshot')
scene.commandAt(7, 'char ChrRachel viseme ShCh 0 1')

scene.commandAt(8, 'char ChrRachel viseme PBM 1 1')
scene.commandAt(9, 'snapshot')
scene.commandAt(9, 'char ChrRachel viseme PBM 0 1')

scene.commandAt(10, 'char ChrRachel viseme FV 1 1')
scene.commandAt(11, 'snapshot')
scene.commandAt(11, 'char ChrRachel viseme FV 0 1')

scene.commandAt(12, 'char ChrRachel viseme wide 1 1')
scene.commandAt(13, 'snapshot')
scene.commandAt(13, 'char ChrRachel viseme wide 0 1')

scene.commandAt(14, 'char ChrRachel viseme open 1 1')
scene.commandAt(14, 'char ChrRachel viseme tBack 1 1')
scene.commandAt(15, 'snapshot')
scene.commandAt(15, 'char ChrRachel viseme tBack 0 1')

scene.commandAt(16, 'char ChrRachel viseme tRoof 1 1')
scene.commandAt(17, 'snapshot')
scene.commandAt(17, 'char ChrRachel viseme tRoof 0 1')

scene.commandAt(18, 'char ChrRachel viseme tTeeth 1 1')
scene.commandAt(19, 'snapshot')
scene.commandAt(19, 'char ChrRachel viseme tTeeth 0 1')

scene.commandAt(20, 'quit')