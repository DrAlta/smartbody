print "|--------------------------------------------|"
print "|        Starting Speech/Face Demo           |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath("script","../../../../data/examples")
scene.addAssetPath("script","../../../../data/examples/functions")
scene.addAssetPath('seq', '../../../../data/sbm-common/scripts')
scene.addAssetPath('seq', '../../../../data/sbm-test/scripts')
scene.addAssetPath('mesh', '../../../../data/mesh')
scene.addAssetPath('mesh', '../../../../data/retarget/mesh')
scene.addAssetPath('audio', '../../../../data/Resources/audio')

# Runs the default viewer for camera
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 152.13, 164.30)
camera.setCenter(0, 141.25, 118.67)

# Add Character script
scene.run('AddCharacter.py')
# Add characters in scene
addCharacter('utah', 'utah')
setPos('utah', SrVec(0, 0, 0))
addCharacter('brad', 'brad')
setPos('brad', SrVec(70, 102, 0))
addCharacter('elder', 'elder')
setPos('elder', SrVec(-70, 102, 0))

# Set camera position
setPawnPos('camera', SrVec(0, -50, 0))

# Turn on GPU deformable geometry for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

# Add Facial Movements script
scene.run('Saccade.py')

# Add Saccade script
scene.run('Saccade.py')
playSaccade('utah', 'talk')
playSaccade('elder', 'listen')
playSaccade('brad', 'think')
