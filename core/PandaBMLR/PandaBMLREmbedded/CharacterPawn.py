# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # 
# #  Panda BML Realizer 	http://cadia.ru.is/projects/bmlr/
# #
# #  Bjarni Thor Arnason 	bjarnia@gmail.com
# #  Aegir Thorsteinsson 	aegirth@gmail.com
# #
# #  Copyright(c)2008 Center for Analysis and Design of Intelligent Agents
# #                   Reykjavik University
# #                   All rights reserved
# #
# #                   http://cadia.ru.is/
# #
# #  Redistribution and use in source and binary forms, with or without
# #  modification, is permitted provided that the following conditions 
# #  are met:
# #
# #  - Redistributions of source code must retain the above copyright notice,
# #    this list of conditions and the following disclaimer.
# #
# #  - Redistributions in binary form must reproduce the above copyright 
# #    notice, this list of conditions and the following disclaimer in the 
# #    documentation and#or other materials provided with the distribution.
# #
# #  - Neither the name of its copyright holders nor the names of its 
# #    contributors may be used to endorse or promote products derived from 
# #    this software without specific prior written permission.
# #
# #  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# #  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# #  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
# #  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
# #  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# #  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
# #  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
# #  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# #  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# #  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
# #  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #


# Python base imports
import sys, os, time
from string import * 
from math import *
from random import choice

# Panda3D imports
from pandac.PandaModules import *
from direct.interval.LerpInterval import *
from direct.gui.OnscreenText import OnscreenText

# PandaBMLR imports
from Pawn import *
from Converter import *


LERP_ANIMS = False
ANIM_SPEED = 0.1 # if LERP_ANIMS == True

