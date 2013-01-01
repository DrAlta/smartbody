import random
print "|--------------------------------------------|"
print "|       Starting Facial Movement Demo        |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath('script', 'sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'ChrRachel')
scene.loadAssets()

# Set scene parameters and camera
print 'Configuring scene parameters and camera'
scene.setScale(1.0)
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0.075, 1.54, 0.57)
camera.setCenter(0.075, 1.29, -1.4)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, -5, 0))

# Setting up joint map for Rachel
print 'Setting up joint map for Rachel'
scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
rachelSkeleton = scene.getSkeleton('ChrRachel.sk')
zebra2Map.applySkeleton(rachelSkeleton)
zebra2Map.applyMotionRecurse('ChrRachel')

# Setting up face definition
print 'Setting up Rachel\'s face definition'
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

# Setting up Rachels
# Basic viseme character
print 'Setting up basic viseme Rachel'
basic = scene.createCharacter('basic', '')
basicSkeleton = scene.createSkeleton('ChrRachel.sk')
basic.setSkeleton(basicSkeleton)
basicPos = SrVec(-.2, 0, 0)
basic.setPosition(basicPos)
basic.setHPR(SrVec(17, 0, 0))
# Set face definition
basic.setFaceDefinition(rachelFace)
# Set up standard controllers
basic.createStandardControllers()
# Deformable mesh
basic.setDoubleAttribute('deformableMeshScale', .01)
basic.setStringAttribute('deformableMesh', 'ChrRachel')
# Idle pose
bml.execBML('basic', '<body posture="ChrRachel_ChrBrad@Idle01"/>')
# Turning on deformable GPU
scene.command('char basic viewer deformableGPU')

# Curve viseme character
print 'Setting up curve viseme Rachel'
curve = scene.createCharacter('curve', '')
curveSkeleton = scene.createSkeleton('ChrRachel.sk')
curve.setSkeleton(curveSkeleton)
curvePos = SrVec(.2, 0, 0)
curve.setPosition(curvePos)
curve.setHPR(SrVec(-17, 0, 0))
# Set face definition
curve.setFaceDefinition(rachelFace)
# Set up standard controllers
curve.createStandardControllers()
# Deformable mesh
curve.setDoubleAttribute('deformableMeshScale', .01)
curve.setStringAttribute('deformableMesh', 'ChrRachel')
# Idle pose
bml.execBML('curve', '<body posture="ChrRachel_ChrBrad@Idle01"/>')
# Turning on deformable GPU
scene.command('char curve viewer deformableGPU')

