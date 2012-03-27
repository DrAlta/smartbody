"""
Reset NVBG, This script is used for debugging
"""

''' Reset NVBG.py '''
reload(NewNVBG.NVBG)
from NewNVBG.NVBG import *	# why simply reload doesn't work???

''' Reset Pyke '''
from pyke import knowledge_engine
import NewNVBG.nvbg
import NewNVBG.brainstem
nvbg_engine = knowledge_engine.engine(NewNVBG.nvbg)
brainstem_engine = knowledge_engine.engine(NewNVBG.brainstem)
