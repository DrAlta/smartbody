print "|--------------------------------------------|"
print "|  data/sbm-common/scripts/default-init.py  |"
print "|--------------------------------------------|"

scene.run("default-viewer.py")

### Load data/sbm-common assets
scene.addAssetPath("seq", "sbm-common/scripts")

scene.run("init-common-assets.py")
scene.run("init-common-face.py")
scene.run("init-utah-face.py")

# start the simulation
sim.start()
scene.vhmsg("vrAllCall")
