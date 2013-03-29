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
# #  Redistribution and uese in source and binary forms, with or without
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


# Panda3D imports
from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from direct.interval.LerpInterval import *
from direct.interval.MetaInterval import *

# PandaBMLR imports
from Converter import *


GEOMTYPE_STATIC = 0
GEOMTYPE_STATIC_ANIMATED = 1
GEOMTYPE_CHARACTER = 2

CREATOR_BMLR = 0
CREATOR_SBM = 1

PLATE_COLOR_CHAR = Vec4(0, 0, 1, 1)
PLATE_COLOR_PAWN = Vec4(0, 1, 1, 1)
PLATE_COLOR_ACTIVE = Vec4(1, 0, 1, 1)	

class Pawn(NodePath, DirectObject):
	""" A smartbody Pawn. A character is also defined as a pawn! """
	
	def __init__(self, bmlr, name, geom = None, geomType = GEOMTYPE_STATIC_ANIMATED, registered = False):
		
		self.__Valid = False
		
		if (len(name) == 0):
			print("A name is required for all Pawns")
			return
		
		if (bmlr.GetScene().Pawns.has_key(name)):
			print("Error creating pawn, it already exists in scene: " + name)
			return

		NodePath.__init__(self, name + "_Pawn")
		self.Scene = bmlr.GetScene()
		self.Scene.Pawns[name] = self
		self.__BMLR = bmlr		
		self.__TypeStr = "Pawn"
		self.__TypeSbm = "pawn"
		self.__Name = name
		self.__Valid = True		
		self.__Registered = True
		self.__Geom = geom		
		self.__GeomType = geomType
		self.__CreatorBMLR = not registered  # If our pawn is registered on creation, a SmartBody client initiated the creation, otherwise we did
		self.__InitGeom()
		self.__InitPlate()
		self.__LastPos = self.getPos(render)
		self.__LastHpr = self.getHpr(render)
		self.__NotValidMessage = False
		self.__NotRegisteredMessage = False

		messenger.send("UpdateHud")
		taskMgr.add(self.Step, "Pawn" + self.GetName() + "_Loop")
		
		if geomType == GEOMTYPE_STATIC_ANIMATED:
			print "Loading local SmartBody pawn: [" + self.GetName() + "]"
			mychar = bmlr.sbscene.createPawn(self.GetName())
		
	def Step(self, task = None):
		""" We calculate how much the pawn has moved or rotated. If it is above a certain threshold, we will let SmartBody know """
		
		newPos = self.getPos(render)
		posDelta = (newPos - self.__LastPos).length()
		hDelta = abs(self.getH(render) - self.__LastHpr.getZ())
		pDelta = abs(self.getP(render) - self.__LastHpr.getY())
		rDelta = abs(self.getR(render) - self.__LastHpr.getX())
		
