print "|--------------------------------------------|"
print "|          Starting Gestures Demo            |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'ChrRachel')
scene.addAssetPath('script', 'scripts')
scene.loadAssets()

# Set scene parameters
print 'Configuring scene parameters and camera'
scene.setScale(1.0)
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(-0.5, 1.87, 3.43)
camera.setCenter(-0.5, 1.36, 1.51)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, -2, 0))

# Set joint map for Brad and Rachel
print 'Setting up joint map for Brad and Rachel'
scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse('ChrBrad')
rachelSkeleton = scene.getSkeleton('ChrRachel.sk')
zebra2Map.applySkeleton(rachelSkeleton)
zebra2Map.applyMotionRecurse('ChrRachel')

# Setting up Brads and Rachels
print 'Setting up Brads'
bradPosX = -145

# prepare male gestures
scene.run('BehaviorSetGestures.py')
setupBehaviorSet()

for i in range(2):
	baseName = 'ChrBrad%s' % i
	brad = scene.createCharacter(baseName, '')
	bradSkeleton = scene.createSkeleton('ChrBrad.sk')
	brad.setSkeleton(bradSkeleton)
	# Set position
	bradPos = SrVec((bradPosX + (i * 100))/100, 0, 0)
	brad.setPosition(bradPos)
	brad.createStandardControllers()
	# Set deformable mesh
	brad.setDoubleAttribute('deformableMeshScale', 0.01)
	brad.setStringAttribute('deformableMesh', 'ChrMaarten.dae')
	# Play idle animation
	bml.execBML(baseName, '<body posture="ChrBrad@Idle01"/>')
	retargetBehaviorSet(baseName)
	
print 'Setting up Rachels'
# prepare female gestures
scene.run('BehaviorSetFemaleGestures.py')
setupBehaviorSet()
rachelPosX = 45
for i in range(2):
	baseName = 'ChrRachel%s' % i
	rachel = scene.createCharacter(baseName, '')
	rachelSkeleton = scene.createSkeleton('ChrRachel.sk')
	rachel.setSkeleton(rachelSkeleton)
	# Set position
	rachelPos = SrVec((rachelPosX + (i * 100))/100, 0, 0)
	rachel.setPosition(rachelPos)
	rachel.createStandardControllers()
	# Set deformable mesh
	rachel.setDoubleAttribute('deformableMeshScale', 0.01)
	rachel.setStringAttribute('deformableMesh', 'ChrRachel.dae')
	# Play idle animation
	bml.execBML(baseName, '<body posture="ChrRachel_ChrBrad@Idle01"/>')
	retargetBehaviorSet(baseName)

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.getCharacter(name).setStringAttribute("displayType", "GPUmesh")

last = 0
delay = 5
class GesturesDemo(SBScript):
	def update(self, time):
		global last
		diff = time - last
		# When time's up, do action
		if diff >= delay:
			# Brad gestures
			# With pauses (manual/hardcoded)
			bml.execBML('ChrBrad0', '<gesture id="cbrad0" lexeme="DEICTIC" type="YOU" stroke="0" relax="2" sbm:joint-range="l_shoulder" sbm:frequency="0.09" sbm:scale="0.04"/>')
			bml.execBML('ChrBrad0', '<gesture id="cbrad0" lexeme="METAPHORIC" type="OBLIGATION" mode="LEFT_HAND" stroke="3" relax="5" sbm:joint-range="l_shoulder" sbm:frequency="0.09" sbm:scale="0.04"/>')
			bml.execBML('ChrBrad0', '<gesture id="cbrad0" lexeme="DEICTIC" type="ME" stroke="6" relax="8" sbm:joint-range="l_shoulder" sbm:frequency="0.09" sbm:scale="0.04"/>')
			# No pauses
			bml.execBML('ChrBrad1', '<gesture id="cbrad1" lexeme="DEICTIC" type="YOU" stroke="0" relax="2"/>')
			bml.execBML('ChrBrad1', '<gesture id="cbrad1" lexeme="METAPHORIC" type="OBLIGATION" mode="LEFT_HAND" stroke="2" relax="4"/>')
			bml.execBML('ChrBrad1', '<gesture id="cbrad1" lexeme="DEICTIC" type="ME" stroke="4"/>')
			
			# Rachel gestures
			# With pauses (automatic)
			bml.execBML('ChrRachel0', '<gesture id="a" lexeme="DEICTIC" type="ME" stroke="2" relax="3" sbm:joint-range="l_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')
			bml.execBML('ChrRachel0', '<gesture id="b" lexeme="DEICTIC" type="LEFT" mode="LEFT_HAND" start="a:relax" ready="a:relax" stroke_start="a:relax" stroke="5" relax="7" sbm:joint-range="l_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')
			bml.execBML('ChrRachel0', '<gesture id="c" lexeme="DEICTIC" type="YOU" start="b:relax" ready="b:relax" stroke_start="b:relax" stroke="9" relax="11" sbm:joint-range="l_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')
			bml.execBML('ChrRachel0', '<gesture id="d" lexeme="DEICTIC" type="RIGHT" mode="RIGHT_HAND" start="c:relax" ready="c:relax" stroke_start="c:relax" stroke="13" relax="14" sbm:joint-range="r_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')
			# No pauses
			bml.execBML('ChrRachel1', '<gesture id="a" lexeme="DEICTIC" type="ME" stroke="2" relax="4"/>')
			bml.execBML('ChrRachel1', '<gesture id="d" lexeme="DEICTIC" type="LEFT" mode="LEFT_HAND" stroke="4" relax="6"/>')
			bml.execBML('ChrRachel1', '<gesture id="b" lexeme="DEICTIC" type="YOU" stroke="6" relax="8"/>')
			bml.execBML('ChrRachel1', '<gesture id="c" lexeme="DEICTIC" type="RIGHT" mode="RIGHT_HAND" stroke="8" relax="10"/>')
			
			# Reset timer
			last = time
			
			
# Run the update script
scene.removeScript('gesturesdemo')
gesturesdemo = GesturesDemo()
scene.addScript('gesturesdemo', gesturesdemo)
