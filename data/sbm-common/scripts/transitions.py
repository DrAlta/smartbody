stateManager = scene.getStateManager()

transition1 = stateManager.createTransition("allStartingLeft", "allLocomotion")
transition1.setEaseInInterval("ChrUtah_Meander01", 0.4, 0.78)
transition1.addEaseOutInterval("ChrUtah_StopToWalkLf01", 0.54, 0.83)

transition2 = stateManager.createTransition("allStartingRight", "allLocomotion")
transition2.setEaseInInterval("ChrUtah_Meander01", 1.1, 1.5)
transition2.addEaseOutInterval("ChrUtah_StopToWalkRt01", 0.54, 0.83)
