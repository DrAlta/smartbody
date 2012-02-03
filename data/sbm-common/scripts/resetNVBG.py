#reset NVBG, it's a hack, need better handle
del p
reload(NewNVBG.NVBG)
# why you still need to import * after reload it ??????
from NewNVBG.NVBG import *
p = NVBG("utah")


# import pyke
import NewNVBG.nvbg
import NewNVBG.brainstem
nvbg_engine = knowledge_engine.engine(NewNVBG.nvbg)
brainstem_engine = knowledge_engine.engine(NewNVBG.brainstem)
