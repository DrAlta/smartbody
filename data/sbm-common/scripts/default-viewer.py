viewer = getViewer()
viewer.show()

camera = getCamera()
camera.setEye(0, 166, 185)
camera.setCenter(0, 92, 0)

cameraPawn = scene.createPawn("camera")
cameraPos = SrVec(0, 166, 185)
cameraPawn.setPosition(cameraPos)

#light0 = scene.createPawn("light0")
#light0Pos = SrVec(100, 250, 400)
#light0.setPosition(light0Pos)

#light1 = scene.createPawn("light1")
#light1Pos = SrVec(100, 500, -200)
#light1.setPosition(light1Pos)