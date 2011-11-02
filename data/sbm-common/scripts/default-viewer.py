viewer = getViewer()
viewer.show()

camera = getCamera()
camera.setEye(0, 166, 185)
camera.setCenter(0, 92, 0)

cameraPawn = createPawn("camera")
cameraPos = SrVec(0, 166, 185)
cameraPawn.setPosition(cameraPos)
