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
from direct.gui.OnscreenText import OnscreenText

# Python base imports
from string import *
import sys

# PandaBMLR imports
import Pawn


def CalcHeight(np):
	""" Calculates the height of a nodepath """
	
	pMin = Point3()
	pMax = Point3()
	np.calcTightBounds(pMin, pMax)		
	h = (pMax-pMin).getZ()
	return h

def CalcWidth(np):
	""" Calculates the width of a nodepath """
	
	pMin = Point3()
	pMax = Point3()
	np.calcTightBounds(pMin, pMax)		
	w = (pMax-pMin).getX()
	return w

class BMLR_Hud(DirectObject):
	""" A reference implementation of a class to handle the HUD (Heads up display ).
		Not required, and is not initialized automatically """
	
	def __init__(self, BMLR):
		""" Initializes the HUD and console """
		
		self.__BMLR = BMLR
		self.InitPanel()
		self.Console = Console(BMLR)
		
		self.UpdateTextStats()
		self.accept("aspectRatioChanged", self.Resize)
		self.accept("f2", self.ToggleHud)
		
	def ToggleHud(self):
		""" Toggles the HUD on and off """
		
		if (self.TextStats.isHidden()):
			self.TextStats.show()
		else:
			self.TextStats.hide()		
		
	def Resize(self):		
		hasp = base.getAspectRatio()
		vasp = 1
		
		if (hasp < 1):
			hasp = 1
			vasp = 1/base.getAspectRatio()
		
		self.TextStats.setPos(hasp - 0.6, 0, -vasp+0.5)
		
	def InitPanel(self):
		self.PanelNP = aspect2d.attachNewNode("BMLR_Hud")
		self.PanelNP.setTransparency(TransparencyAttrib.MAlpha)
		
		self.PanelConsoleNP = self.PanelNP.attachNewNode("BMLR_Console")
		self.PanelInputeNP = self.PanelNP.attachNewNode("BMLR_Input")
		self.PanelTextDataNP = self.PanelNP.attachNewNode("BMLR_TextData")
		self.PanelTextStatsNP = self.PanelNP.attachNewNode("BMLR_TextStats")		
		
		self.TextStats = TextBox("BMLR_TextStats", Vec4(1, 0, 0, 0.5), self.PanelTextStatsNP)
		
		
		self.FrameRater = FrameRateMeter("FrameRateMeter")
	
	def UpdateTextStats(self, event = None):
		
		self.TextStats.setFrameColor(Vec4(0, 1, 0, 0.5))
		
		text = ""
		
		if (self.__BMLR.Camera != None):
			c = self.__BMLR.Camera.CamTarget
			n = ""
			if (c != base.camera):
				n = "(" + c.GetName() + ")"
				
			text += "Camera: " + n + " \n X: " + str(int(c.getX())) + " Y: " + str(int(c.getY())) + " Z: " + str(int(c.getZ())) + "\n"
			text += " H: " + str(int(c.getH())) + " P: " + str(int(c.getP())) + " R: " + str(int(c.getR())) + "\n"
									
		text += "FPS:\t"
		self.FrameRater.update()
		text += str(self.FrameRater.getText().replace(" fps" , ""))+ "\n"
		
		self.TextStats.setText(text)		
		taskMgr.doMethodLater(0.5, self.UpdateTextStats, "HudTextStatsLoop")

		

class TextBox(NodePath):
	def __init__(self, name, borderColor, parent):
		NodePath.__init__(self, name+"_NP")
		
		self.Text = TextNode(name);
		self.Text.setAlign(TextNode.ALeft);
		self.Text.setFrameColor(borderColor);
		self.Text.setFrameAsMargin(0.4, 0.4, 0.4, 0.4);
		self.Text.setFrameLineWidth(4);
		self.Text.setFrameCorners(True);
		self.Text.setCardColor(0, 0, 0, 0.4);
		self.Text.setCardAsMargin(0.4, 0.4, 0.4, 0.4);
		self.Text.setText("");
		self.TextStatsNP = self.attachNewNode(self.Text)
		self.setScale(0.05)
		self.reparentTo(parent)
		
	def setText(self, text):
		self.Text.setText(text)
		
	def setFrameColor(self, borderColor):
		self.Text.setFrameColor(borderColor);
		



