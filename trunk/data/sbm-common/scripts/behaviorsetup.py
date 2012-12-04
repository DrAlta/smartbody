scene.addAssetPath("script", "../../../../data/sbm-common/scripts")
scene.addAssetPath("script", "../../../../data/sbm-common/scripts/behaviorsets")
scene.addAssetPath("motion", "../../../../data/retarget")

scene.run("default-behavior-sets.py")

getCamera().reset()