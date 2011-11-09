stateManager = scene.getStateManager()

transition1 = stateManager.createTransition("allStartingLeft", "allLocomotion")
transition1.addCorrespondancePoint("ChrUtah_StopToWalkLf01", "ChrUtah_Meander01", 0.54, 0.83, 0.4, 0.78)
transition2 = stateManager.createTransition("allStartingRight", "allLocomotion")
transition2.addCorrespondancePoint("ChrUtah_StopToWalkRt01", "ChrUtah_Meander01", 0.54, 0.83, 1.1, 1.5)

