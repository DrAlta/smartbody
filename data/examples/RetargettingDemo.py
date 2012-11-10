print "|--------------------------------------------|"
print "|        Starting Retargetting Demo          |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath("script","../../../../data/examples")
scene.addAssetPath("script","../../../../data/examples/functions")
scene.addAssetPath("script","../../../../data/sbm-common/scripts")
scene.addAssetPath('seq', '../../../../data/sbm-common/scripts')
scene.addAssetPath('seq', '../../../../data/sbm-test/scripts')
scene.addAssetPath('mesh', '../../../../data/mesh')
scene.addAssetPath('mesh', '../../../../data/retarget/mesh')
scene.addAssetPath('audio', '../../../../data/Resources/audio')

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 188.56, 241.41)
camera.setCenter(0, 114.56, 56.41)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad1', 'brad')
setPos('brad1', SrVec(-50, 102, 0))

# .dae
addCharacter('brad2', 'brad')
setPos('brad2', SrVec(50, 102, 0))

addNewBrad('ChrBrad')

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Turn on GPU deformable geometry
scene.command("char brad1 viewer deformableGPU")
scene.command("char brad2 viewer deformableGPU")

# Add Retargetting script
scene.run('Retargetting.py')
'''Find solution to get retargetted animation name'''

output = ''
if output == '':
	autoRetarget('ChrBrad@Guitar01', 'ChrBrad.sk', 'common.sk', '../../../../data/sbm-common/common-sk/retargetMotion/')
	output = 'common.skChrBrad@Guitar01'
	
last = 0
canTime = True
delay = 10
class RetargettingDemo(SBScript):
	def update(self, time):
		global canTime, last, output
		diff = time - last
		if diff >= delay:
			canTime = True
			diff = 0
		if canTime:
			last = time
			canTime = False
			# Play non retargetted and retargetted
			bml.execBML('brad1', '<animation name="ChrBrad@Guitar01"/>')
			bml.execBML('brad2', '<animation name="common.skChrBrad@Guitar01"/>')
			
scene.removeScript('retargettingdemo')
retargettingdemo = RetargettingDemo()
scene.addScript('retargettingdemo', retargettingdemo)