# Update to repeat reaches
last = 0
canTime = True
delay = 20
class FacialMovementDemo(SBScript):
	def update(self, time):
		global canTime, last
		diff = time - last
		if diff >= delay:
			diff = 0
			canTime = True
		# If time's up, do action
		if canTime:
			# Basic viseme
			scene.commandAt(1, 'char basic viseme open 1 1')
			scene.commandAt(2, 'char basic viseme open 0 1')
			scene.commandAt(3, 'char basic viseme W 1 1')
			scene.commandAt(4, 'char basic viseme W 0 1')
			scene.commandAt(5, 'char basic viseme ShCh 1 1')
			scene.commandAt(6, 'char basic viseme ShCh 0 1')
			scene.commandAt(7, 'char basic viseme PBM 1 1')
			scene.commandAt(8, 'char basic viseme PBM 0 1')
			scene.commandAt(9, 'char basic viseme FV 1 1')
			scene.commandAt(10, 'char basic viseme FV 0 1')
			scene.commandAt(11, 'char basic viseme wide 1 1')
			scene.commandAt(12, 'char basic viseme wide 0 1')
			scene.commandAt(13, 'char basic viseme open 1 1')
			scene.commandAt(13, 'char basic viseme tBack 1 1')
			scene.commandAt(14, 'char basic viseme tBack 0 1')
			scene.commandAt(15, 'char basic viseme tRoof 1 1')
			scene.commandAt(16, 'char basic viseme tRoof 0 1')
			scene.commandAt(17, 'char basic viseme tTeeth 1 1')
			scene.commandAt(17, 'char basic viseme tTeeth 0 1')
			
			# Curve viseme
			scene.commandAt(1, 'char curve viseme open curve 15 1.494999 0.000000 0.000000 0.000000 1.575090 0.907001 0.000000 0.000000 1.728332 0.000000 0.000000 0.000000 2.344998 0.000000 0.000000 0.000000 2.407052 0.434791 0.000000 0.000000 2.444998 0.000000 0.000000 0.000000 2.494998 0.000000 0.000000 0.000000 2.562448 0.979868 0.000000 0.000000 2.678331 0.000000 0.000000 0.000000 3.378331 0.000000 0.000000 0.000000 3.438799 0.936010 0.000000 0.000000 3.494997 0.000000 0.000000 0.000000 4.995010 0.000000 0.000000 0.000000 5.068371 0.722211 0.000000 0.000000 5.128345 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme W curve 20 1.145000 0.000000 0.000000 0.000000 1.207052 0.232501 0.000000 0.000000 1.245000 0.000000 0.000000 0.000000 2.961664 0.000000 0.000000 0.000000 3.084544 0.575955 0.000000 0.000000 3.161664 0.000000 0.000000 0.000000 3.478331 0.000000 0.000000 0.000000 3.548612 0.756913 0.000000 0.000000 3.594997 0.000000 0.000000 0.000000 3.744997 0.000000 0.000000 0.000000 3.831084 0.848908 0.000000 0.000000 3.919096 0.010160 0.000000 0.000000 3.992235 0.788789 0.000000 0.000000 4.094998 0.000000 0.000000 0.000000 4.411669 0.000000 0.000000 0.000000 4.491794 0.998539 0.000000 0.000000 4.778341 0.000000 0.000000 0.000000 5.661686 0.000000 0.000000 0.000000 5.759173 0.987504 0.000000 0.000000 5.845022 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme ShCh curve 12 1.045000 0.000000 0.000000 0.000000 1.150685 0.639824 0.000000 0.000000 1.195000 0.000000 0.000000 0.000000 2.278332 0.000000 0.000000 0.000000 2.354447 0.631276 0.000000 0.000000 2.394998 0.000000 0.000000 0.000000 3.011664 0.000000 0.000000 0.000000 3.145709 0.691957 0.000000 0.000000 3.194998 0.000000 0.000000 0.000000 4.745007 0.000000 0.000000 0.000000 4.995147 0.993372 0.000000 0.000000 5.061678 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme PBM curve 15 0.761667 0.000000 0.000000 0.000000 0.838783 0.994936 0.000000 0.000000 0.911667 0.000000 0.000000 0.000000 2.328332 0.000000 0.000000 0.000000 2.380139 0.410033 0.000000 0.000000 2.428332 0.000000 0.000000 0.000000 3.961663 0.000000 0.000000 0.000000 4.067662 0.083357 0.000000 0.000000 4.094998 0.000000 0.000000 0.000000 5.061678 0.000000 0.000000 0.000000 5.111935 0.764950 0.000000 0.000000 5.195013 0.000000 0.000000 0.000000 6.411696 0.000000 0.000000 0.000000 6.487586 0.995027 0.000000 0.000000 6.545031 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme FV curve 14 0.311667 0.000000 0.000000 0.000000 0.408956 0.988257 0.000000 0.000000 0.495144 0.058030 0.000000 0.000000 0.602841 0.998716 0.000000 0.000000 0.761667 0.000000 0.000000 0.000000 1.978332 0.000000 0.000000 0.000000 2.099215 0.992864 0.000000 0.000000 2.211665 0.000000 0.000000 0.000000 3.978330 0.000000 0.000000 0.000000 4.100688 0.985783 0.000000 0.000000 4.228333 0.000000 0.000000 0.000000 6.195026 0.000000 0.000000 0.000000 6.291809 0.970848 0.000000 0.000000 6.411696 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme wide curve 9 2.378332 0.000000 0.000000 0.000000 2.449136 0.992679 0.000000 0.000000 2.511665 0.000000 0.000000 0.000000 3.811664 0.000000 0.000000 0.000000 3.907230 0.690468 0.000000 0.000000 3.978330 0.000000 0.000000 0.000000 6.061691 0.000000 0.000000 0.000000 6.122948 0.715391 0.000000 0.000000 6.211693 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme tRoof curve 15 1.494999 0.000000 0.000000 0.000000 1.575090 0.907001 0.000000 0.000000 1.728332 0.000000 0.000000 0.000000 2.344998 0.000000 0.000000 0.000000 2.407052 0.434791 0.000000 0.000000 2.444998 0.000000 0.000000 0.000000 2.494998 0.000000 0.000000 0.000000 2.562448 0.979868 0.000000 0.000000 2.678331 0.000000 0.000000 0.000000 3.378331 0.000000 0.000000 0.000000 3.438799 0.936010 0.000000 0.000000 3.494997 0.000000 0.000000 0.000000 4.995010 0.000000 0.000000 0.000000 5.068371 0.722211 0.000000 0.000000 5.128345 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme tTeeth curve 31 0.311667 0.000000 0.000000 0.000000 0.370000 0.302231 0.000000 0.000000 0.411251 0.029209 0.000000 0.000000 0.500579 0.996390 0.000000 0.000000 0.595000 0.000000 0.000000 0.000000 0.845000 0.000000 0.000000 0.000000 0.911489 0.974515 0.000000 0.000000 1.011666 0.000000 0.000000 0.000000 1.245000 0.000000 0.000000 0.000000 1.342800 0.067957 0.000000 0.000000 1.361666 0.000000 0.000000 0.000000 1.644999 0.000000 0.000000 0.000000 1.733897 0.497891 0.000000 0.000000 1.785109 0.009984 0.000000 0.000000 1.884976 0.941893 0.000000 0.000000 1.944999 0.000000 0.000000 0.000000 2.211665 0.000000 0.000000 0.000000 2.285816 0.994784 0.000000 0.000000 2.361665 0.000000 0.000000 0.000000 2.828331 0.000000 0.000000 0.000000 2.960795 0.992375 0.000000 0.000000 3.111664 0.000000 0.000000 0.000000 3.528331 0.000000 0.000000 0.000000 3.602323 0.996746 0.000000 0.000000 3.694997 0.000000 0.000000 0.000000 4.094998 0.000000 0.000000 0.000000 4.230028 0.992379 0.000000 0.000000 4.361669 0.000000 0.000000 0.000000 6.495030 0.000000 0.000000 0.000000 6.556650 0.998295 0.000000 0.000000 6.695033 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme open curve 31 0.311667 0.000000 0.000000 0.000000 0.370000 0.302231 0.000000 0.000000 0.411251 0.029209 0.000000 0.000000 0.500579 0.996390 0.000000 0.000000 0.595000 0.000000 0.000000 0.000000 0.845000 0.000000 0.000000 0.000000 0.911489 0.974515 0.000000 0.000000 1.011666 0.000000 0.000000 0.000000 1.245000 0.000000 0.000000 0.000000 1.342800 0.067957 0.000000 0.000000 1.361666 0.000000 0.000000 0.000000 1.644999 0.000000 0.000000 0.000000 1.733897 0.497891 0.000000 0.000000 1.785109 0.009984 0.000000 0.000000 1.884976 0.941893 0.000000 0.000000 1.944999 0.000000 0.000000 0.000000 2.211665 0.000000 0.000000 0.000000 2.285816 0.994784 0.000000 0.000000 2.361665 0.000000 0.000000 0.000000 2.828331 0.000000 0.000000 0.000000 2.960795 0.992375 0.000000 0.000000 3.111664 0.000000 0.000000 0.000000 3.528331 0.000000 0.000000 0.000000 3.602323 0.996746 0.000000 0.000000 3.694997 0.000000 0.000000 0.000000 4.094998 0.000000 0.000000 0.000000 4.230028 0.992379 0.000000 0.000000 4.361669 0.000000 0.000000 0.000000 6.495030 0.000000 0.000000 0.000000 6.556650 0.998295 0.000000 0.000000 6.695033 0.000000 0.000000 0.000000')
			scene.commandAt(1, 'char curve viseme tBack curve 15 0.761667 0.000000 0.000000 0.000000 0.838783 0.994936 0.000000 0.000000 0.911667 0.000000 0.000000 0.000000 2.328332 0.000000 0.000000 0.000000 2.380139 0.410033 0.000000 0.000000 2.428332 0.000000 0.000000 0.000000 3.961663 0.000000 0.000000 0.000000 4.067662 0.083357 0.000000 0.000000 4.094998 0.000000 0.000000 0.000000 5.061678 0.000000 0.000000 0.000000 5.111935 0.764950 0.000000 0.000000 5.195013 0.000000 0.000000 0.000000 6.411696 0.000000 0.000000 0.000000 6.487586 0.995027 0.000000 0.000000 6.545031 0.000000 0.000000 0.000000')
			
			last = time
			canTime = False

# Run the update script
scene.removeScript('facialmovementdemo')
facialmovementdemo = FacialMovementDemo()
scene.addScript('facialmovementdemo', facialmovementdemo)