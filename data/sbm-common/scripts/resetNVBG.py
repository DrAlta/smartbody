"""
Reset NVBG, This script is used for debugging
"""

print 'Resetting NVBG...'

''' Reset NVBG.py '''
reload(NewNVBG.NVBG)
from NewNVBG.NVBG import *  

''' Reset Pyke '''
reload(NewNVBG.nvbg)
reload(NewNVBG.brainstem)
nvbg_engine = knowledge_engine.engine(NewNVBG.nvbg)
brainstem_engine = knowledge_engine.engine(NewNVBG.brainstem)

characterNames = scene.getCharacterNames()
numCharacters = len(characterNames)
parser = getScene().getParser()
for i in range(0, numCharacters):
    n = NVBG(characterNames[i], getScene())
    n.setParser(parser)
    scene.getCharacter(characterNames[i]).getNvbg().nvbg = n
    scene.getCharacter(characterNames[i]).getNvbg().clearAttributes()
    scene.getCharacter(characterNames[i]).getNvbg().setupTool()
