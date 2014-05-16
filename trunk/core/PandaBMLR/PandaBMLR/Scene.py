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
import sys
import re

# PandaBMLR imports
from Pawn import *
from CharacterPawn import *
from ClassCacher import * 
import Converter

class Scene:
	""" Handles a SmartBody "scene". Maintains a list of characters and
		pawns and processes incoming commands from SmartBody """
	
	def __init__(self, BMLR):
		self.__BMLR = BMLR
		
		# Name indexed dictionary
		self.Pawns = {}
	
		# ID indexed dictionary for fast lookups
		self.Characters = {}
		
		self.__SmartBodyReady = False
		self.__ClassCacher = ClassCacher()
		self.Connected = False
	
		
	def CreateChar(self, charClass):		
		return self.__ClassCacher.CreateChar(charClass)		

	def ProcessJointRotations(self, time, charId, boneData):
		""" Processes bundle of requests for joint rotational changes from SmartBody """
		
		try:
			character = self.Characters[int(charId)]
			if (character != None):
				for rotation in boneData:
					# 0 = BoneID, 1-4 = q.wxyz
					#character.SetJointQuat(rotation[0], Converter.Unreal2Panda_Quat(Vec4(rotation[1], rotation[2], rotation[3], rotation[4])))
					character.SetJointQuat(int(time), int(rotation[0]), Converter.Sbm2Panda_Quat(Vec4(rotation[2], rotation[3], rotation[4], rotation[1])))
			else:
				print "No character " + charId + " found, cannot process joint rotations..."
		except:
			print("Error parsing UDP packet")
			print sys.exc_info()
	
	def ProcessJointPositions(self, time, charId, boneData):
		""" Processes bundle of requests for joint positional changes from SmartBody """
		
		try:
			character = self.Characters[int(charId)]
			if (character != None):
				for pos in boneData:
					# 0 = BoneID, 1-3 = xyz
					character.SetJointPos(int(time), int(pos[0]), Converter.Sbm2Panda_Pos(Vec3(pos[1], pos[2], pos[3]))) 
		except:
			print("Error parsing UDP packet")
			print sys.exc_info()
	
	def OnConnect(self, task = None):
		""" Executed when a SmartBody process connects """
		if (not self.__BMLR.GetNet().IsConnected()):
			return
		
		self.__SmartBodyReady = True

		for pawn in self.Pawns.values():
			pawn.RegisterInit()
			print "Pawn registered..."
			
		messenger.send("SbmConnected")
		self.Connected = True

	def OnDisconnect(self, task = None):
		""" Executed when a SmartBody process connects """
		
		self.__SmartBodyReady = False
		
		for pawn in self.Pawns.values():
			pawn.Unregister()
			
		self.Connected = False

	def IsSmartBodyReady(self):
		return self.__SmartBodyReady
			
	def SendSbmCommand(self, command):
		self.__BMLR.GetNet().SendCommand(command)

	def ProcessCommands(self, buffer):
		""" Processes a bundle of TCP commands, seperated by ; """
		tokens = buffer.split(";")
		for token in tokens:
			arguments = token.split("|")
			command = arguments[0]
			del arguments[0]
			self.ProcessCommand(command, arguments)
			
	def ProcessCommand(self, command, arguments):
		""" Processes a single TCP/UDP string command and it's arguments """
		#print "Command is " + command
		if (command == "CreateActor"):
			print "Actor created!"
			if (len(arguments) > 3):
				try:
					charID      = int(arguments[0])
					charClass   = strip(arguments[1])
					charName    = strip(arguments[2])
					skelType    = arguments[3] # Not used
					
					found = False
					for char in self.Pawns.values():
						if (char.GetName() == charName):
							char.SetID(charID)
							found = True
							break				
					
					if (not found):
						CharacterPawn(self.__BMLR, charName, charClass, charID, registered = True)
				except ValueError:
					print "Bad CreateActor data"
		if (command == "UpdateActor"):
			if (len(arguments) > 3):
				try:
					charID      = int(arguments[0])
					charClass   = strip(arguments[1])
					charName    = strip(arguments[2])
					skelType    = arguments[3] # Not used
					
					found = False
					for char in self.Pawns.values():
						if (char.GetName() == charName):
							char.SetID(charID)
							found = True
							break				
					
					if (not found):
						CharacterPawn(self.__BMLR, charName, charClass, charID, registered = True)
				except ValueError:
					print "Bad UpdateActor data"
				
		elif (command == "DeleteActor"):
			if (len(arguments) > 0):
				try:
					charID 			= int(arguments[0])
				
					char = self.Characters.get(charID)
					if (char != None):
						char.Destroy()
				except ValueError:
					print "Bad DeleteActor data"						
				
		elif (command == "SetActorPos"):
			if (len(arguments) > 3):
				try:
					charID 	= int(arguments[0])
					x 		= float(arguments[1])
					y 		= float(arguments[2])
					z 		= float(arguments[3])
					
					char = self.Characters.get(charID)

					if (char != None):
						if (not self.IgnorePawnPosRot(char)):						
								#char.setPos(Converter.Unreal2Panda_Pos(Vec3(x, y, z)))
							char.setPos(Converter.Sbm2Panda_Pos(Vec3(x, y, z)))
				except ValueError:
					print "Bad SetActorPos data"	
		
		elif (command == "SetActorRot"):
			if (len(arguments) > 4):
				
				try:
					charID 	= int(arguments[0])
					w 		= float(arguments[1])
					x 		= float(arguments[2])
					y 		= float(arguments[3])
					z 		= float(arguments[4])				
					char = self.Characters.get(charID)
				
					if (char != None):
						if (not self.IgnorePawnPosRot(char)):						
							char.SetRotQuat(Converter.Sbm2Panda_Quat(Quat(x, y, z, w)))
				except ValueError:
					print "Bad SetActorRot data"				
		elif (command == "SetActorViseme"):
			if (len(arguments) > 3):
				
				try:
					charID 	= int(arguments[0])
					visemeId = int(arguments[1])
					weight = float(arguments[2])
					blendTime = float(arguments[3])
					char = self.Characters.get(charID)
				
					if (char != None):
						if (not self.IgnorePawnPosRot(char)):						
							char.SetViseme(visemeId, weight, blendTime)
				except ValueError:
					print "Bad SetActorViseme data"									
				
					
		elif (command == "SpeakText"):
			if (len(arguments) > 2):
				try:
					id			= arguments[0]
					charName		= arguments[1]
					text 			= arguments[2]
				
					char = self.Pawns.get(charName)
					
					if (char != None):			
						char.Speak(text, 0, bubbleID = id)
				except ValueError:
					print "Bad SpeakText data"						
					
		elif (command == "SetBoneId"):
			if (len(arguments) > 2):
				try:
					charName	= arguments[0]
					boneName	= arguments[1]
					id 			= arguments[2]
					char = self.Characters.get(int(charName))
					
					if len(id) != 0:
						if (char != None):
							char.AddBoneBusMap(boneName, int(id))
				except ValueError:
					print "Bad SetBoneId data"			
		elif (command == "SetVisemeId"):
			if (len(arguments) > 2):
				try:
					charName	= arguments[0]
					visemeName	= arguments[1]
					id 			= arguments[2]
					char = self.Characters.get(int(charName))
					
					if len(id) != 0:
						if (char != None):
							char.AddVisemeMap(visemeName, int(id))
				except ValueError:
					print "Bad SetVisemeId data"											
			
			"""
		elif (command == "SetPawnPosHpr"):
			# TODO ! We are receiving this msg only for just pawns but characters as well!
			if (len(arguments) > 6):
				name 	= arguments[0]
				x 		= float(arguments[1])
				y 		= float(arguments[2])
				z 		= float(arguments[3])
				h 		= float(arguments[4])
				p 		= float(arguments[5])
				r 		= float(arguments[6])		
				
				p = self.Pawns.get(name)
				
				if (p != None):
					print arguments
					if (not self.IgnorePawnPosRot(p)):
						p.setPos(Converter.Sbm2Panda_Pos(Vec3(x, y, z)))
			"""		
			
		elif (command == "CreatePawn"):
			if (len(arguments) > 3):
				try:
					name 	= arguments[0]
					x 		= float(arguments[1])
					y 		= float(arguments[2])
					z 		= float(arguments[3])
					
					if (not self.Pawns.has_key(name)):
						p = Pawn(self.__BMLR, name, registered = True, geom = "DEFAULT")									
						if (p.IsValid()):
							p.setPos(Converter.Sbm2Panda_Pos(Vec3(x, y, z)))
				except ValueError:
					print "Bad SetBoneId data"	
		
		elif (command == "vrAllCall"):
			taskMgr.doMethodLater(4, self.OnConnect, "vrAllCall")

	def IgnorePawnPosRot(self, p):
		if (self.__BMLR.Camera != None):
			if (self.__BMLR.Camera.CamTarget != base.camera):
				if (self.__BMLR.Camera.CamTarget.GetName() == p.GetName()):
					# Here we check if the pawn is being controled by the camera. If so, we ignore
					# incoming SetPawnPosHpr for that pawn to avoid jerky behaviour
					return True
		return False