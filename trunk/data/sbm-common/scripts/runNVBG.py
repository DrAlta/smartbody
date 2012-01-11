
from NewNVBG.NVBG import *

p = NVBG()
class N(Nvbg):
    #def __init__(self, pcfg_model_fname=None):
     #   p = NVBG()

    def execute(self, character, recipient, messageId, xml):
        bmlstr = p.process_speech(character,recipient,messageId, xml)
        if (len(bmlstr) != 0):
            bml.execBML(character, str(bmlstr))
            print str(bmlstr)
        return True

    def objectEvent(self, character, name, isAnimate, position, velocity, relativePosition, relativeVelocity):
        bmlstr = p.process_events(character, name, isAnimate, position, velocity, relativePosition, relativeVelocity)
        if (len(bmlstr) != 0):
            bml.execXML(character, str(bmlstr))
        return

#    def eventOfInterest(self, character, objectName):
#       bmlstr = p.process_event(character,objectName)
#       bml.execBML(character, str(bmlstr))
#       return True
 

 
Mynvbg = N()
d = scene.getCharacter("utah")
d.setNvbg(Mynvbg)




