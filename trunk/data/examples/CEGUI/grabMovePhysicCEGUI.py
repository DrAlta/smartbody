import random
print "|--------------------------------------------|"
print "|            Starting GrabTouchCEGui Demo             |"
print "|--------------------------------------------|"

# Add asset paths
scene.addAssetPath('mesh', 'mesh')
scene.addAssetPath('motion', 'ChrBrad')
scene.addAssetPath('motion', 'ChrRachel')
scene.addAssetPath("script", "behaviorsets")
scene.addAssetPath('script', 'scripts')
scene.loadAssets()

# Set scene parameters and camera
print 'Configuring scene parameters and camera'
scene.setScale(1.0)
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 3.45, 5.02)
camera.setCenter(0, 2.46, 3.29)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, -2, 0))

# Set joint map for Brad and Rachel
print 'Setting up joint map for Brad and Rachel'
scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')
bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse('ChrBrad')


# Setting up Brad
print 'Setting up Brad'

baseName = 'ChrBrad%s' % 1
brad = scene.createCharacter(baseName, '')
bradSkeleton = scene.createSkeleton('ChrBrad.sk')
brad.setSkeleton(bradSkeleton)
brad.createStandardControllers()
	# Set deformable mesh
brad.setDoubleAttribute('deformableMeshScale', 0.01)
brad.setStringAttribute('deformableMesh', 'ChrBrad')
	# Play idle animation
bml.execBML(baseName, '<body posture="ChrBrad@Idle01"/>')
scene.run('BehaviorSetReaching.py')
setupBehaviorSet()
retargetBehaviorSet(baseName)		

scene.run('BehaviorSetMaleMocapLocomotion.py')
setupBehaviorSet()
retargetBehaviorSet(baseName)


#set gravity correctly
phyManager = scene.getPhysicsManager()
phyManager.getPhysicsEngine().setDoubleAttribute('gravity',9.80)


# Setting character positions
print 'Setting character positions'
scene.getCharacter('ChrBrad1').setPosition(SrVec(0, 0, 1))


# Set up pawns in scene
print 'Adding pawns to scene'
numPawns = 0

baseName = 'phy%s' % numPawns
shapeList = ['sphere', 'box', 'capsule']
size = random.randrange(5, 30)
pawn = scene.createPawn(baseName)
pawn.setStringAttribute('collisionShape', random.choice(shapeList))
pawn.getAttribute('collisionShapeScale').setValue(SrVec(0.1, 0.1, 0.1))
pawn.setPosition(SrVec(-0.4, 6.6, 1.35))
numPawns += 1

	
#set first pawn to yellow
baseName = 'phy%s' % 0
pawn = scene.getPawn(baseName)
pawn.getAttribute('color').setValue(SrVec( 1, 1, 0))
				
# Append all pawn to list
pawnList = []
for name in scene.getPawnNames():
	if 'phy' in name:
		pawnList.append(scene.getPawn(name))
		print scene.getPawn(name)
		

# Setup pawn physics	
print 'Setting up pawn physics'
phyManager = scene.getPhysicsManager()
phyManager.getPhysicsEngine().setBoolAttribute('enable', True)

for pawn in pawnList:
	pawn.getAttribute('createPhysics').setValue()
	phyManager.getPhysicsPawn(pawn.getName()).setDoubleAttribute('mass', 1)

for pawn in pawnList:
	pawn.setBoolAttribute('enablePhysics', True)
	
	
# Turn on GPU deformable geomtery for all
for name in scene.getCharacterNames():
	scene.command("char %s viewer deformableGPU" % name)

	
	
# ---- pawn: pawn3 mesh pawn
baseName = 'pawn3'
pawn = scene.createPawn(baseName)

pawn.setPosition(SrVec( 5, 0, 5))
pawn.setStringAttribute("mesh" , "../../../../data/mesh/shelf.dae")
pawn.setDoubleAttribute("meshScale",0.15)
pawn.setDoubleAttribute('rotY' , 90)
	
	
# ---- pawn: pawnTop
baseName = 'pawnTop'
pawn = scene.createPawn(baseName)
pawn.setStringAttribute('collisionShape', 'box')
pawn.getAttribute('collisionShapeScale').setValue(SrVec( 0.2, 0.02, 0.5))
pawn.setPosition(SrVec( 4.93, 1.22, 4.93))
pawn.getAttribute('createPhysics').setValue()