class Console(DirectObject):
	
	def __init__(self, BMLR):
		self.__BMLR = BMLR
		self.__ConsoleOn = False
		self.__ConsoleBuffer = [] # The list of all lines in the console buffer
		self.__ConsoleLines = 0 # The size of the buffer that is shown at each time, this is recalculated in Resize()
		self.__ConsoleOffset = 0 # Scrolling up and down the console modifies the offset
			
		self.__CommandBuffer = [] # The list of all commands entered into the console
		self.__CommandOffset = 0 # Scrolling up and down the 
		
		self.KeyBoardOffFunc = None
		self.KeyBoardOnFunc = None
		
		self.__InitConsole()		
		
		self.StdOutHook()
		
		# Redraw on window resize
		self.accept("aspectRatioChanged", self.Resize)
		self.accept("wheel_up", self.Scroll, ["up"])
		self.accept("page_up", self.Scroll, ["up"])
		self.accept("wheel_down", self.Scroll, ["down"])
		self.accept("page_down", self.Scroll, ["down"])		
		self.accept("tab", self.Toggle)
		self.accept(self.GetConsoleAcceptEvent(), self.ConsoleAccept)
		
		if (self.__BMLR.Camera != None):
			self.KeyBoardOffFunc = self.__BMLR.Camera.DropInput
			self.KeyBoardOnFunc = self.__BMLR.Camera.InitInput
	
	
	def StdOutHook(self):
		""" We hijack the standard out buffer that "print()" prints to, and make it appear in our console """
		
		# Hook into the standard out
		self.__StdOut = sys.stdout		
		sys.stdout = self
		
		# Hook into the standard error out
		self.__StdErr = sys.stderr
		sys.stderr = self	
	
	def Scroll(self, args = None):
		""" Scrolls up and down the console """
		
		if (len(self.__ConsoleBuffer) < self.__ConsoleLines):
			return
		
		if (args == "up"):
			self.__ConsoleOffset += 3
		elif(args == "down"):
			self.__ConsoleOffset -= 3
		
		if (self.__ConsoleOffset < 0):
			self.__ConsoleOffset = 0
			
		if (self.__ConsoleOffset > len(self.__ConsoleBuffer) - self.__ConsoleLines):
			self.__ConsoleOffset = len(self.__ConsoleBuffer) - self.__ConsoleLines
			
		self.DrawText()
		
	def DrawText(self):
		""" Draws everything in the console buffer """
		
		self.__ConsoleText.clearText()
		
		if (len(self.__ConsoleBuffer) < self.__ConsoleLines):
			i = self.__ConsoleLines - len(self.__ConsoleBuffer)
			self.__ConsoleText.appendText("\n"*i)
			
		for i in range(0, self.__ConsoleLines):
			if (len(self.__ConsoleBuffer) >= (self.__ConsoleLines - i)):
				
				lineNo = len(self.__ConsoleBuffer) - (self.__ConsoleLines - i) - self.__ConsoleOffset
				
				self.__ConsoleText.appendText(self.__ConsoleBuffer[lineNo] + "\n")		
		
		
	def Resize(self):
		""" Resizes the console components after the aspect ratio is changed """
		
		hasp = base.getAspectRatio()
		vasp = 1
		
		if (hasp < 1):
			hasp = 1
			vasp = 1/base.getAspectRatio()
		
		self.__ConsoleTextBackNP.setScale(-hasp, 1, vasp)
		self.__InputNP.setPos(-hasp+0.01, 0, 0)
		self.__ConsoleTextNP.setPos(-hasp, 0, vasp-0.05)
		
		self.__ConsoleText.setMaxRows(int(19.5 * vasp))
		self.__ConsoleLines = self.__ConsoleText.getMaxRows()

	def NavigateConsole(self, eventArg):
		""" Event handler for up/down arrow key presses.
			Has the same functionality as a normal DOS shell,
			navigates through the command history"""
		
		# Couple of initial checks to see if we should do anything at all
		if (not (eventArg == "up" or eventArg == "down")):
			return		
		if (len(self.__CommandBuffer) == 0):
			return
		
		if (eventArg == "up"):
			self.__CommandOffset += 1			
		elif (eventArg == "down"):
			self.__CommandOffset -= 1
		
		# Don't go below 0 because nothing exists
		if (self.__CommandOffset < 1):
			self.__CommandOffset = 1
			self.__Input.setText("")
		else:
			# Make sure we don't go above bounds either
			if (self.__CommandOffset > len(self.__CommandBuffer)):
				self.__CommandOffset = len(self.__CommandBuffer)
	
			# And finally overwrite whatever text is in the console input box
			self.__Input.setText(self.__CommandBuffer[len(self.__CommandBuffer) - self.__CommandOffset])
			self.__Input.setCursorPosition(len(self.__Input.getText()))
		
	def write(self, text):
		""" stdout print() handler """
		
		self.__StdOut.write(text)
		self.ConsoleAppendText(text.replace("\n", ""))
		self.__ConsoleOffset = 0
		
	def __InitConsole(self):
		""" Initializes the nodes/nodepaths for the console """
	
		# Set up a container object for the console
		self.__ConsoleNP = aspect2d.attachNewNode("BML_ConsolePanel")
		self.__ConsoleNP.setTransparency(TransparencyAttrib.MAlpha)
		self.__ConsoleNP.hide()

		# Set up the background of the console
		self.__ConsoleTextBack = CardMaker("BML_ConsoleBackground")
		self.__ConsoleTextBack.setColor(0, 0, 0, 0.3)
		self.__ConsoleTextBack.setFrame(-1, 1, 0, 1)
		self.__ConsoleTextBackNP = self.__ConsoleNP.attachNewNode(self.__ConsoleTextBack.generate())

		# Set up an image for the background of the console
		self.__ConsoleText = TextNode("BML___ConsoleText")
		self.__ConsoleText.setAlign(TextNode.ALeft)		
		self.__ConsoleText.setMaxRows(int(19 * self.__ConsoleText.getLineHeight()))

		self.__ConsoleTextNP = self.__ConsoleNP.attachNewNode(self.__ConsoleText)
		self.__ConsoleTextNP.setScale(0.04)
		
		# Set up the text input for the console
		self.__Input = PGEntry("BML_ConsoleInput")
		self.__Input.setup(200, 1)
		self.__Input.setMaxChars(0)
		self.__Input.setMaxWidth(0)
		self.__Input.setBlinkRate(3)	
		
		self.__InputNP = self.__ConsoleNP.attachNewNode(self.__Input)
		self.__InputNP.setScale(0.04)
		
		self.SwitchOff()
		self.Resize()

	def GetConsoleAcceptEvent(self):
		return self.__Input.getAcceptEvent(KeyboardButton.enter())
		
	def ProcessCommand(self, command):
		""" Handles a command input by user """
		
		# Remove whitespace
		command = strip(command)	
		
		# Add the command to the stack for up/down arror functionality
		self.__CommandBuffer.append(command)
		
		closeConsole = False
		if (command.endswith("'''")):
			command = command[:-3]
			closeConsole = True
		
		if (command.startswith(">")):
			# This is a SmartBody command, send it to smartbody!
			command = command.replace(">", "", 1)
			print("> " + command)
			self.__BMLR.GetNet().SendCommand(command)
			
		elif (command.startswith("%")):
			# This is a python command executed with eval
			command = command.replace("%", "", 1)
			print("% " + command)
			
			try:
				exec(command)
			except:
				print sys.exc_info()
				
		else:
			# This is an internal command
			print("# " + command)

			if (command.lower() == "q" or command.lower() == "quit"):
				sys.exit()
			elif (command.lower() == "sbm"):
				self.__BMLR.SbmStart()
			
			messenger.send("CMD_" + command)
		
		if (closeConsole):
			self.Toggle()
			
	def ConsoleAppendText(self, text):
		""" Appends a line of text to the console """
		
		if (len(text) > 0):
			self.__ConsoleBuffer.append(text)
			self.DrawText()
		
	def ConsoleAccept(self, args):
		""" Event handler for the enter key press in the console """
		
		self.__Input.setFocus(True)
		
		text = self.__Input.getText()
		text = strip(text)		
		
		if (len(text) > 0):
			self.__Input.setText("")
			self.__CommandOffset = 0
			self.ProcessCommand(text)		
		
	def Toggle(self):
		""" Toggles console on and off """
		
		if (self.__ConsoleOn):
			self.SwitchOff()
		else:
			self.SwitchOn()
		
	def SwitchOn(self):
		""" Enables the console """
		
		self.__ConsoleOn = True		
		self.__ConsoleNP.show()		
		self.__Input.setActive(True)
		self.__Input.setAcceptEnabled(True)
		self.__Input.setFocus(True)
		if (self.KeyBoardOffFunc != None):
			self.KeyBoardOffFunc()
		self.__CommandOffset = 0
		
		self.accept("arrow_up", self.NavigateConsole, ["up"])
		self.accept("arrow_down", self.NavigateConsole, ["down"])
	
	def SwitchOff(self):
		""" Disables the console """
		
		self.__ConsoleOn = False
		self.__ConsoleNP.hide()
		self.__Input.setText("")
		self.__Input.setFocus(False)
		self.__Input.setActive(False)
		self.__Input.setAcceptEnabled(False)
		if (self.KeyBoardOnFunc != None):
			self.KeyBoardOnFunc()
			
		self.ignore("arrow_up")
		self.ignore("arrow_down")
