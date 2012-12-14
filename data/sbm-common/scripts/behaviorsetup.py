

scene.addAssetPath("script", os.path.relpath("../../../../data/sbm-common/scripts",scene.getMediaPath()))
scene.addAssetPath("script", os.path.relpath("../../../../data/sbm-common/scripts/behaviorsets",scene.getMediaPath()))
scene.addAssetPath("motion", os.path.relpath("../../../../data/retarget",scene.getMediaPath()))

scene.run("default-behavior-sets.py")

getCamera().reset()