# ---- pawn: pawnMid
baseName = 'pawnMid'
pawn = scene.createPawn(baseName)
pawn.setStringAttribute('collisionShape', 'box')
pawn.getAttribute('collisionShapeScale').setValue(SrVec( 0.2, 0.02, 0.5))
pawn.setPosition(SrVec( 4.93, 0.62, 4.93))
pawn.getAttribute('createPhysics').setValue()



	
# ---- pawn: pawnBot
baseName = 'pawnBot'
pawn = scene.createPawn(baseName)
pawn.setStringAttribute('collisionShape', 'box')
pawn.getAttribute('collisionShapeScale').setValue(SrVec( 0.2, 0.02, 0.5))
pawn.setPosition(SrVec( 4.93, 0.09, 4.93))
pawn.getAttribute('createPhysics').setValue()



curTime = 0
lastPass =0
delay = 3
onAction =False


class ReachDemo(SBScript):
	def update(self, time):
		global curTime,lastPass,delay,onAction
		curTime = time
		
		#print onAction
		
		if(not onAction):
			lastPass=curTime
			
		
		if(curTime-lastPass > delay):
			print'finish counting delay'
			lastPass = curTime
			onAction=False
		
	
	

# Run the update script
scene.removeScript('reachdemo')
reachdemo = ReachDemo()
scene.addScript('reachdemo', reachdemo)


currentPawnNum=0;
grabbed = False
released=True
canOtherAction =True
class ReachingHandler(SBEventHandler):
	def executeAction(self, ev):
		params = ev.getParameters()
		global grabbed ,released , canOtherAction  , currentPawnNum
		if 'ChrBrad1' in params:
			if 'pawn-attached' in params and not grabbed :
				grabbed = True;
				released = False
				canOtherAction=True
						
			elif 'pawn-released' in params and grabbed :
				grabbed = False
				canOtherAction =True
				baseName = 'phy%s' % (currentPawnNum - 1)
				pawn = scene.getPawn(baseName)	
				#pawn.getAttribute('color').setValue(SrVec( 1, 0, 0))
				baseName = 'phy%s' % (currentPawnNum)
				pawn = scene.getPawn(baseName)	
				pawn.getAttribute('color').setValue(SrVec( 1, 1, 0))
				
		
		
		
		
evtMgr = scene.getEventManager()
reachingHdl = ReachingHandler()
evtMgr.addEventHandler('reachNotifier', reachingHdl)





print 'Setting up GUI'
scene.run('GUIUtil.py')
gui = GUIManager()


