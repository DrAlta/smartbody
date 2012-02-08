import sys
import os

from NewNVBG.NVBG import *
from xml.etree import ElementTree as ET

p = NVBG("utah")
class N(Nvbg):

        def notifyAction(self, name):
                print "In notifyAction, attribute is " + name
                if (name == "play"):
                        dialogStr = Mynvbg.getAttribute("dialog").getValue()
                        if (dialogStr == ""):
                                print "dialog not selected, play default one"
                                p.speak()
                        else:
                                p.speak(dialogStr)

                if (name == "reset"):
                       scene.run("resetNVBG.py")
                if (name == "test_percept"):
                        p.percept()
		return

	def notifyBool(self, name, val):
                print "In notifyBool, attribute is " + name
                if (name == "enable"):
                        if (val is True):
                                print "enabled!"
                        else:
                                print "disabled!"
		return

	def notifyString(self, name, val):
                print "In notifyString, attribute is " + name
                if (name == "mylist"):
                        print val
		return	

	def execute(self, character, recipient, messageId, xml):
                xmlRoot = ET.XML(xml)
                rootTag = xmlRoot.tag
                if (rootTag == "pml-nvbg"):     # if it's pml-nvbg, then process percepts
                        # for perception, character is the speaker, recipient is the character
                        bmlstr = p.process_percepts(character, recipient, xml)
                        if (len(bmlstr) != 0):
                                bml.execXML(recipient, str(bmlstr))
                else:
                        bmlstr = p.process_speech(character,recipient,messageId, xml)
                        if (len(bmlstr) != 0):
                                bml.execBML(character, str(bmlstr))
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
Mynvbg.createActionAttribute("reset", True, "nvbgs", 20, False, False, False, "Reload pyke")
a = Mynvbg.createStringAttribute("dialog", "", True, "nvbgs", 50, False, False, False, "Dialog")
v = StringVec()
v.append("yes")
v.append("yes this is a good idea")
v.append("no i do not like it")
a.setValidValues(v)
Mynvbg.createActionAttribute("play", True, "nvbgs", 60, False, False, False, "Play the chosen dialog")
Mynvbg.createActionAttribute("test_percept", True, "nvbgs", 70, False, False, False, "Test perception")



 



