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
from direct.showbase.DirectObject import DirectObject
from direct.task import Task

# PandaBMLR imports
import Pawn


MOUSE_SENSITIVITY = 60
MOVEMENT_SPEED = 60
ROTATIONAL_SPEED = 40

class BMLR_Camera(DirectObject):
	""" A reference implementation of a class to handle camera navigation.
		Not required, and is not initialized automatically """
	
	def __init__(self, bmlr):
		""" Handles keyboard and mouse input """
		
		self.__BMLR = bmlr
		self.__BMLR.Camera = self
		self.__LastTime = 0
		self.CamTarget = base.camera
		
		self.InitInput()
		self.InitMouse()
		
		self.TargetCam()
		
		taskMgr.add(self.Update, "BMLR_CameraUpdate")
		
	def TargetCam(self):
		""" Resets the camera target to default """
		self.CamTarget = base.camera
		self.TargetPos = 0
		
		targets = self.__BMLR.GetScene().Pawns.values()
		for p in targets:
			p.SetPlateColor()		
		
	def TargetPrev (self):
		""" Sets the camera target backwards """
		
		self.TargetNext(-1)	
		
		
	def TargetNext(self, incr = 1):
		""" Sets the camera target forwards """
		
		targets = self.__BMLR.GetScene().Pawns.values()
		targets.sort(Pawn.Pawn.CmpName)
		
		if len(targets) == 0:
			self.CamTarget()
			return
		
		self.TargetPos += incr
		
		if (self.TargetPos > len(targets)):
			self.TargetPos = 0		
		elif (self.TargetPos < -1):
			self.TargetPos = len(targets)
				
		# Resets the plate color
		for p in targets:
			p.SetPlateColor()
				
		if (self.TargetPos > 0):
			self.CamTarget = targets[self.TargetPos-1]
			self.CamTarget.SetPlateColor(Pawn.PLATE_COLOR_ACTIVE)			
		else:
			self.CamTarget = base.camera
			
	def InitMouse(self):
		""" Set mouse settings """
		
		base.disableMouse()
		self.__LastX = 0
		self.__LastY = 0
		
	def InitInput(self):
		""" Set input (keyboard and mouse) event handlers """
				
		self.keystate = { "back":0, "forw":0, "sleft":0, "rleft":0, "sright":0, "rright":0, "down":0, "up":0, "p_down":0, "p_up":0, "r_down":0, "r_up":0, "mouse3":0, "shift":0 }
		
		# move forwards
		self.accept("w", self.SetKeystate, ["forw",1])
		self.accept("w-up", self.SetKeystate, ["forw",0])
		
		# move backwards
		self.accept("s", self.SetKeystate, ["back",1])
		self.accept("s-up", self.SetKeystate, ["back",0])
		
		# strafe left
		self.accept("a", self.SetKeystate, ["sleft",1])
		self.accept("a-up", self.SetKeystate, ["sleft",0])
		
		# strafe right
		self.accept("d", self.SetKeystate, ["sright",1])
		self.accept("d-up", self.SetKeystate, ["sright",0])
		
		# rotate left
		self.accept("q", self.SetKeystate, ["rleft",1])
		self.accept("q-up", self.SetKeystate, ["rleft",0])
		
		# rotate right
		self.accept("e", self.SetKeystate, ["rright",1])
		self.accept("e-up", self.SetKeystate, ["rright",0])	
		
		# move up
		self.accept("space", self.SetKeystate, ["up",1])
		self.accept("space-up", self.SetKeystate, ["up",0])	
		
		# move down
		self.accept("z", self.SetKeystate, ["down",1])
		self.accept("z-up", self.SetKeystate, ["down",0])
		
		# pitch up
		self.accept("u", self.SetKeystate, ["p_up",1])
		self.accept("u-up", self.SetKeystate, ["p_up",0])
		
		# pitch down
		self.accept("j", self.SetKeystate, ["p_down",1])
		self.accept("j-up", self.SetKeystate, ["p_down",0])
		
		# roll up
		self.accept("i", self.SetKeystate, ["r_up",1])
		self.accept("i-up", self.SetKeystate, ["r_up",0])
		
		# roll down
		self.accept("k", self.SetKeystate, ["r_down",1])
		self.accept("k-up", self.SetKeystate, ["r_down",0])
		
		# shift
		self.accept("shift", self.SetKeystate, ["shift",1])
		self.accept("shift-up", self.SetKeystate, ["shift",0])					
		
		self.accept("b", self.TargetCam)
		self.accept("n", self.TargetNext)
		self.accept("m", self.TargetPrev)		
		
		self.accept("mouse3", self.SetKeystate, ["mouse3",1])
		self.accept("mouse3-up", self.SetKeystate, ["mouse3",0])
		
	def DropInput(self):
		""" Stops listening for input (when console is open) """
		
		self.ignoreAll()
		
	def SetKeystate(self, key, state):
		""" Handles the press of a button """
		
		self.keystate[key] = state		

	def SetMouseCentered(self):
		""" Reset mouse position to the center of the window """
		
		base.win.movePointer(0, base.win.getXSize()/2, base.win.getYSize()/2)

	def Update(self, task):
		""" Move the camera in accordance to the input given """
		
		# figure out how much time has passed since last frame
		elapsed = task.time - self.__LastTime
		self.__LastTime = task.time
		
		# Get the target that is being controled by the camera
		node = self.CamTarget
		
		speed = MOVEMENT_SPEED
		if (self.keystate['shift']):
			speed *= 4
		
		# update position		
		if (self.keystate['forw']):
			node.setY(node, speed*elapsed)
		if (self.keystate['back']):
			node.setY(node, -speed*elapsed)

		if (self.keystate['sright']):
			node.setX(node, speed*elapsed)
		if (self.keystate['sleft']):
			node.setX(node, -speed*elapsed)

		if (self.keystate['up']):
			node.setZ(node, speed*elapsed)
		if (self.keystate['down']):
			node.setZ(node, -speed*elapsed)
			
			
		# Update orientation
			
		if (self.keystate['rright']):
			node.setH(node, elapsed*-ROTATIONAL_SPEED)
		if (self.keystate['rleft']):
			node.setH(node, elapsed*ROTATIONAL_SPEED)
			
		if (self.keystate['p_up']):
			node.setP(node, elapsed*-ROTATIONAL_SPEED)
		if (self.keystate['p_down']):
			node.setP(node, elapsed*ROTATIONAL_SPEED)
			
		if (self.keystate['r_up']):
			node.setR(node, elapsed*-ROTATIONAL_SPEED)
		if (self.keystate['r_down']):
			node.setR(node, elapsed*ROTATIONAL_SPEED)						

		
		if base.mouseWatcherNode.hasMouse(): # Need to check otherwise game crashes on alt tab
			
				x = base.mouseWatcherNode.getMouseX()
				y = base.mouseWatcherNode.getMouseY()
				
				if (self.keystate["mouse3"]):
					dX = x - self.__LastX
					dY = y - self.__LastY
					
					base.camera.setHpr(base.camera, -dX*MOUSE_SENSITIVITY, dY*MOUSE_SENSITIVITY, 0)
				
				base.camera.setR(0)
				self.__LastX = x
				self.__LastY = y
			
		return task.cont
