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


# Panda3D imports
from pandac.PandaModules import *

#  Vector as string
def Pos2Str(vec3):
	""" Takes in a Vector3 and prints it to a string that SmartBody understands """
	# Note, this truncates floats!
	return "x " + str(int(vec3.getX())) + " y " + str(int(vec3.getY())) + " z " + str(int(vec3.getZ()))

def Hpr2Str(vec3):
	""" Takes in a Vector3 and prints it to a string that SmartBody understands """
	# Note, this truncates floats!
	return "h " + str(int(vec3.getX())) + " p " + str(int(vec3.getY())) + " r " + str(int(vec3.getZ()))


## Unreal 2 Panda
def Unreal2Panda_Pos(vec3):
	""" Transforms a Position from Unreal's coordinate system to a Position in Panda's coordinate system """

	return Vec3(vec3.getY(), vec3.getX(), vec3.getZ())	

def Unreal2Panda_Quat(quat):
	""" Transforms a Quaternion from Unreal's coordinate system to a Quaternion in Panda's coordinate system """

	#Quat(rotation[1], -rotation[2], rotation[4], rotation[3])
#	return Quat(quat.getX(), -quat.getY(), quat.getW(), quat.getZ())
	return Quat(quat.getY(), quat.getX(), -quat.getZ(), quat.getW())


## Sbm 2 Panda
def Sbm2Panda_Pos(vec3):
	""" Transforms a Position from SmartBody's coordinate system to a Position in Panda3D's coordinate system """
	return Vec3(-vec3.getX(), vec3.getZ(), vec3.getY())	


## Sbm 2 Panda
def Sbm2Panda_Quat(quat):
	# x y z w
	# i j k r
	return Quat(quat.getW(), quat.getX(), -quat.getZ(), quat.getY())


## Panda 2 Sbm
def Panda2Sbm_Pos(vec3):
	""" Transforms a Position from SmartBody's coordinate system to a Position in Panda3d's coordinate system """
	
	return Vec3(-vec3.getX(), vec3.getZ(), vec3.getY())

def Panda2Sbm_Hpr(vec3):
	""" Transforms a Hpr from Panda3D's coordinate system to a Position in SmartBody's coordinate system """
	return Vec3(vec3.getX(), vec3.getY(), vec3.getZ())


