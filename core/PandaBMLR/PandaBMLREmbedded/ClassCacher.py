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
import string
import os
import time

# Panda3D imports
from pandac.PandaModules import *

# PandaBMLR imports
from CharacterPawn import *


class ClassCacher:
	""" Chaches the models and textures of all classes.
	On program startup it will go through all class definitions present in the Classes
	directory and load them into memory"""
	
	def __init__(self, parseClassDir = True):
		
		self.Classes = {}
		self.BoneFiles = {}
		
		if (parseClassDir):
			self.ParseClassDirectory()

	def ParseClassDirectory(self):
		""" Parses all classes in the class dir from config file """
		
		print("Caching classes from Classes dir")
		t = time.time()
		classDir = ConfigVariableString("ClassDir", "Classes/").getValue()
		
		for obj in os.listdir(classDir):
			if (obj.endswith(".class")):
				self.CacheChar(obj, classDir)
		
		print("Class caching completed in %2.3f sec" % (time.time() - t))		
	
	def CacheChar(self, classFile, classDir):
		""" Caches one char """
		
		className = classFile.replace(".class", "")
		print("Caching class: " + className)
		
		char = CachedCharacter()
		
		try:			
			char.CharClass = className.replace(".class", "")
			self.ParseClassFile(classDir + classFile, char)
			
			if (len(char.BoneFile) > 0 and char.Valid):
				self.ParseBonesFile(char.BoneFile, char)
			else:
				print("Class file is missing bonefile")
				char.Valid = False
				
			if (len(char.ModelPath) > 0 and char.Valid):
				char.Model = loader.loadModel(char.ModelPath)
				if (char.Flip):
					char.Model.setH(180)
			else:
				print("Class file is missing model")
				char.Valid = False
			
			if (len(char.TexturePath) > 0 and char.Valid):
				char.Texture = loader.loadTexture(char.TexturePath)
				
				if (char.EnvMap > 0):
					char.Model.setTexGen(TextureStage.getDefault(), TexGenAttrib.MWorldCubeMap)
					char.Model.setTexScale(TextureStage.getDefault(), 0.15)
					char.Model.setTexture(TextureStage.getDefault(), char.Texture)
					char.Model.setShaderOff() # This is sort of a hack to env mapping work with the included shadow shader
				
		except:
			print("Failed to load class: " + className)
			print sys.exc_info()			
			char.Valid = False
			
		if (char.Valid):	
			self.Classes[className] = char
		else:
			print("Failed to load class: " + className)
				
	def ParseBonesFile(self, filePath, char):
		""" Parses a bone file """
		
		try:
			if (self.BoneFiles.has_key(filePath)):
				char.Bones = self.BoneFiles[filePath]
			else:
				print("Parsing bone file: " + filePath)
				file = open(filePath, "r")
				b = {}
				
				for line in file:
					if (not line.startswith("#")):
						lineSplit = line.split("=", 1)
						if (len(lineSplit) == 2):					
							boneID = int(lineSplit[0].strip())
							boneName = lineSplit[1].strip()
							b[boneID] = boneName
							
				char.Bones = b
				self.BoneFiles[filePath] = b
				
				file.close()
		except:
			char.Valid = False
			print("Failed to load bone definitions from: " + filePath)
			print sys.exc_info()
		
	def ParseClassFile(self, filePath, char):
		""" Parses a character class file """
		
		file = open(filePath, "r")
		for wholeline in file:
			try:
				line = wholeline.strip()
				print "" + line
				if (not line.startswith("#")):				
					if (line.startswith("texture=")):
						char.TexturePath = line.replace("texture=", "", 1)
						
					elif (line.startswith("model=")):
						char.ModelPath = line.replace("model=", "", 1)
						
					elif (line.startswith("flip=")):
						f = int(line.replace("flip=", "", 1))
						if (f == 1):
							char.Flip = True
						else:
							char.Flip = False
						
					elif (line.startswith("scalex=")):
						char.Scale[0] = float(line.replace("scalex=", "", 1))
						
					elif (line.startswith("scaley=")):
						char.Scale[1] = float(line.replace("scaley=", "", 1))
						
					elif (line.startswith("scalez=")):
						char.Scale[2] = float(line.replace("scalez=", "", 1))
						
					elif (line.startswith("offsetx=")):
						char.Offset[0] = float(line.replace("offsetx=", "", 1))
						
					elif (line.startswith("offsety=")):
						char.Offset[1] = float(line.replace("offsety=", "", 1))
						
					elif (line.startswith("offsetz=")):
						char.Offset[2] = float(line.replace("offsetz=", "", 1))
						
					elif (line.startswith("skeletontype=")):
						char.SkeletonType = int(line.replace("skeletontype=", "", 1))						
						
					elif (line.startswith("bonefile=")):
						char.BoneFile = line.replace("bonefile=", "", 1)
						
					elif (line.startswith("envmap=")):
						char.EnvMap = int(line.replace("envmap=", "", 1))
			except:
				char.Valid = False
				print("Failed to load class definition from: " + filePath)
				print(sys.exc_info())

		file.close()	
				
	
	def CreateChar(self, charClass):
		""" Creates a new instance of a character with class charClass """
		
		print("Initializing character of class: " + charClass)
		
		if(self.Classes.has_key(charClass)):
			return self.Classes[charClass]
		else:
			return None
			
		return 0

class CachedCharacter:
	""" Data storage for a cached character class """
	
	def __init__(self):
		self.SkeletonType = 0
		self.CharClass = ""
		self.Flip = False
		self.ModelPath = ""
		self.TexturePath = ""
		self.Scale = [1.0, 1.0, 1.0]
		self.Offset = [0, 0, 0]
		self.JointArr = []
		self.Model = None
		self.Texture = None
		self.Valid = True
		self.BoneFile = ""
		self.Bones = {}
		self.EnvMap = 0
	
	def Info(self):
		""" Prints out relevant info for this class """
		
		print("Printing CachedCharacter data:")
		print("Class: " + self.CharClass)
		print("ModelPath: " + self.ModelPath)
		print("Model: ", self.Model)
		print("Texture: " + self.TexturePath)
		print("Flip: " + str(self.Flip))
		print("Scale: " + str(self.Scale))
		print("Offset: " + str(self.Offset))
		print("SkeletonType: " + str(self.SkeletonType))	