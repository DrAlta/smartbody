print "|--------------------------------------------|"
print "|          Starting Constraint Demo          |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath("script", "behaviorsets")
scene.addAssetPath('script', 'scripts')
scene.loadAssets()

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(10, 250, 328.23)
camera.setCenter(10, 150, 155.43)

# Set joint map for Brad
print 'Setting up joint map for Brad'
scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse('ChrBrad')

# Set proper motion scale
print 'Scaling motion to fit rescaled skeleton'
scene.getMotion('ChrBrad@Idle01').scale(100)

print 'Adding characters into scene'
# Set up multiple Brads
posX = -145;
for i in range(4):
	baseName = 'ChrBrad%s' % i
	brad = scene.createCharacter(baseName, '')
	bradSkeleton = scene.createSkeleton('ChrBrad.sk')
	bradSkeleton.rescale(100)
	brad.setSkeleton(bradSkeleton)
	# Set position
	bradPos = SrVec(posX + (i * 100), 0, 0)
	brad.setPosition(bradPos)
	# Set up standard controllers
	brad.createStandardControllers()
	# Set deformable mesh
	brad.setDoubleAttribute('deformableMeshScale', 1)
	brad.setStringAttribute('deformableMesh', 'ChrBrad.dae')
	# Play idle animation
	bml.execBML(baseName, '<body posture="ChrBrad@Idle01"/>')
	# Retarget character
	if i== 0 : 
		scene.run('BehaviorSetReaching.py')
		setupBehaviorSet()
	retargetBehaviorSet(baseName)	
	
# Set camera position
scene.getPawn('camera').setPosition(SrVec(0, -50, 0))

# Turn on GPU deformable geometry for all
print 'Turning on GPU deformable geometry for all'
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Set up multiple pawns
print 'Adding pawns into scene'
for i in range(5):
	baseName = 'pawn%s' % i
	pawn = scene.createPawn(baseName)
	pawn.setStringAttribute('collisionShape', 'sphere')
	collisionShapeScale = SrVec(5, 5, 5)
	pawn.getAttribute('collisionShapeScale').setValue(collisionShapeScale)

# Set pawn position and collision scale
print 'Set pawn position'
scene.getPawn('pawn0').setPosition(SrVec(130, 130, 15))
scene.getPawn('pawn0').getAttribute('collisionShapeScale').setValue(SrVec(0, 0, 0))
scene.getPawn('pawn1').setPosition(SrVec(190, 130, 15))
scene.getPawn('pawn1').getAttribute('collisionShapeScale').setValue(SrVec(0, 0, 0))
scene.getPawn('pawn2').setPosition(SrVec(-100, 175, 150))
scene.getPawn('pawn3').setPosition(SrVec(-110, 155, 43))
scene.getPawn('pawn4').setPosition(SrVec(-10, 155, 43))

# 2 on the left touch pawns
print 'ChrBrad0 and ChrBrad1 touching pawns'
bml.execBMLAt(1, 'ChrBrad0', '<sbm:reach sbm:action="touch" target="pawn3"/>')
bml.execBMLAt(1, 'ChrBrad1', '<sbm:reach sbm:action="touch" target="pawn4"/>')
# Set constraint for ChrBrad0
bml.execBMLAt(2, 'ChrBrad0', '<sbm:constraint effector="l_wrist" sbm:effector-root="l_sternoclavicular" sbm:handle="cbrad0" target="pawn3" sbm:fade-in="0.5"/>')

# 2 on the right use constraint
print 'ChrBrad3 using contraints'
bml.execBMLAt(2, 'ChrBrad3', '<sbm:constraint effector="r_wrist" sbm:effector-root="r_sternoclavicular" sbm:handle="cbrad3" target="pawn0" sbm:fade-in="0.5"/>')
bml.execBMLAt(2, 'ChrBrad3', '<sbm:constraint effector="l_wrist" sbm:effector-root="l_sternoclavicular" sbm:handle="cbrad3" target="pawn1" sbm:fade-in="0.5"/>')

# Make all characters gaze at pawn2
bml.execBMLAt(5, '*', '<gaze target="pawn2"/>')

# Move pawn around
gazeX = -200
dir = 1
speed = 1.0
pawn2 = scene.getPawn('pawn2')

class ConstraintDemo(SBScript):
	def update(self, time):
		global gazeX, dir, speed
		# Change direction when hit border
		if gazeX > 200:
			dir = -1
		elif gazeX < -200:
			dir = 1
		gazeX = gazeX + speed * dir
		# Set pawn position
		pawn2.setPosition(SrVec(gazeX, 175, 150))
		
# Run the update script
scene.removeScript('constraintdemo')
constraintdemo = ConstraintDemo()
scene.addScript('constraintdemo', constraintdemo)