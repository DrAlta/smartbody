print "|--------------------------------------------|"
print "|        Starting Retargetting Demo          |"
print "|--------------------------------------------|"
'''FEET SLIDING STUFF'''

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
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(10000)
camera.setNearPlane(1)
camera.setAspectRatio(1.02632)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(-50, 0, 0))

# .dae
addCharacter('brad', 'brad')
setPos('brad', SrVec(50, 102, 0))

# Add Retargetting script
scene.run('Retargetting.py')
'''Find solution to get retargetted animation name'''

output = ''
if output == '':
	autoRetarget('LHandOnHip_Arms_GestureWhy', 'common.sk', 'test_utah.sk', '../../../../data/sbm-common/common-sk/retargetMotion/')
	output = getOutput()
	
last = 0
canTime = True
delay = 5
class Retargetting(SBScript):
	def update(self, time):
		global canTime, last, output
		if canTime:
			last = time
			canTime = False
		diff = time - last
		if diff >= delay:
			canTime = True
			diff = 0
		if canTime:
			bml.execBML('brad', '<animation name="LHandOnHip_Arms_GestureWhy"/>')
		'''
		#print diff
		if 5 < diff < 5.1:
			bml.execBML('brad','<animation name="LHandOnHip_Arms_GestureWhy"/>')
		if 10 < diff < 10.1: 
			if output == '':
				autoRetarget('LHandOnHip_Arms_GestureWhy', 'test_utah.sk', 'common.sk', '../../../../data/sbm-common/common-sk/retargetMotion/')
				output = getOutput()
		if 12 < diff < 12.1:
			bml.execBML('utah','<animation name="' + output + '"/>')
		if diff > 15:
			canTime = True
			diff = 0
			#setPos('brad', SrVec(50, -102, 0))
		'''
scene.removeScript('retargetting')
retargetting = Retargetting()
scene.addScript('retargetting', retargetting)
