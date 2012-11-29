import math
print "|--------------------------------------------|"
print "|      Starting Multiple Character Demo      |"
print "|--------------------------------------------|"

# Add asset paths
scene.setMediaPath('../../../../data')
scene.addAssetPath('script', 'sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('mesh', 'retarget/mesh')
scene.addAssetPath('motion', 'ChrRachel')
scene.loadAssets()

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 367.58, 574.66)
camera.setCenter(0, 275, 395)

# Set proper motion scale
print 'Scaling motion to fit rescaled skeleton'
scene.getMotion('ChrRachel_ChrBrad@Idle01').scale(100)

print 'Adding characters into scene'
# Set up multiple Rachels
amount = 25
row = 0; column = 0; 
offsetX = 0; offsetZ = 0;
for i in range(amount):
	baseName = 'ChrRachel%s' % i
	rachel = scene.createCharacter(baseName, '')
	rachelSkeleton = scene.createSkeleton('ChrRachel.sk')
	rachelSkeleton.rescale(100)
	rachel.setSkeleton(rachelSkeleton)
	# Set position logic
	posX = (-100 * (5/2)) + 100 * column
	posZ = ((-100 / math.sqrt(amount)) * (amount/2)) + 100 * row	
	column = column + 1
	if column >= 5:
		column = 0
		row = row + 1
	rachelPos = SrVec(posX + offsetX, 0, posZ + offsetZ)
	rachel.setPosition(rachelPos)
	# Set up standard controllers
	rachel.createStandardControllers()
	# Set deformable mesh
	rachel.setDoubleAttribute('deformableMeshScale', 1)
	rachel.setStringAttribute('deformableMesh', 'ChrRachel')
	# Play default animation
	bml.execBML(baseName, '<body posture="ChrRachel_ChrBrad@Idle01_YouLf01"/>')

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Set camera position
scene.getPawn('camera').setPosition(SrVec(0, -50, 0))