class CharacterPawn(Pawn):
	""" The character class. Represent an humanoid character in smartbody/bmlr world. """
	
	def __init__(self, bmlr, name, charClass, charID = -1, registered = False):
		""" Initializes a CharacterPawn and puts it in the scene """
		
		t = time.time()
		Pawn.__init__(self, bmlr, name, charClass, GEOMTYPE_CHARACTER, registered)
	
		if (self.Scene.Characters.has_key(charID) and charID >= 0):
			print("Error creating character, it already exists in scene: " + name) 
			return
	
		self.SetID(charID)
		self.SetRegistered(True)
		
		self.CharClass = charClass
		self.__SpeechArgs = ""
		self.__BubbleMan = BubbleManager(self)
		self.__TypeStr = "CharacterPawn"		
		self.__TypeSbm = "char"
		self.__BmlQueue = []
		self.InitSkeleton()
		
		
		taskMgr.add(self.__StepBML, "Char" + self.GetName() + "_BmlLoop")
		
		print("Character '" + self.GetName() +  "' loaded in %2.3f sec" % (time.time() - t))
		
		
		print "Loading local SmartBody character: [" + self.GetName() + "], [" + self.GetCharClass() + "]"
		
		mychar = bmlr.sbscene.createCharacter(self.GetName(), self.GetCharClass())
		myskel = bmlr.sbscene.createSkeleton("common.sk")
		mychar.setSkeleton(myskel)
		mychar.createStandardControllers()
		mychar.setVoice("remote")
		mychar.setVoiceCode("Microsoft|Anna")
		print "About to setup the character..."
		bmlr.sbscene.command("python characterSetup(\"" + self.GetName() + "\")")
		
		self.RegisterPosHpr()
	
		
	def __StepBML(self, task = None):
		""" Checks if there is any BML in the queue and sends it out to SmartBody if we are connected """
		
		if (True): #self.Scene.Connected and self.IsRegistered()):
			while(len(self.__BmlQueue) > 0):
				msg = self.__BmlQueue.pop(0)
				cleanmsg = msg.replace('"', '\\"')
				self.Scene.SendSbmCommand("python bml.execBML('" + self.GetName() + "', '" + cleanmsg + "')")
				
		if (task != None):
			return task.cont
		
		
	def BML(self, bmlString):
		""" Adds a block of BML to the BML queue. Will be sent out once the charater registers with SmartBody
			or right away if the character is already registered """
			
		self.__BmlQueue.append(bmlString)
	
	def SetSpeechArgs(self, args):
		""" Set arguments specific to this character that are sent to a speech synthesizer """
		
		self.__SpeechArgs = args
		
	def SetID(self, id):
		""" Set the ID of the character. Do not call manually! """
		
		self.__CharID = int(id)
		if (id >= 0):
			self.Scene.Characters[id] = self
		
	def InitSkeleton(self):
		""" Initiates the skeleton for a character """
		
		if (self.Class.SkeletonType == 1):
			# Retrieve the character node to get access to the skeleton	
			charNode = self.ModelNode.find("**/+Character").node()
		
			# Retrieve the joints from the skeleton
			self.__Joints = charNode.getBundle(0)
			self.__JointMap = {}
				
			# Now we expose all of the joints to create a PandaNode hiearcy for easier accessability
			self.ExposeAllJoints()
	
	def Destroy(self):
		""" Completely removes the character geometry and association with the scene and smartbody """
		
		Pawn.Destroy(self)
		self.Scene.Characters.pop(self.GetCharID())
		self.__CharID = -1
		
	def GetCharID(self):
		""" Gets the ID of the character """
		
		return self.__CharID

	def GetCharClass(self):
		""" Gets the class of the character """
		return self.CharClass
	
	def FindJoint(self, name):
		""" Looks up a joint NodePath by name """
		
		return self.__JointMap.get(name)
			
	def ListAllJoints(self):
		""" Prints out the joint hierarchy for the character """
		
		for i in range(0, self.__Joints.getNumChildren()):
			self.__ListJoint(0, self.ModelNode, self.__Joints.getChild(i))
			
	def __ListJoint(self, level, parent, bundle):
		""" Recursive subroutine for ListAllJoints """
		
		str = ""
		for i in range(0, level):
			str += "   "
		print(str + bundle.getName())
		
		for i in range(0, bundle.getNumChildren()):
			self.__ListJoints(level+1, parent, bundle.getChild(i))
	
	def ExposeAllJoints(self):
		""" Calls self.ControlJoint() on all of the joints in the skeleton """
		
		for i in range(0, self.__Joints.getNumChildren()):
			self.__ExposeJoint(0, self.ModelNode, self.__Joints.getChild(i))

	def __ExposeJoint(self, level, parent, bundle):
		""" Recursive subroutine for ExposeAllJoints """
		
		if (level > 0):
			parent = self.ControlJoint(bundle.getName(), parent)
			
		for i in range(0, bundle.getNumChildren()):
			self.__ExposeJoint(level+1, parent, bundle.getChild(i))
		
	def SetRotQuat(self, quat):
		""" Rotates the character in quanternions """
		
		self.setHpr(0, 0, 0)
		self.setQuat(quat)
		
	def ControlJoint(self, jointName, parent):
		""" Exposes a joint to a NodePath so it can be controlled manually """
		
		jointNP = parent.attachNewNode(jointName)
	
		if (self.__Joints.controlJoint(jointName, jointNP.node())):
			joint = self.__Joints.findChild(jointName)
			
			if PandaSystem.getMinorVersion() > 5:
				jointNP.setMat(joint.getDefaultValue())
			else:
				jointNP.setMat(joint.getInitialValue())
				
			
			self.__JointMap[jointName] = jointNP
			return jointNP
		else:
			print("Joint control request for '" << _jointName << "' FAILED!\n")
			return None;
			
	
	def Speak(self, text, duration = 0, type = "bubble", bubbleID = -1):
		""" Makes the character act out a speech BML element request """
		
		if (type == "bubble"):
			self.__BubbleMan.Add(text, duration, bubbleID)
			
			if (ConfigVariableBool("SpeechOn", False).getValue()):
				""" Use a speech synth """
				cmd = "start /BELOWNORMAL /MIN /D" + ConfigVariableString("SpeechPath", "").getValue() + " " + ConfigVariableString("SpeechPath", "").getValue() + ConfigVariableString("SpeechBin", "").getValue() + " " + ConfigVariableString("SpeechArgs", "").getValue()
				cmd += " " + self.__SpeechArgs + " " + text
				os.system(cmd)
				
				
		elif(type == "speech"):
			print("Speech is not implemented.")
	

			
