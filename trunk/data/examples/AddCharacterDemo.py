print "|--------------------------------------------|"
print "|         Starting Character Demo            |"
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
camera.setEye(0, 367.58, 574.66)
camera.setCenter(0, 275, 395)
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
setPos('utah', SrVec(75, 0, 250))

# .dae
addCharacter('brad', 'brad')
setPos('brad', SrVec(-75, 102, 250))

# Multiple
addMultipleCharacters('elder', 'elder', 25)