speechTextBox = gui.createStaticText('editBox', "number of Pawn:"+str(numPawns), 100, 15, 300)
class GUIHandler:
	def __init__(self):
		print "init"
		

	def handleTouchButton(self,args):		
		global onAction, delay,canOtherAction ,currentPawnNum
		
		if(not onAction and canOtherAction):
			print 'try touching pawn'
			baseName = 'phy%s' % currentPawnNum
			onAction = True
			delay = 2
			bml.execBML('ChrBrad1', '<sbm:reach sbm:action="touch" target="'+baseName+'" sbm:reach-finish="true"/>')
			

		
	def handlePointButton(self,args):
		
		global onAction , delay,canOtherAction,currentPawnNum
		if(not onAction and canOtherAction):
			print 'try pointing pawn'
			baseName = 'phy%s' % currentPawnNum
			delay = 3
			bml.execBML('ChrBrad1', '<sbm:reach sbm:action="point-at" sbm:reach-duration="1" target="'+baseName+'"/>')
			bml.execBML('ChrBrad1', '<gaze target="touchPawn" sbm:joint-range="EYES NECK"/>')
			onAction = True
			
		
	def handleGrabButton(self,args):
		global onAction , delay,currentPawnNum,grabbed,canOtherAction
		
		if(not onAction and not grabbed):
			print 'grab'
			canOtherAction=False
			onAction = True
			delay = 2
			baseName = 'phy%s' % currentPawnNum
			bml.execBML('ChrBrad1', '<sbm:reach sbm:action="pick-up" sbm:reach-finish="true" sbm:reach-duration="2" target="'+baseName+'" sbm:use-locomotion="true"/>')	
			bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')		
		
	def handleGrabMoveButton(self,args):
		
		global pawn1 , onAction , delay,currentPawnNum ,numPawns,grabbed,released,canOtherAction
		if(not onAction and grabbed and not released):
			print 'putting it somewhere else'
			baseName = 'phy%s' % currentPawnNum
			if(currentPawnNum <=numPawns):
				currentPawnNum += 1
				
			canOtherAction = False
			released = True
			layer = random.randint(1, 3)
			
			if(layer is 1):
				bml.execBML('ChrBrad1', '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' + '%s %s %s' % (4.9, 1.6, 4.9) + '" sbm:use-locomotion="true"/>')
				bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')
			if (layer is 2):
				bml.execBML('ChrBrad1', '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' + '%s %s %s' % (4.9, 0.9, 4.9) + '" sbm:use-locomotion="true"/>')
				bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')
			if (layer is 3):
				bml.execBML('ChrBrad1', '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' + '%s %s %s' % (4.9, 0.3, 4.9) + '" sbm:use-locomotion="true"/>')
				bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')
				
			delay = 3
			onAction=True
			
			
	def handleGrabMoveTopButton(self,args):	
		global pawn1 , onAction , delay,currentPawnNum ,numPawns,grabbed,released,canOtherAction
		if(not onAction and grabbed and not released):
			print 'putting it somewhere else'
			baseName = 'phy%s' % currentPawnNum
			if(currentPawnNum <=numPawns):
				currentPawnNum += 1
			canOtherAction = False
			released = True
			bml.execBML('ChrBrad1', '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' + '%s %s %s' % (4.9, 1.6, 4.9) + '" sbm:use-locomotion="true"/>')
			bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')	
			delay = 3
			onAction=True
			
		
	
	def handleGrabMoveMidButton(self,args):
		global pawn1 , onAction , delay,currentPawnNum ,numPawns,grabbed,released,canOtherAction
		if(not onAction and grabbed and not released):
			print 'putting it somewhere else'
			baseName = 'phy%s' % currentPawnNum
			if(currentPawnNum <=numPawns):
				currentPawnNum += 1
			canOtherAction = False
			released = True
			bml.execBML('ChrBrad1', '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' + '%s %s %s' % (4.9, 0.9, 4.9) + '" sbm:use-locomotion="true"/>')
			bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')	
			delay = 3
			onAction=True	
	
	
	def handleGrabMoveBotButton(self,args):
		global pawn1 , onAction , delay,currentPawnNum ,numPawns,grabbed,released,canOtherAction
		if(not onAction and grabbed and not released):
			print 'putting it somewhere else'
			baseName = 'phy%s' % currentPawnNum
			if(currentPawnNum <=numPawns):
				currentPawnNum += 1
			canOtherAction = False
			released = True
			bml.execBML('ChrBrad1', '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' + '%s %s %s' % (4.9, 0.3, 4.9) + '" sbm:use-locomotion="true"/>')
			bml.execBML('ChrBrad1', '<gaze target="'+baseName+'" sbm:joint-range="EYES NECK"/>')
			delay = 3
			onAction=True
	
		
	def addPawnButton(self,args):
		global numPawns
		baseName = 'phy%s' % numPawns
		shapeList = ['sphere', 'box', 'capsule']
		size = random.randrange(5, 30)
		
		
		pawn = scene.createPawn(baseName)
		pawn.setPosition(SrVec(0, 6.6, 0))
		pawn.setStringAttribute('collisionShape', random.choice(shapeList))
		pawn.getAttribute('collisionShapeScale').setValue(SrVec(0.1, 0.1, 0.1))
		
		numPawns += 1
		speechTextBox.setText("number of Pawn:"+str(numPawns))
		pawnList.append(scene.getPawn(baseName))
		scene.getPawn(baseName).getAttribute('createPhysics').setValue()
		phyManager.getPhysicsPawn(scene.getPawn(baseName).getName()).setDoubleAttribute('mass', 1)
		
		scene.getPawn(baseName).setBoolAttribute('enablePhysics', True)
		
		

	
		

	

		
guiHandler = GUIHandler()	
touchButton = gui.createButton('touchBtn','Touch object')
touchButton.subscribeEvent(PushButton.EventClicked, guiHandler.handleTouchButton)

pointButton = gui.createButton('pointBtn','Point at Object')
pointButton.subscribeEvent(PushButton.EventClicked, guiHandler.handlePointButton)

grabButton = gui.createButton('grabBtn','Grab Object')
grabButton.subscribeEvent(PushButton.EventClicked, guiHandler.handleGrabButton)

grabMoveButton = gui.createButton('grabMoveBtn','Place on shelf')
grabMoveButton.subscribeEvent(PushButton.EventClicked, guiHandler.handleGrabMoveButton)

grabMoveTopButton = gui.createButton('grabMoveTopBtn','Place on TopShelf')
grabMoveTopButton.subscribeEvent(PushButton.EventClicked, guiHandler.handleGrabMoveTopButton)

grabMoveMidButton = gui.createButton('grabMoveMidBtn','Place on Midshelf')
grabMoveMidButton.subscribeEvent(PushButton.EventClicked, guiHandler.handleGrabMoveMidButton)

grabMoveBotButton = gui.createButton('grabMoveBotBtn','Place on Bottomshelf')
grabMoveBotButton.subscribeEvent(PushButton.EventClicked, guiHandler.handleGrabMoveBotButton)

addPawnButton = gui.createButton('addPawnBtn','Add object')
addPawnButton.subscribeEvent(PushButton.EventClicked, guiHandler.addPawnButton)


