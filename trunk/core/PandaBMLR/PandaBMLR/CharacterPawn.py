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
	
		if (not self.IsValid()):
			print("Error creating character " + name)
			return

		if (self.Scene.Characters.has_key(charID) and charID >= 0):
			print("Error creating character, it already exists in scene: " + name) 
			return
	
		self.SetID(charID)
		self.SetRegistered(False)
		
		self.CharClass = charClass
		self.__SpeechArgs = ""
		self.__BubbleMan = BubbleManager(self)
		self.__TypeStr = "CharacterPawn"		
		self.__TypeSbm = "char"
		self.__BmlQueue = []
		self.__boneBusMap = {}
		self.__boneBusRotTimes = {}
		self.__boneBusPosTimes = {}
		self.__visemeMap = {}
		self.__morphTargetMap = {}
		self.__morphTargetCounter = 0
		self.InitSkeleton()
		
		
		taskMgr.add(self.__StepBML, "Char" + self.GetName() + "_BmlLoop")
		
		print("Character '" + self.GetName() +  "' loaded in %2.3f sec" % (time.time() - t))
		# add morph target map here:
		self.__morphTargetMap["blink"] = "BlendShapes.15"
		#self.__morphTargetMap["D"] = "BlendShapes.1"
		#self.__morphTargetMap["EE"] = "BlendShapes.2"
		#self.__morphTargetMap["Er"] = "BlendShapes.3"
		#self.__morphTargetMap["f"] = "BlendShapes.4"
		#self.__morphTargetMap["j"] = "BlendShapes.5"
		#self.__morphTargetMap["KG"] = "BlendShapes.6"
		#self.__morphTargetMap["Ih"] = "BlendShapes.7"
		
	def __StepBML(self, task = None):
		""" Checks if there is any BML in the queue and sends it out to SmartBody if we are connected """
		
		if (self.Scene.Connected and self.IsRegistered()):
			while(len(self.__BmlQueue) > 0):
				msg = self.__BmlQueue.pop(0)
				# TODO USE VRSPEAK INSTEAD OF TEST BML
				cleanmsg = msg.replace('"', '\\"')
				self.Scene.SendSbmCommand("python bml.execBML(\"" + self.GetName() + "\", \"" + cleanmsg + "\")")
				
		if (task != None):
			return task.cont
		
		
	def BML(self, bmlString):
		""" Adds a block of BML to the BML queue. Will be sent out once the charater registers with SmartBody
			or right away if the character is already registered """
		
		if (not self.IsValid()):
			print("Trying to send BML for an invalid CharacterPawn: " + self.GetName())
			return
	
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
		
		self.Unregister()
		Pawn.Destroy(self)
		self.Scene.Characters.pop(self.GetCharID())
		self.__CharID = -1
		
	def GetCharID(self):
		""" Gets the ID of the character """
		
		return self.__CharID

	def GetCharClass(self):
		""" Gets the class of the character """
		return self.CharClass
	
	def FindJoint(self, jointID):
		""" Looks up a joint NodePath by ID """
		
		return self.__JointMap.get(jointID)
	
	def Unregister(self):
		""" Sets the character to a disconnected state, if a smartbody client disconnects """
		
		Pawn.Unregister(self)
		# Resets all of the bones to the original T-Pose
		for joint in self.__JointMap.values():
			joint.setHpr(0, 0, 0)
	
	def JointNameToID(self, jointName):
		""" Translates a joint name to joint ID"""
		
		for bonePair in self.Class.Bones.items():
			if (bonePair[1] == jointName):
				return bonePair[0]

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
			self.__JointMap[jointName] = jointNP
			try:
				if PandaSystem.getMinorVersion() > 5:
					jointNP.setMat(joint.getDefaultValue())
				else:
					jointNP.setMat(joint.getInitialValue())

			except TypeError:
				print("Initial value on joint " + jointName + " not a matrix. Assuming blendShape")
				jointNP.setX(joint.getDefaultValue())
			return jointNP
		else:
			print("Joint control request for '" + jointName + "' FAILED!\n")
			return None;
	
	def SetJointQuat(self, time, boneBusjointID, quat):
		""" Rotates a joint by quanternions """
		# make sure the timing of the joint data is for a future time
		val = self.__boneBusRotTimes.get(boneBusjointID, -100)
		if val == -100:
			return
		if  val > time:
			return
		self.__boneBusRotTimes[boneBusjointID] = time
		# get the mapped joint name
		mappedName = self.GetBoneBusMap(boneBusjointID)
		# now get the corresponding Panda character joint name
		pandaJointID = self.JointNameToID(mappedName)
		
		joint = self.FindJoint(pandaJointID)
		
		if (joint != None):
			if (LERP_ANIMS):
				lerp = LerpPosQuatInterval(joint, ANIM_SPEED, joint.getPos(), quat)						
				lerp.start()
			else:
				joint.setQuat(quat)
						
	def SetJointPos(self, time, boneBusjointID, vec3):
		""" Sets the position of a joint. Input is relative to the start position of that joint  """
		# make sure the timing of the joint data is for a future time
		val = self.__boneBusPosTimes.get(boneBusjointID, -100)
		if val == -100:
			return
		if  val > time:
			return
		self.__boneBusPosTimes[boneBusjointID] = time
		# get the mapped joint name
		mappedName = self.GetBoneBusMap(boneBusjointID)
		# now get the corresponding Panda character joint name
		pandaJointID = self.JointNameToID(mappedName)
		
		if (round(vec3.getX(), 3) == 0 and round(vec3.getY(), 3) == 0 and round(vec3.getZ(), 3) == 0): # No movement
			return
		
		joint = self.FindJoint(pandaJointID)
		
		if (joint != None):
			joint.setPos(vec3)
			
	def SetViseme(self, visemeId, weight, blend):
		# get the mapped viseme name
		mappedViseme = self.GetVisemeMap(visemeId)
		
		try:
			morphTarget = self.__morphTargetMap[mappedViseme]
			#print "SetViseme " + mappedViseme  + " morph=" + morphTarget 
			# get the joint id
			# pandaJointID = self.JointNameToID(morphTarget)
			#print " jointID=" + pandaJointID 
			# joint = self.FindJoint(pandaJointID)
			joint = self.FindJoint(morphTarget)
		
			if joint is not None:
				joint.setX(weight)
		except:
			print "Cannot find morph target for viseme " + mappedViseme + " on character " + self.GetName()
			# add a dummy value
			if mappedViseme not in self.__morphTargetMap:
				self.__morphTargetMap[mappedViseme] = "unknown" + str(self.__morphTargetCounter)
				self.__morphTargetCounter = self.__morphTargetCounter + 1
			
		
	
		
	def RegisterInit(self):
		""" Registers the character with a connected SmartBody client so
			that the character is initialized in SB"""
			
		if (not self.IsValid()):
			print("Trying to register an invalid CharacterPawn: " + self.GetName())
			return
		
		if (not self.IsRegistered()):
			print "Now sending command to register character " + self.GetName() + " with SmartBody..."
			self.Scene.SendSbmCommand("python mychar = scene.createCharacter(\"" + self.GetName() + "\", \"" + self.GetCharClass() + "\")")
			self.Scene.SendSbmCommand("python myskel = scene.createSkeleton(\"common.sk\")")
			self.Scene.SendSbmCommand("python mychar.setSkeleton(myskel)")
			self.Scene.SendSbmCommand("python mychar.createStandardControllers()")
	
			self.Scene.SendSbmCommand("python mychar.setVoice(\"text\")")
			self.Scene.SendSbmCommand("python mychar.setVoiceCode(\"" + self.GetName() + "\")")
			print "About to setup character " + self.GetName()
			self.Scene.SendSbmCommand("python characterSetup(\"" + self.GetName() + "\")")
			print "Finished setting up character " + self.GetName()
			
			self.SetRegistered(True)
			self.RegisterPosHpr()
			
	
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
	
	def AddBoneBusMap(self, boneName, boneId):
		""" mapping between the bone name and the bone id """
		self.__boneBusMap[boneId] = boneName
		# also seed the timings
		self.__boneBusRotTimes[boneId] = -1
		self.__boneBusPosTimes[boneId] = -1
		
	def GetBoneBusMap(self, boneId):
		""" retrieves the mapping between the bone name and the bone id """
		x = self.__boneBusMap[boneId]
		return x
		
	def AddVisemeMap(self, visemeName, visemeId):
		""" mapping between the viseme name and the viseme id """
		self.__visemeMap[visemeId] = visemeName
		
	def GetVisemeMap(self, visemeId):
		""" retrieves the mapping between the viseme name and the viseme id """
		try:
			x = self.__visemeMap[visemeId]
			return x		
		except:
			return ""
			
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
