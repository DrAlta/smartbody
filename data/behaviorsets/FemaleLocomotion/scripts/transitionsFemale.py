def femaleTransitionSetup(prefix, statePreFix):
	#mirrorMotion = scene.getMotion("ChrHarmony@Idle01_ToWalkRt01")
	#mirrorMotion.mirror("ChrHarmony@Idle01_ToWalkLf01", "ChrHarmony.sk")

	stateManager = scene.getStateManager()

	transition1 = stateManager.createTransition(statePreFix + "StartingLeft", statePreFix + "Locomotion")
	transition1.setEaseInInterval(prefix + "ChrHarmony@Walk01", 0.38, 1.06)
	transition1.addEaseOutInterval(prefix + "ChrHarmony@Idle01_ToWalkLf01", 0.94, 1.65)

	transition2 = stateManager.createTransition(statePreFix + "StartingRight", statePreFix + "Locomotion")
	transition1.setEaseInInterval(prefix + "ChrHarmony@Walk01", 1.06, 1.70)
	transition1.addEaseOutInterval(prefix + "ChrHarmony@Idle01_ToWalkRt01",  0.94, 1.65)

