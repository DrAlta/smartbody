scene.run("initSmartBodyNVBG.py")

''' Create NVBG, SmartBodyNVBG, Set SmartbodyNVBG '''
nvbg = NVBG("utah")
sbNvbg = SmartBodyNVBG()
sbNvbg.setNvbg(nvbg)
scene.getCharacter("utah").setNvbg(sbNvbg)
sbNvbg.setupTool()
