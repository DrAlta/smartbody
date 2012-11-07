print "|--------------------------------------------|"
print "|          Starting Gestures Demo            |"
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
camera.setEye(-1.81, 181.82, 384.84)
camera.setCenter(-1.81, 120.27, 195.35)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('brad', 'brad')
setPos('brad', SrVec(-60, 102, 0))
addCharacter('utah', 'utah')
setPos('utah', SrVec(145, 0, 0))
addCharacter('brad2', 'brad')
setPos('brad2', SrVec(-145, 102, 0))
addCharacter('utah2', 'utah')
setPos('utah2', SrVec(60, 0, 0))

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Add Blend script
scene.run('Gestures.py')
initGestureMap()

#scene.getCharacter('brad').setBoolAttribute('bmlRequest.autoGestureTransition', True)
#scene.getCharacter('utah').setBoolAttribute('bmlRequest.autoGestureTransition', True)

last = 0
canTime = True
delay = 10
class GesturesDemo(SBScript):
	def update(self, time):
		global canTime, last
		diff = time - last
		if canTime:
			last = time
			canTime = False
		if diff >= delay:
			diff = 0
			canTime = True
		# When time's up, do action
		if canTime:
			# Brad2
			playGesture('brad2', 'SWEEP', 'BOTH_HANDS', 0, 1)
			playGesture('brad2', 'OFFER', 'RIGHT_HAND', 1.5, 2.5)
			playGesture('brad2', 'YOU', 'RIGHT_HAND', 3, 4)
			playGesture('brad2', 'SWEEPRIGHT', 'RIGHT_HAND', 4.5, 5)
			# Brad
			playGesture('brad', 'SWEEP', 'BOTH_HANDS', stroke=0, stroke_end=2.5)
			playGesture('brad', 'OFFER', 'RIGHT_HAND', stroke=3, stroke_end=4)
			playGesture('brad', 'YOU', 'RIGHT_HAND', stroke=5, stroke_end=6)
			playGesture('brad', 'SWEEPRIGHT', 'RIGHT_HAND', stroke=7, stroke_end=8)
			# Utah2
			playGesture('utah2', 'YOU', 'RIGHT_HAND', 0, 1)
			playGesture('utah2', 'RIGHT', 'RIGHT_HAND', 1.5, 2.5)
			playGesture('utah2', 'LEFT', 'LEFT_HAND', 3, 4)
			playGesture('utah2', 'WHY', 'BOTH_HANDS', 4.5, 5.5)
			# Utah
			playGesture('utah', 'YOU', 'RIGHT_HAND', stroke=0, stroke_end=2.5)
			playGesture('utah', 'RIGHT', 'RIGHT_HAND', stroke=3, stroke_end=4.5)
			playGesture('utah', 'LEFT', 'LEFT_HAND', stroke=5.5, stroke_end=6.5)
			playGesture('utah', 'WHY', 'BOTH_HANDS', stroke=7.5, stroke_end=8.5)
			
# Run the update script
scene.removeScript('gesturesdemo')
gesturesdemo = GesturesDemo()
scene.addScript('gesturesdemo', gesturesdemo)