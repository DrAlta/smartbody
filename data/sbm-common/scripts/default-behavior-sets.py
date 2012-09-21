behaviorSetManager = scene.getBehaviorSetManager()

behav = behaviorSetManager.createBehaviorSet("MaleLocomotion")
behav.setScript("BehaviorSetMaleLocomotion.py")

behav = behaviorSetManager.createBehaviorSet("FemaleLocomotion")
behav.setScript("BehaviorSetFemaleLocomotion.py")

behav = behaviorSetManager.createBehaviorSet("Reaching")
behav.setScript("BehaviorSetReaching.py")

behav = behaviorSetManager.createBehaviorSet("MocapReaching")
behav.setScript("BehaviorSetMocapReaching.py")

behav = behaviorSetManager.createBehaviorSet("Gestures")
behav.setScript("BehaviorSetGestures.py")

behav = behaviorSetManager.createBehaviorSet("Jumping")
behav.setScript("BehaviorSetJumping.py")
