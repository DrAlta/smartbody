"""
Reset NVBG, This script is used for debugging
"""

print 'Resetting NVBG...'

''' Reset NVBG.py & PyKE'''
reload(Cerebella.NVBG)
from Cerebella.NVBG import *
from Cerebella import UniversalFactUtil

"""
reload(Cerebella.nvbg)
reload(Cerebella.brainstem)
nvbg_engine = knowledge_engine.engine(Cerebella.nvbg)
brainstem_engine = knowledge_engine.engine(Cerebella.brainstem)
"""

characterNames = scene.getCharacterNames()
numCharacters = len(characterNames)
parser = getScene().getParser()
for i in range(0, numCharacters):
    n = NVBG(characterNames[i], getScene())
    n.setParser(parser)
    scene.getCharacter(characterNames[i]).getNvbg().nvbg = n
    scene.getCharacter(characterNames[i]).getNvbg().resetTool()
