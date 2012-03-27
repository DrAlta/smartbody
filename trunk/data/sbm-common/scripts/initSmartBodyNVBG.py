import sys
import os
from NewNVBG.NVBG import *
from xml.etree import ElementTree as ET

"""
This class define the NVBG python class that override smartbody c++ one
SmartBodyNVBG serves as an connection to the real NVBG to smartbody
- Steps to setup pNVBG for a character: create NVBG, create SmartBodyNVBG, set NVBG to SmartBodyNVBG, set SmartBodyNVBG to character
- Tool setup is in charge of setting up the attributes that can used inside notifyAction function
"""
class SmartBodyNVBG(Nvbg):
        def setNvbg(self, n):
                self.nvbg = n

        def reset(self):
                nvbgCharName = ""
                if hasattr(self, 'nvbg') is False:
                        return
                if (self.nvbg is not None):
                        nvbgCharName = self.nvbg.characterName
                        del self.nvbg

                scene.run("resetNVBG.py")
                self.nvbg = NVBG(nvbgCharName)

        
        def notifyAction(self, name):
                """
                Override the C++ notifiyAction function.
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
                #print "In notifyAction, attribute is " + name
                if (name == "play"):
                        dialogStr = self.getAttribute("dialog").getValue()
                        if (dialogStr == ""):
                                #print "dialog not selected, play default one"
                                self.nvbg.speak()
                        else:
                                self.nvbg.speak(dialogStr)

                if (name == "reset"):
                       self.reset()
		return

	def notifyBool(self, name, val):
                """
                Override the C++ notifyBool function.
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
                if (name == "enable"):
                        if (val is True):
                                print "enabled!"
                        else:
                                print "disabled!"
		return

	def notifyString(self, name, val):
                """
                Override the C++ notifyString function.
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
                if (name == "mylist"):
                        print val
		return	

        def executeEvent(self, character, messageId, state):
                """
                Override the C++ executeEvent function
                Event Handler for vrX message
                Process vrAgentBML message which indicate the status of bml execution
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
                if (state == "start"):
                        #print "NVBG executeEvent: start executing msg " + messageId
                        return True
                if (state == "end"):
                        #print "NVBG executeEvent: end executing msg " + messageId
                        self.nvbg.reset_feedback_msgId(character, messageId)
                return True
                
	def execute(self, character, recipient, messageId, xml):
                """
                Override the C++ execute function
                Execute all xml vrX messages
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
                xmlRoot = ET.XML(xml)
                rootTag = xmlRoot.tag
                ''' processing PML '''
                if (rootTag == "pml"):
                        bmlstr = self.nvbg.process_percepts(character, xml)
                        if (len(bmlstr) != 0):
                                bml.execXML(character, str(bmlstr))
                else:
                        actElem = ET.XML(xml)
                        bmlElem = actElem.find('bml')
                        fmlElem = actElem.find('fml')
                        ''' processing BML Speech '''
                        if (bmlElem != None):
                                bmlstr = self.nvbg.process_speech(character,recipient,messageId, xml)
                                if (len(bmlstr) != 0):
                                        bml.execBML(character, str(bmlstr))

                        ''' processing FML '''
                        if (fmlElem != None):
                                bmlstr = self.nvbg.process_feedback(character, xml)
                                if (len(bmlstr) != 0):
                                        msgId = bml.execBML(character, str(bmlstr))
                                        print "Keep track of msgId for vrBCFeedback " + msgId        
                                        if (msgId != ""):
                                                self.nvbg.set_feedback_msgId(character, msgId)
                return True

	def objectEvent(self, character, name, isAnimate, position, velocity, relativePosition, relativeVelocity):
                """
                Override the C++ objectEvent function
                Process events from virtual world (SmartBody)
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
		bmlstr = self.nvbg.process_events(character, name, isAnimate, position, velocity, relativePosition, relativeVelocity)
		if (len(bmlstr) != 0):
			bml.execXML(character, str(bmlstr))
                return

        def setupTool(self):
                """
                Debugging options and tools
                """
                if hasattr(self, 'nvbg') is False:
                        return
                
                self.createBoolAttribute("enable", True, True, "nvbgs", 10, False, False, False, "Enables or disables NVBG.")
                self.createActionAttribute("reset", True, "nvbgs", 20, False, False, False, "Reload pyke")
                dialog = self.createStringAttribute("dialog", "", True, "nvbgs", 50, False, False, False, "Dialog")
                dialogVec = StringVec()
                dialogVec.append("yes")
                dialogVec.append("yes this is a good idea")
                dialogVec.append("no i do not like it")
                dialogVec.append("this is a big one but i prefer a small one")
                dialog.setValidValues(dialogVec)
                self.createActionAttribute("play", True, "nvbgs", 60, False, False, False, "Play the chosen dialog")    
