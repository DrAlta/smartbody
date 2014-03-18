# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # 
# #  Panda BML Realizer 	http://cadia.ru.is/projects/bmlr/
# #
# #  Bjarni Thor Arnason 	bjarnia@gmail.com
# #  Aegir Thorsteinsson 	aegirth@gmail.com
# #
# #  Copyright(c)2007 Center for Analysis and Design of Intelligent Agents
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
from direct.distributed.PyDatagram import PyDatagram

# Python base imports
from socket import *
from struct import *
from string import * 
import sys

# PandaBMLR imports
from Converter import *


TCP_PORT_DEFAULT = 15102
UDP_PORT_DEFAULT = 15100

class NetHandler(DirectObject):
	""" Handles all incoming and outgoing network communications """
	
	def __init__(self, BMLR):
		self.__BMLR = BMLR
		self.__Scene = BMLR.GetScene()
		self.__Manager = None
		self.__Listener = None
		self.__Reader = None
		self.__UdpReader = None
		self.__Connection = None
		self.__UdpSocket = None
		self.__UdpDataTotal = 0 # in kilobytes
		self.__Connected = False
		self.__HostName = ""
		
		# Basic network stuff
		self.__Manager = QueuedConnectionManager()
		# Listen for new connections
		self.__Listener = QueuedConnectionListener(self.__Manager, 0)		

		self.ListenTCP()
		self.ListenUDP()
	
	def GetDataInKbs(self):
		ret = self.__UdpDataTotal
		self.__UdpDataTotal = 0
		return ret
	
	def GetHostAddress(self):
		return self.__HostName
		
	def IsConnected(self):
		return self.__Connected
		
	def ListenTCP(self):
		""" Sets up a TCP listener """
		
		port = ConfigVariableInt("TcpPort", TCP_PORT_DEFAULT).getValue()
		try:
			self.__Reader = QueuedConnectionReader(self.__Manager, 0)
			self.__Reader.setRawMode(True)
			
			# Sends data
			self.__writer = ConnectionWriter(self.__Manager, 0)
			self.__writer.setRawMode(True)
			
			# Open a socket
			self.__Connection = self.__Manager.openTCPServerRendezvous(port, 5)
			
			# Listen to new connections
			self.__Listener.addConnection(self.__Connection)
			
			print ("Started TCP listener on port: " + str(port))
		except:
			print("Error initializing TCP listener on port: " + str(port) + ", perhaps BMLR is already running?")
			sys.exit()
		
	def ListenUDP(self):
		""" Sets up a UDP listener """
		
		port = ConfigVariableInt("UdpPort", UDP_PORT_DEFAULT).getValue()
		try:
			self.__UdpReader = QueuedConnectionReader(self.__Manager, 0)
			self.__UdpReader.setRawMode(True)
			self.__UdpReader.addConnection(self.__Manager.openUDPConnection(port))
			print ("Started UDP listener on port: " + str(port))
		except:
			print("Error initializing UDP listener on port: " + str(port) + ", perhaps BMLR is already running?")
			sys.exit()		

	def CheckTCP(self):        
		""" Polls the TCP listener to see if there is any data to be received,
			and sends the data to the appropriate handler if there is any."""
			
		# Check for data
		if (self.__Reader.dataAvailable()):
			dg = NetDatagram()
			if (self.__Reader.getData(dg)):
				self.__Scene.ProcessCommands(dg.getMessage())

		# Check for new connections
		if (self.__Listener.newConnectionAvailable()):
			conn = PointerToConnection()
			if (self.__Listener.getNewConnection(conn)):
				self.__Reader.addConnection(conn.p())
				self.__HostName = conn.p().getAddress()
				self.__Connection = conn
				self.__Connected  = True
				self.__Scene.OnConnect()				
			
		# Check if connection is OK
		if  (self.__Connected):
			ok = self.__Reader.isConnectionOk(self.__Connection.p())
			if (not ok):
				self.__Connected = False
				self.__HostName = ""
				messenger.send("SbmDisconnect")
				taskMgr.doMethodLater(1, self.__BMLR.GetScene().OnDisconnect, "SmartBodySceneDestruct")
				
	def CheckUDP(self):
		""" Polls the UDP listener to see if there is any data to be received,
			and sends the data to the appropriate handler if there is any"""
		
		if (self.__UdpReader.dataAvailable()):
			dg = Datagram()
			while (self.__UdpReader.getData(dg)):
				
				self.__UdpDataTotal += dg.getLength() / 1024.0
				
				packetId = unpack("i", dg.getMessage()[:4])[0]
				
				#print "PacketId = " + str(packetId)
				if (packetId == 16):
					# 0 = time, 1 = charid, 2 = numBoneRotations
					BulkBoneRotations = unpack("iii", dg.getMessage()[4:16])
					BulkBoneItems = []
					count = 0
					for i in range(16, dg.getLength(), 20):
						BulkBoneItems.append(unpack("iffff", dg.getMessage()[i:i+20]))
						count = count + 1
					#print "Got " + str(BulkBoneRotations[2]) + " Rotations " + str(count)
					self.__Scene.ProcessJointRotations(BulkBoneRotations[0], BulkBoneRotations[1], BulkBoneItems)
				elif (packetId == 17):
					# 0 = time, 1= charid, 2 = numBonePositions
					BulkBonePositions = unpack("iii", dg.getMessage()[4:16])
					# 0 = boneId, 1 = pos_x, 2 = pos_y, 3 = pos_z
					BulkBoneItems = []
					count = 0
					for i in range(16, dg.getLength(), 16):
						BulkBoneItems.append(unpack("ifff", dg.getMessage()[i:i+16]))
						count = count + 1
					#print "Got " + str(BulkBonePositions[2]) + " Positions " + str(count) + " at time " + str(BulkBonePositions[0]) + " for character " + str(BulkBonePositions[1])
					self.__Scene.ProcessJointPositions(BulkBonePositions[0], BulkBonePositions[1], BulkBoneItems)					
				elif (packetId == 18):
					continue # This is not handled at the moment
					# 0 = charId, 1 = visemeId, 2 = weight, 3 = blendTime
					VisemeBlendData = unpack("iiff", dg.getMessage()[4:20])
				elif (packetId == 4):
					# String command
					msg = dg.getMessage()[4:]
					msg = msg.split(";")[0]
					self.__Scene.ProcessCommands(msg)
				else:
					print("Unknown UDP packet received. ID == " + str(packetId) + "\n")
		
	def SendCommand(self, command):
		""" Sends a command directly to SmartBody, as if it was typed in the SmartBody console """
		
		if (not self.__Connected):
			print("Error sending command to SmartBody, connection is most likely down")
			return
		
		try:
			print(">> " + command)
			self.__writer.send(Datagram(command + "\r\r\r"), self.__Connection.p());
		except:
			print("Error sending command to SmartBody, connection is most likely down")