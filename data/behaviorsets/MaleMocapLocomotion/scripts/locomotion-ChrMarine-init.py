#locomotion mirror
mirrorMotion = scene.getMotion("ChrMarine@WalkCircleRt01")
mirrorMotion.mirror("ChrMarine@WalkCircleLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@WalkTightCircleRt01")
mirrorMotion.mirror("ChrMarine@WalkTightCircleLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@StrafeFastRt01")
mirrorMotion.mirror("ChrMarine@StrafeFastLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@RunCircleRt01")
mirrorMotion.mirror("ChrMarine@RunCircleLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@RunTightCircleRt01")
mirrorMotion.mirror("ChrMarine@RunTightCircleLf01", "ChrBackovic.sk")

#idle turn mirror
mirrorMotion = scene.getMotion("ChrMarine@Turn90Rt")
mirrorMotion.mirror("ChrMarine@Turn90Lf", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@Turn180Rt")
mirrorMotion.mirror("ChrMarine@Turn180Lf", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@Turn360Rt01")
mirrorMotion.mirror("ChrMarine@Turn360Lf01", "ChrBackovic.sk")

#starting mirror
mirrorMotion = scene.getMotion("ChrMarine@Idle01_ToWalk01")
mirrorMotion.mirror("ChrMarine@Idle01_ToWalkLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@Idle01_ToWalk01_Turn90Rt01")
mirrorMotion.mirror("ChrMarine@Idle01_ToWalk01_Turn90Lf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@Idle01_ToWalk01_Turn180Rt01")
mirrorMotion.mirror("ChrMarine@Idle01_ToWalk01_Turn180Lf01", "ChrBackovic.sk")

#step mirror
mirrorMotion = scene.getMotion("ChrMarine@Idle01_StepBackwardsRt01")
mirrorMotion.mirror("ChrMarine@Idle01_StepBackwardsLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@Idle01_StepForwardRt01")
mirrorMotion.mirror("ChrMarine@Idle01_StepForwardLf01", "ChrBackovic.sk")
mirrorMotion = scene.getMotion("ChrMarine@Idle01_StepSidewaysRt01")
mirrorMotion.mirror("ChrMarine@Idle01_StepSidewaysLf01", "ChrBackovic.sk")

scene.run("locomotion-ChrMarine-state-Locomotion.py")
scene.run("locomotion-ChrMarine-state-IdleTurn.py")
scene.run("locomotion-ChrMarine-state-StartingLeft.py")
scene.run("locomotion-ChrMarine-state-StartingRight.py")
scene.run("locomotion-ChrMarine-state-Step.py")

