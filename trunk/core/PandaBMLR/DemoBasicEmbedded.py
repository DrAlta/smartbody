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


# Load a config file before anything else
from pandac.PandaModules import loadPrcFile
loadPrcFile("PandaBMLR.config")

# Panda3D imports
from pandac.PandaModules import *
import direct.directbase.DirectStart
from direct.showbase.DirectObject import DirectObject

# BMLR imports
from PandaBMLREmbedded.PandaBMLR import PandaBMLR
from PandaBMLREmbedded.CharacterPawn import CharacterPawn

class Init(DirectObject):
	""" This demo shows the bare minimum required code to get BMLR working """
	
	def __init__(self):
		self.BMLR = PandaBMLR()

		
		base.disableMouse()
		base.camera.setPosHpr(0, 340, 140, 180, -7, 0)
		
		self.George = CharacterPawn(self.BMLR, "George", "CADIA.George")
		self.George.setPos(0, 0, 102)
		self.George.RegisterPosHpr()

		#self.George.BML('<body posture="HoldWrist_Motex_FrontLow" />')
		self.George.BML('<body posture="HandsAtSide_Motex" />')
		self.George.BML('<speech id="sid">Thanks <mark time="0.3"/>for <mark time="0.6"/>trying <mark name="markbeat" time="1"/>BML Realizer!</speech>'
						+ '<head type="nod" id="n1" relax="0" />'
						+ '<gesture type="dismiss" start="n1:relax" />'
						)

		self.InitEnvironment()

	def InitEnvironment(self):
		""" Creates a simple floor plane """
		
		maker = CardMaker("floorMaker")
		maker.setFrame(-500, 500, -500, 500)
		floorNP = render.attachNewNode(maker.generate( ))
		floorNP.setPosHpr(0, 0, 26, 0, -90, 0)
		floorNP.setColor(0.3, 0.3, 0.3, 0.7)
		
Init()
run()


