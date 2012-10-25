print "|--------------------------------------------|"
print "|         Starting initialization            |"
print "|--------------------------------------------|"
# Runs the default viewer for camera
scene.run("default-viewer.py")

# Add asset paths
scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.addAssetPath("seq", "../../../../data/sbm-test/scripts")
scene.addAssetPath("mesh", "../../../../data/mesh")
scene.addAssetPath("mesh", "../../../../data/retarget/mesh")
scene.addAssetPath("audio", "../../../../data/Resources/audio")

# Add Character script
scene.run("AddCharacter.py")
# Add characters in scene
addCharacter('brad', 'brad', True)
addMultipleCharacters('utah', 'utah', 5)
addPawn('apple', 'sphere')

# Add Locomotion script
scene.run("Locomotion.py")

# Add Reach script
scene.run("Reach.py")

# Add Speech script
scene.run("Speech.py")

# Add Gaze script
scene.run("Gaze.py")

# Add Gestures script
scene.run("Gestures.py")

# Add Facial Movement script
scene.run("FacialMovements.py")

# Add Retargetting script
scene.run("Retargetting.py")

# Add Physics script
scene.run("Physics.py")