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
from direct.interval.MetaInterval import Sequence

# BMLR imports
from PandaBMLREmbedded.PandaBMLR import PandaBMLR
from PandaBMLREmbedded.Pawn import Pawn
from PandaBMLREmbedded.CharacterPawn import CharacterPawn
from PandaBMLREmbedded.Hud import BMLR_Hud
from PandaBMLREmbedded.Camera import BMLR_Camera


class Init(DirectObject):
	""" This demo will showcase a lot of BMLR functionality such as using the camera, hud, console, characters, pawns
		BML (sync points, postures, gestures, speech) """
	
	def __init__(self):
		
		#############
		# Set up PandaBMLR
		#############		
		# Creates a BMLR instance to handle SmartBody communications
		self.BMLR = PandaBMLR()
		
		# Creates a camera to navigate around the scene
		self.Cam = BMLR_Camera(self.BMLR)
		
		# Creates the default BMLR hud
		self.Hud = BMLR_Hud(self.BMLR)
	
		# Sets the starting position of the camera		
		base.camera.setPosHpr(-27, 335, 201, -165, -17, 0)
		
		#############
		# Pawns
		#############
		
		# Pawn that is stuck to the camera to allow characters to gaze at camera
		self.CameraPawn = Pawn(self.BMLR, "CameraPawn", geom=None, geomType = 0)
		self.CameraPawn.reparentTo(base.camera)

		# And annother pawn that Rob gazes at!		
		self.Pawn = Pawn(self.BMLR, "RobsPawn", geom="DEFAULT", geomType = 1)
		self.Pawn.setPos(40, 50, 90)
		# This makes the pawn move in a triangle over 15 seconds
		(Sequence(self.Pawn.posInterval(5, Vec3(90, 120, 90)), self.Pawn.posInterval(5, Vec3(-20, 110, 90)), self.Pawn.posInterval(5, Vec3(40, 50, 90)))).loop()
		
		#############
		# Characters
		#############
		self.Rob = CharacterPawn(self.BMLR, "Rob", "CADIA.Rob")
		self.Rob.setPosHpr(133, 46, 99, 42, 0, 0)
		self.Rob.RegisterPosHpr()
		self.Rob.BML('<body posture="LHandOnHip_Motex" /><gaze target="RobsPawn"/>')
		
		self.SuperHumanoid = CharacterPawn(self.BMLR, "SuperHumanoid", "CADIA.MayaHumanoid.Silver")
		self.SuperHumanoid.setPosHpr(35, -10, 99, -6, 0, 0)
		self.SuperHumanoid.RegisterPosHpr()
		#self.SuperHumanoid.BML('<body posture="CrossedArms_Motex" /><gaze target="CameraPawn"/>')		
		self.SuperHumanoid.BML('<body posture="LHandOnHip_Motex" /><gaze target="CameraPawn"/>')		
		
		self.Kelly = CharacterPawn(self.BMLR, "Kelly", "CADIA.Kelly")
		self.Kelly.setPosHpr(89, 10, 99, 23, 0, 0)
		self.Kelly.RegisterPosHpr()
		#self.Kelly.BML('<body posture="HoldWrist_Motex_FrontLow" /><gaze target="CameraPawn"/>')
		self.Kelly.BML('<body posture="HandsAtSide_Motex" /><gaze target="CameraPawn"/>')
		
		self.Sam = CharacterPawn(self.BMLR, "Sam", "CADIA.Sam")
		self.Sam.setPosHpr(-18, 12, 99, -5, 0, 0)
		self.Sam.RegisterPosHpr()
		self.Sam.BML('<body posture="HandsAtSide_Motex" /><gaze target="CameraPawn"/>')
		
		#############
		# Scripted behavior
		#############
		print "About to run Kelly..."
		self.Kelly.BML('<speech><mark time="3"/>Hello there!</speech>')
		self.Kelly.BML('<speech><mark time="4"/>Thanks <mark time="4.3"/>for <mark time="4.6"/>trying <mark time="5"/>BML Realizer</speech>')
		print "Done with Kelly..."
		self.Sam.BML('<speech id="ssam1"><mark time="7"/>I am Sam<mark time="7.5"/>, and these <mark name="mall" time="8"/>are my friends!</speech>'
					 + '<gesture type="me" start="ssam1:mall" />'
					)
					 
		self.Sam.BML('<speech id="ssam2"><mark name="mrob" time="8.8"/>Rob, <mark  name="mkelly" time="11"/>Kelly <mark name="msh" time="13.5"/>and SuperHumanoid<mark name="mdone" time="14"/></speech>'
					 + '<gesture id="indright" start="ssam2:mkelly" type="indicate" armdirection="right" posture="CrossedArms" hand="right"/>'
					 + '<gaze stroke="ssam2:mrob" target="Rob"/>'
					 + '<gaze stroke="ssam2:msh" target="SuperHumanoid"/>'
					 + '<gaze stroke="ssam2:mdone" target="CameraPawn"/>'
					)
		
		self.Rob.BML('<speech id="srob"><mark time="9.1"/>Hey!<mark name="mstart" time="10" /> I\'m busy!</speech>'
					 + '<gesture start="srob:mstart" type="me" posture="HandsAtSide" hand="right"/>'
					)
		
		self.Kelly.BML('<speech id="skelly"><mark time="11.1"/>Hello!<mark name="mstart" time="11.5"/></speech>'
					   + '<gesture start="skelly:mstart" type="emphasis" posture="HandsAtSide" hand="right"/>'
					   )
		
		self.SuperHumanoid.BML('<speech id="ssh"><mark time="14"/>Top of the<mark name="mstart" time="14.4"/> morning to you!</speech>'
								+'<gesture start="ssh:mstart" type="chop" posture="HandsAtSide" hand="right"/>'
								)
		
		self.SuperHumanoid.BML('<speech><mark time="21"/>Try moving the camera around with WASD keys<mark time="22"/>, notice how we keep looking at you!</speech>')
		
		
		self.Kelly.BML('<speech><mark time="25"/>Also try opening the console <mark time="26"/>and typing \'test\', or <mark time="26.5"/>pressing F5.</speech>')
		
		self.Kelly.BML('<locomotion target="100 300"/>')
		
		#############
		# Load the environment
		#############
		self.InitEnvironment()
		
		# This binds the console command "test" to the function self.cmdTest
		self.accept("CMD_test", self.cmdTest)
		
		#this binds the key f5 to self.cmdTest
		self.accept("f5", self.cmdTest)
		
	
	def cmdTest(self):
		self.Kelly.BML("<speech>This is a test!</speech>")

	def InitEnvironment(self):
		if (not self.InitLightAdv()):
			self.InitLightBasic()
			
		# Creates a patterned floor with a texture
		floorTex = loader.loadTexture("Content/envir-ground.jpg")
		cm = CardMaker("FloorMaker")
		cm.setFrame(-30,30,-30,30)
		floor = render.attachNewNode(PandaNode("floor"))
		for y in range(12):
			for x in range(12):
				nn = floor.attachNewNode(cm.generate())
				nn.setP(-90)
				nn.setPos((x-6)*60, (y-6)*60, 25)
		floor.setTexture(floorTex)
		floor.flattenStrong()
		
	def InitLightBasic(self):
		""" Default lighting if video card does not support shaders """
		
		aLight = AmbientLight("AmbientLight")
		aLight.setColor(Vec4(0.3, 0.3, 0.3, 1))
		render.setLight(render.attachNewNode(aLight))
	
		dLight1 = DirectionalLight("DirectionalLight1")
		dLight1.setColor(Vec4(0.85, 0.9, 0.9, 1))		
		dLight1NP = render.attachNewNode(dLight1)
		dLight1NP.setHpr(210, 0, 0)
		render.setLight(dLight1NP)
	
		dLight2 = DirectionalLight("DirectionalLight2")
		dLight2.setColor(Vec4(0.35, 0.35, 0.3, 1))
		dLight2NP = render.attachNewNode(dLight2)
		dLight2NP.setHpr(150, -60, 0)
		render.setLight(dLight2NP)		
		
	def InitLightAdv(self):
		""" Creates lightning with shadows using shaders, and turns antialiasing on. Nothing BMLR related here. """

		###########################################################################################
		##        This code is borrowed from Panda3D's included shadow mapping sample            ##
		###########################################################################################
		
		if (base.win.getGsg().getSupportsBasicShaders()==0):
			print("Video driver reports that shaders are not supported.")
			return False
		
		if (base.win.getGsg().getSupportsDepthTexture()==0):
			print("Video driver reports that depth textures are not supported.")
			return False
		
		
		# Create the offscreen buffer.		
		winprops = WindowProperties.size(1024, 1024)
		props = FrameBufferProperties()
		props.setRgbColor(True)
		props.setAlphaBits(True)
		props.setDepthBits(True)
		LBuffer = base.graphicsEngine.makeOutput(base.pipe, "ShadowBuffer", -2,
					props, winprops, GraphicsPipe.BFRefuseWindow, base.win.getGsg(), base.win)		
		
		if (LBuffer == None):
			print("Video driver cannot create an offscreen buffer.")
			return False

		# Turn antialiasing on
		render.setAntialias(AntialiasAttrib.MMultisample,1)
		
		Ldepthmap = Texture()
		LBuffer.addRenderTexture(Ldepthmap, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPDepth)
		if (base.win.getGsg().getSupportsShadowFilter()):
			Ldepthmap.setMinfilter(Texture.FTShadow)
			Ldepthmap.setMagfilter(Texture.FTShadow)
		# Set up the shadow "camera"
		self.LCam=base.makeCamera(LBuffer)
		self.LCam.node().setScene(render)
		self.LCam.node().getLens().setFov(35)
		self.LCam.node().getLens().setNearFar(5, 1000)
		self.LCam.setPosHpr(-82, 382, 550, -162, -50, 0)
		self.LCam.reparentTo(render)

		# DEBUG
		#Lcolormap = Texture()
		#LBuffer.addRenderTexture(Lcolormap, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)
		#self.LCam.node().showFrustum()
		#self.accept("g", base.bufferViewer.toggleEnable)
		
		self.pushBias = 0.9
		self.ambient = 0.6
		
		mci = NodePath(PandaNode("Main Camera Initializer"))
		mci.setShader(Shader.load("Content/shadow.sha"))
		base.cam.node().setInitialState(mci.getState())
		
		# Set up shader inputs
		render.setShaderInput("light", 		self.LCam)
		render.setShaderInput("Ldepthmap", 	Ldepthmap)
		render.setShaderInput("ambient",	self.ambient, 0, 0, 1)
		render.setShaderInput("texDisable",	0, 0, 0, 0)
		render.setShaderInput("scale", 		1, 1, 1, 1)
		render.setShaderInput("push", 		self.pushBias, self.pushBias, self.pushBias, 0)
		return True

Init()
run()
