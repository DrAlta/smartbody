# mirror functions here... (need to be moved to SBMotion)
scene.command("mirror ChrUtah_Relax001_CrouchProtectHead_right ChrUtah_Relax001_CrouchProtectHead_left")

scene.run("stateAllCrouchProtectHead.py")
scene.run("stateAllCrouchSwayBlock.py")

scene.run("transitionTest.py")