class BubbleManager:
	""" Handles the spawning and positioning of chat bubbles for a CharacterPawn instance """
	
	def __init__(self, character, OriginZ = 40, FloorZ = -50):
		""" Initializes a new bubble manager """
		
		self.TotalHeight = 0
		self.Character = character
		self.OriginZ = OriginZ
		self.FloorZ = FloorZ
		self.NextZ = self.OriginZ
		self.Active = 0
		self.Bubbles = {}
	
	def Add(self, text, duration = 0, bubbleID = -1):
		""" Adds a new bubble to the bubblemanager """
		
		if (bubbleID == -1 or not self.Bubbles.has_key(bubbleID)):
			z = self.NextZ
			
			cb = Bubble(text, duration, self, z, bubbleID = bubbleID)
			self.TotalHeight += cb.Height +5
			self.Active += 1
			
			nextZ = self.OriginZ - self.TotalHeight 
	
			if (nextZ < self.FloorZ ):
				self.Reset()
				
			self.NextZ = nextZ
			
			if (bubbleID >= 0):
				self.Bubbles[bubbleID] = cb
		else:
			self.Bubbles[bubbleID].AppendText(text, duration)
		
	def Reset(self):
		""" Reset the accumulated height of the bubblemanager. Do not call manually. """
		
		self.TotalHeight = 0			
		self.NextZ = self.OriginZ
		
	def AddHeight(self, height):
		""" Manually add height to the bubblemanager. Do not call manually. """
		
		self.TotalHeight += height
		self.NextZ -= height
		
	
	def OnExpire(self, height):
		""" Called when a bubble expires """
		
		self.Active -= 1
		
		if (self.Active <= 0): ## Reset if there are no bubbles being displayed
			self.Active = 0 # Just in case!
			self.Reset()
			
def CalcHeight(np):
	""" Function to calculate the height of a nodepath """
	
	pMin = Point3()
	pMax = Point3()
	np.calcTightBounds(pMin, pMax)
	return pMax.getZ() - pMin.getZ()
	
class Bubble:
	""" Represents a single chat bubble, managed and positioned by the bubbleMan(ager) """
	
	def __init__ (self, text, duration, bubbleMan, zPos, bubbleID = -1):
		""" Creates a new chat bubble and places it in the scene """		
		
		self.BubbleMan = bubbleMan
		self.Character = bubbleMan.Character
		self.BubbleID = bubbleID
		
		self.BubbleText = TextNode(self.Character.GetName() + "_Bubble");
		
		text = self.Character.GetName() + ":\n" + text
	
		self.BubbleText.setFrameColor(0, 0, 0, 1)
		self.BubbleText.setFrameAsMargin(0.4, 0.4, 0.1, 0.1)
		self.BubbleText.setFrameLineWidth(4)
		self.BubbleText.setFrameCorners(False)
		
		self.BubbleText.setCardColor(1, 1, 1, 0.5)
		self.BubbleText.setCardAsMargin(0.4, 0.4, 0.1, 0.1)
		self.BubbleText.setCardDecal(True)
		
		self.BubbleText.setWordwrap(10)		
		self.BubbleText.setAlign(TextNode.ALeft)
		self.BubbleText.setTextColor(0, 0, 0, 1)
		self.BubbleText.setText(text)
		
		self.textNP = self.Character.attachNewNode(self.BubbleText)
		self.textNP.setPos(25, ConfigVariableDouble("ChatBubbleOffsetY", 20).getValue(), zPos + ConfigVariableDouble("ChatBubbleOffsetZ", 0).getValue())
		self.textNP.setScale(ConfigVariableDouble("ChatBubbleScale", 4).getValue())
		self.textNP.setBillboardPointEye()
		self.textNP.setDepthTest(False)
		self.textNP.setLightOff()
		self.textNP.setShaderOff()
		
		if (duration == 0):
			duration = self.CalcDuration(text)
		
		self.RemainingTime = duration
		
		self.Height = CalcHeight(self.textNP)	
		
		taskMgr.add(self.Expire, "BubbleExpire")

	def CalcDuration(self, text):
		""" Calculates estimated duration of the text to stay on the scren based on the length """
		
		return (len(text) / 12.0) + 0.5
	
	def AppendText(self, text, duration = 0):
		""" Adds more text to the SAME chat balloon """
		
		if (duration == 0):
			duration = self.CalcDuration(text)
			
		self.RemainingTime += duration
		
		self.BubbleText.appendText(text)
		
		prev = self.Height
		self.Height = CalcHeight(self.textNP)
		self.BubbleMan.AddHeight(self.Height - prev)
		
	def Expire(self, task = None):
		""" Removes the bubble once its lifetime has expired """
		
		if (self.RemainingTime < task.time):			
			self.BubbleMan.OnExpire(self.Height)
			self.textNP.removeNode()
			if (self.BubbleID >= 0):
				self.BubbleMan.Bubbles.pop(self.BubbleID)
		else:
			return task.cont
