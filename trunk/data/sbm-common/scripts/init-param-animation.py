# locomotion main state
scene.run("stateAllLocomotion.py")
# starting state, starting locomotion with different angle
scene.run("stateAllStarting.py")
# idle turn state, facing adjusting
scene.run("stateAllIdleTurn.py")
# step state, stepping adjusting
scene.run("stateAllStep.py")

# transitions
scene.run("transitions.py")

scene.run("stateSimple.py")