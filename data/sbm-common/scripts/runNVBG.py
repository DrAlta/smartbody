import sys
import os

from NewNVBG.NVBG import *

p = NVBG()
class N(Nvbg):

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

Mynvbg = N()
d = scene.getCharacter("utah")
d.setNvbg(Mynvbg)
Mynvbg.setBoolAttribute("rule1", True)
  



