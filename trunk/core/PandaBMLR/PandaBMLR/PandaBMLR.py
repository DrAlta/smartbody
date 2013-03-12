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
from NetHandler import NetHandler
from Scene import Scene
from ClassCacher import ClassCacher


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
		self.__NetHandler = NetHandler(self)
		self.__PlatesOn = True
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
		
		self.__NetHandler.SendCommand(cmd)		
		
	def SbmStart(self, auto = False):
		""" Starts SmartBody with paremeters from config file. Windows only """
		
		if (auto):				
			autoStart = ConfigVariableBool("SbmAutoStart", 0).getValue()
			if (not autoStart):
				return
		
		path = ConfigVariableString("SbmPath", "").getValue()
		if (not path.endswith("\\")):
			path += "\\"							  
		bin = ConfigVariableString("SbmBin", "").getValue()
		args = ConfigVariableString("SbmArgs", "").getValue()
		initSeq = ConfigVariableString("SbmInitSeq", "").getValue()
		if (len(initSeq) > 0):
			initSeq = " -script " + initSeq
	
		if (0 != system("start /MIN /D" + path + " " + path+bin + " " + args + " " + initSeq)):
			print ("ERROR STARTING SmartBody!")
			print ("Path used: " + path+bin + " " + args + " " + initSeq)
			exit(1)
		
		atexit.register(self.__SbmOnQuit)
		
	def __SbmOnQuit(self):
		""" Exit SmartBody """
		
		self.__NetHandler.SendCommand("q")
		
	def GetPawn(self, pawnName):
		""" Finds a Pawn with name pawnName. Returns CharacterPawns as well. """
		
		return self.__Scene.Pawns.get(pawnName)
		
	def GetCharByID(self, charID):
		""" Finds CharacterPawns by their ID (not name) """
		
		return self.__Scene.Characters.get(charID)		
		
	def GetScene(self):
		""" Returns a reference to the Scene object """
		
		return self.__Scene
	
	def GetNet(self):
		""" Returns a reference to the NetHandler object """
		
		return self.__NetHandler
			
	def Step(self, task = None):
		""" Frame by frame updates """
		
		self.__NetHandler.CheckTCP()
		self.__NetHandler.CheckUDP()
		
		if (task != None):
			return task.cont
		
		
		
	