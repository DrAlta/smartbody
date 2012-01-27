import sys
import os

from NewNVBG.NVBG import *

p = NVBG()
class N(Nvbg):

	def notify(self, subject):
		if (subject is SBAttribute):
                        attrName = subject.getName()
                        if (name == "enable"):
                                if (subject.getValue()):
                                        print "enabled!"
                                else:
                                        print "disabled!"
		return

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
Mynvbg.createBoolAttribute("enable", True, True, "nvbgs", 10, False, False, False, "Enables or disables NVBG.")
Mynvbg.createActionAttribute("doSomething", True, "nvbgs", 20, False, False, False, "Does something...")
a = Mynvbg.createStringAttribute("mylist", "", True, "nvbgs", 50, False, False, False, "Does something...")
v = StringVec()
v.append("one")
v.append("two")
v.append("three")
v.append("four")
v.append("five")
a.setValidValues(v)


 



