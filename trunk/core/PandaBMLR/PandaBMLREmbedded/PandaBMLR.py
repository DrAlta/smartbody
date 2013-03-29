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
from sys import *
from os import system
import atexit

# Panda3D imports
from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject

# PandaBMLR imports
from Scene import Scene
from ClassCacher import ClassCacher
import Converter

import sys
sys.path.append("..\\core\smartbody\\Python26\\Lib\site-packages")
import SmartBody


class PandaBMLR(DirectObject):
	
	def __init__(self):
		""" Initializes PandaBMLR """
		
		print("**********************************")
		print("*          BML Realizer          *")
		print("*      Reykjavik University      *")
		print("*             CADIA              *")
		print("**********************************")
		print("")
		
		self.__Scene = Scene(self)
		self.__PlatesOn = True
		print "Starting embedded SmartBody..."
		self.sbscene = SmartBody.getScene()
		self.sbscene.startFileLogging("./smartbody.log")
		self.Camera = None
		self.SbmStart(True)
		
		# Keybindings
		self.accept("escape", exit)
		self.accept("f1", self.TogglePlates)		
		self.accept("f11", base.toggleWireframe)
		self.accept("f12", base.screenshot)	
		
		taskMgr.add(self.Step, "BMLR_MainLoop")
		
		
	def PlatesOn(self):
		""" Whether or not nameplates are shown """
		
		return self.__PlatesOn
	
	def TogglePlates(self):
		""" Toggles visibility of all nameplates """
		
		if (self.__PlatesOn):
			messenger.send("HideAllPlates")
		else:
			messenger.send("ShowAllPlates")
			
		self.__PlatesOn = not self.__PlatesOn
		
	def SbmCommand(self, cmd):
		""" Sends a raw command to SmartBody """
		print "About to run: " + cmd
		self.sbscene.command(cmd)
		
	def SbmStart(self, auto = False):
		""" Starts SmartBody with paremeters from config file. Windows only """
		
		print "About to load embedded script in SmartBody: " 
		self.sbscene.setMediaPath("../../data")
		self.sbscene.addAssetPath("script", "sbm-common/scripts")
		print "About to run default-init-empty.py"
		ret = self.sbscene.run("default-init-empty.py")
		if ret is False:
			print "Problem running script: " + initSeq
		self.__Scene.OnConnect()
		
		
		
		
	def __SbmOnQuit(self):
		""" Exit SmartBody """
		
		self.SbmCommand("q")
		
	def GetPawn(self, pawnName):
		""" Finds a Pawn with name pawnName. Returns CharacterPawns as well. """
		
		return self.__Scene.Pawns.get(pawnName)
				
	def GetScene(self):
		""" Returns a reference to the Scene object """
		
		return self.__Scene
	
		
	def Step(self, task = None):
		""" Frame by frame updates """
		
		self.sbscene.getSimulationManager().setTime(task.time)
		self.sbscene.update()
		#print "SmartBody updated at " + str(self.__simTime)
		
		# update all characters
		characterNames = self.sbscene.getCharacterNames()
		for c in range(0, len(characterNames)):
			sbchar = self.sbscene.getCharacter(characterNames[c])
			# get the corresponding Panda character
			found = False
			pandachar = self.GetPawn(characterNames[c])
			if pandachar is None:
				print "Can't find Panda character for SmartBody character named [" + characterNames[c] + "]"
				continue
			sbskel = sbchar.getSkeleton()
			
			# global position
			globalPos = sbchar.getPosition()
			globalPandaPos = Vec3(globalPos.getData(0), globalPos.getData(1), globalPos.getData(2))
			finalGlobalPos = Converter.Sbm2Panda_Pos(globalPandaPos)
			pandachar.setPos(finalGlobalPos)
			
			# global orientation
			globalQuat = sbchar.getOrientation()
			globalPandaQuat = Quat(globalQuat.getData(1), globalQuat.getData(2), globalQuat.getData(3), globalQuat.getData(0))
			finalGlobalQuat = Converter.Sbm2Panda_Quat(globalPandaQuat)
			pandachar.setQuat(finalGlobalQuat)
			
			
			numJoints = sbskel.getNumJoints()
			
			# joint state
			for j in range(0, numJoints):
				sbjoint = sbskel.getJoint(j)
				
				pandaJoint = pandachar.FindJoint(sbjoint.getName())
						
				if pandaJoint is not None:
					# set the position and orientation of the joint
					#sbpos = sbjoint.getPosition()
					#pandaPos = Vec3(sbpos.getData(0), sbpos.getData(1), sbpos.getData(2))
					#finalPos = Converter.Sbm2Panda_Pos(pandaPos)
					#pandaJoint.setPos(finalPos)
					
					sbquat = sbjoint.getQuat()
					pandaQuat = Quat(sbquat.getData(1), sbquat.getData(2), sbquat.getData(3), sbquat.getData(0))
					finalQuat = Converter.Sbm2Panda_Quat(pandaQuat)
					pandaJoint.setQuat(finalQuat)
				
		
					
		
		
		if (task != None):
			return task.cont
					
		
		
	