#		if ((posDelta > 3 or hDelta > 1 or pDelta > 1 or rDelta > 1) and self.IsRegistered()):
		if (False):
			self.__LastPos = newPos
			self.__LastHpr = self.getHpr(render)
			self.RegisterPosHpr()
			
		if (task != None):
			return task.cont
	
	def SetPlateColor(self, color = None):
		if (color == None):
			# Resets it
			color = self.__PlateColor
			
		self.Plate.NamePlate.setFrameColor(color);
		
	def GetCreator(self):
		if (self.__CreatorBMLR):
			return CREATOR_BMLR
		else:
			return CREATOR_SBM
	
	def CmpName(self, other):
		return cmp(self.GetName(), other.GetName())
		
	def Destroy(self):
		if (type(self) == Pawn):
			self.Scene.SendSbmCommand("python scene.removePawn(\"" + self.GetName() + "\")")
		else:
			self.Scene.SendSbmCommand("python scene.removeCharacter(\"" + self.GetName() + "\")")
		self.removeNode()
		self.Scene.Pawns.pop(self.GetName())
	

	def __InitPlate(self):
		""" Loads a nameplate for the pawn """
		
		self.PlateNode = self.attachNewNode("Plate")
		
		if (self.__GeomType == GEOMTYPE_CHARACTER):
			self.__PlateColor = PLATE_COLOR_CHAR			
		else:
			self.__PlateColor = PLATE_COLOR_PAWN
			
		plateText = self.__Name
		
		self.Plate = NamePlate(self.__Name + "_Plate", plateText, self.__PlateColor)
		self.PlateNode.attachNewNode( self.Plate.node() )
		self.PlateNode.reparentTo(self)

		# Calculate how far above the model the nameplate should be
		pMin = Point3()
		pMax = Point3()
		self.ModelNode.calcTightBounds(pMin, pMax)
		self.PlateNode.setZ(pMax.getZ() + 10 )
		
		if (not self.__BMLR.PlatesOn()):
			self.Plate.hide()
					
		self.accept("HideAllPlates", self.HidePlate)
		self.accept("ShowAllPlates", self.ShowPlate)

	def __InitGeom(self):
		hide = False
		if (self.__Geom == None):
			self.__Geom = ConfigVariableString("DefaultPawnModel", "").getValue()
			hide = True
		elif (self.__Geom == "" or self.__Geom.lower() == "default"):
			self.__Geom = ConfigVariableString("DefaultPawnModel", "").getValue()
		
		self.ModelNode = self.attachNewNode("Model")		
		
		if (self.__GeomType == GEOMTYPE_STATIC or self.__GeomType == GEOMTYPE_STATIC_ANIMATED):
			# Load the model
			self.ModelNode.attachNewNode((loader.loadModel(self.__Geom)).node())
			self.ModelNode.setTwoSided(True)
			self.ModelNode.setScale(50)
			
			if (self.__GeomType == GEOMTYPE_STATIC_ANIMATED):
				# Setup animation intervals for the model				
				posInt1 = LerpPosInterval(self.ModelNode, 1, self.getPos() + Vec3(0, 0, -7), self.getPos(), blendType = "easeInOut")
				posInt2 = LerpPosInterval(self.ModelNode, 1, self.getPos(), self.getPos() + Vec3(0, 0, -7), blendType = "easeInOut")
				posInt = Sequence(posInt1, posInt2)
				hprInt = LerpHprInterval(self.ModelNode, 2, Vec3(360, 0, 0), Vec3(0, 0, 0))		
				self.Anim = Parallel(hprInt, posInt)
				
		elif (self.__GeomType == GEOMTYPE_CHARACTER):
			self.Class = self.Scene.CreateChar(self.__Geom)
			if (self.Class == None):
				print ("ERROR: Class: " + self.__Geom + " does not exist.")
				self.__Valid = False
				return
			self.Class.Model.copyTo(self.ModelNode)
			self.ModelNode.setScale(self.Class.Scale[0], self.Class.Scale[1], self.Class.Scale[2])
	
		self.reparentTo(render)
		if (hide):
			self.hide()
		
	def __str__(self):
		return "Pawn information..." + \
			"\nName: " + self.GetName() + \
			"\nGeom: " + str(self.__Geom) 
		
	def HidePlate(self):
		self.Plate.hide()
		
	def ShowPlate(self):
		self.Plate.show()
			
	def GetName(self):
		return self.__Name
	
	def IsRegistered(self):
		return self.__Registered
	
	def RegisterInit(self):

			
		self.__NotValidMessage = False
		
		if (not self.__Registered):
			print "Now sending command to register pawn " + self.GetName() + " with SmartBody..."
			self.Scene.SendSbmCommand("python scene.createPawn(\"" + self.GetName() + "\")")
			self.__Registered = True
			self.RegisterPosHpr()
			if (self.__GeomType == GEOMTYPE_STATIC_ANIMATED):
				self.Anim.loop()
		
	def RegisterPosHpr(self):
		self.__NotValidMessage = False
		if (not self.IsRegistered()):
			if (not self.__NotRegisteredMessage):
				print("Trying to register position of an unregistered " + self.__TypeStr + ": " + self.GetName())
				self.__NotRegisteredMessage = True
			return
		self.__NotRegisteredMessage = False
		print "REGISTERING POSHPR..."
		#pos = Pos2Str(Panda2Sbm_Pos(self.getPos(render)))
		#hpr = Hpr2Str(Panda2Sbm_Hpr(self.getHpr(render)))
		#self.Scene.SendSbmCommand("set " + self.__TypeSbm + " " + self.GetName() + " world_offset " + pos + " " + hpr)
		hpr = self.getHpr(render)
		pos = self.getPos(render)
		typeCommand = "getPawn"
		if self.__TypeStr == "CharacterPawn":
			typeCommand = "getCharacter"
		self.Scene.SendSbmCommand("python scene." + typeCommand + "(\"" + self.GetName() + "\").setHPR(SrVec(" + str(hpr.getX()) + ", " + str(hpr.getY()) + ", " + str(hpr.getZ()) + "))")
		self.Scene.SendSbmCommand("python scene." + typeCommand + "(\"" + self.GetName() + "\").setPosition(SrVec(" + str(-pos.getX()) + ", " + str(pos.getZ()) + ", " + str(pos.getY()) + "))")
		

			
	def SetValid(self, valid):
		self.__Valid = valid
		
	def SetRegistered(self, registered):
		self.__Registered = registered
	
	def Unregister(self):
		self.__Registered = False
		if (self.__GeomType == GEOMTYPE_STATIC_ANIMATED):
			self.Anim.pause()
		

class NamePlate(NodePath):
	""" Nameplate to display text, such as the name of a character or a pawn """
	
	def __init__(self, id, text, borderColor):
		NodePath.__init__(self, id)
		self.setBillboardPointEye();
		
		self.NamePlate = TextNode(id);		
		self.NamePlate.setAlign(TextNode.ACenter);
		self.NamePlate.setFrameColor(borderColor);
		self.NamePlate.setFrameAsMargin(0.4, 0.4, 0.1, 0.1);
		self.NamePlate.setFrameLineWidth(4);
		self.NamePlate.setFrameCorners(True);
		self.NamePlate.setCardColor(0, 0, 0, 0.4);
		self.NamePlate.setCardAsMargin(0.4, 0.4, 0.1, 0.1);
		self.NamePlate.setCardDecal(True);
		self.NamePlate.setText(text);
		
		self.attachNewNode(self.NamePlate)
		self.setScale(ConfigVariableDouble("NamePlateScale", 4).getValue())
		self.setLightOff()		
		self.setShaderOff()