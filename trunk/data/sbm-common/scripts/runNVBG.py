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
Mynvbg.createBoolAttribute("rule1", True, False,"Rules", 40, False, False, False, "This is the description of the attribute. This line creates a boolean attribute called 'rule1' that is True by default, will not notify the NVBG object when it is changed, has a priority value of 40, is not read-only, is not locked, and is not hidden.");
  



