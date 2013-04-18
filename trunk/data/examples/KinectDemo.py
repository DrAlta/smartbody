print "|--------------------------------------------|"
print "|        Starting Kinect Demo                |"
print "|--------------------------------------------|"
# Add asset paths
scene.addAssetPath("script", 'sbm-common/scripts')
scene.addAssetPath('seq', 'sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 166, 185)
camera.setCenter(0, 92, 0)

scene.run("init-common-assets.py")
scene.run("init-common-face.py")

utah = scene.createCharacter("utah", "")
utahSkeleton = scene.createSkeleton("test_utah.sk")
utah.setSkeleton(utahSkeleton)
utah.setFaceDefinition(defaultFace)
utahPos = SrVec(0, 0, 0)
utah.setPosition(utahPos)
utahHPR = SrVec(0, 0, 0)
utah.setHPR(utahHPR)
utah.setVoice("remote")
utah.setVoiceCode("Festival_voice_rab_diphone")
utah.createStandardControllers()
utah.setStringAttribute("deformableMesh", "utah")
scene.setBoolAttribute("internalAudio", True)

sim.start()
#bml.execBML('utah', '<body posture="ChrUtah_Idle003"/>')
#bml.execBML('utah', '<saccade mode="listen"/>')
sim.resume()

