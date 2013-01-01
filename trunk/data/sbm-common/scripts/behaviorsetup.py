

scene.addAssetPath("mesh", "mesh")

scene.addAssetPath("script", os.path.relpath("sbm-common/scripts",scene.getMediaPath()))
scene.addAssetPath("script", os.path.relpath("sbm-common/scripts/behaviorsets",scene.getMediaPath()))
scene.addAssetPath("motion", os.path.relpath("retarget",scene.getMediaPath()))

scene.run("default-behavior-sets.py")

getCamera().reset()