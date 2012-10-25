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

# Add Character script
scene.run('AddCharacter.py')
# Add character to scene
addCharacter('brad', 'brad')
setPos('brad', SrVec(0, 102, 0))

# Add Gestures script
scene.run('Gestures.py')
initGestureMap()

#print scene.getGestureMapManager().getGestureMap('brad').getGestureByIndex(3)