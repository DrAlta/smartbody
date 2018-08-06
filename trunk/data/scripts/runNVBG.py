scene.run("initSmartBodyNVBG.py")

''' Create NVBG, SmartBodyNVBG, Set SmartbodyNVBG '''
characterNames = scene.getCharacterNames()
numCharacters = len(characterNames)
sbNvbgs = []
parser = getScene().getParser()
for i in range(0, numCharacters):
    nvbg = NVBG(characterNames[i], getScene())
    nvbg.setParser(parser)
    nvbg.initializeCharniakParser()
    sbNvbgs.append(SmartBodyNVBG())
    sbNvbgs[i].setNvbg(nvbg)
    scene.getCharacter(characterNames[i]).setNvbg(sbNvbgs[i])
    sbNvbgs[i].setupTool()
