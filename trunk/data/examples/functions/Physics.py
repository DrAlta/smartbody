physicsOn = True
phyManager = scene.getPhysicsManager()
scene.getPhysicsManager().getPhysicsEngine().setBoolAttribute('enable', physicsOn)

def printJointNames(chr):
	jointList = scene.getCharacter(chr).getSkeleton().getJointNames()	
	for joint in jointList:
		print joint
	
def setupPawnPhysics(chr):
	''' Name of pawn to set up physics for '''
	scene.getPawn(chr).getAttribute('createPhysics').setValue()
	
def setupCharacterPhysics(chr):
	''' Name of character to set up physics for '''
	# Retrieve physics manager and create physics character
	phyManager.createPhysicsCharacter(chr)
	
	jointName = 'base'
	# Get corresponding body link
	phyBodyLink = phyManager.getJointObj(chr, jointName)
	# Sets collision geometry
	phyBodyLink.setStringAttribute('geomType', 'capsule')
	# Sets size for collision geometry
	phyBodyLink.setVec3Attribute('geomSize', 10, 10, 10)
	
	mass = 50
	# Get corresponding body link
	phyBodyLink = phyManager.getJointObj(chr, jointName)
	# Set the mass for this body link
	phyBodyLink.setDoubleAttribute('mass', mass)
	
	# Get corresponding physics joint
	phyJoint = phyManager.getPhysicsJoint(chr, jointName)
	# Set rotation axis0 according to the 3 floats
	phyJoint.setVec3Attribute('axis0', 0, 0, 0)
	# Set maximum rotation axis, must be larger than zero
	phyJoint.setDoubleAttribute('axis0LimitHigh', 50)
	# Set minimum rotation axis, must be smaller than zero
	phyJoint.setDoubleAttribute('axis0LimitLow', -50)
	
def setupPhysicsParameters():
	gravity = 980
	dt = 0.0003
	kd = 2000
	ks = 23000
	maxSimTime = 0.01

	phyEngine = phyManager.getPhysicsEngine()
	phyEngine.setDoubleAttribute('gravity', gravity)
	phyEngine.setDoubleAttribute('dT', dt)
	phyEngine.setDoubleAttribute('Ks', ks)
	phyEngine.setDoubleAttribute('Kd', kd)
	phyEngine.setDoubleAttribute('MaxSimTime', maxSimTime)
	phyJoint = phyEngine.getPhysicsJoint(chr, 'spine1')
	phyJoint.setDoubleAttribute('KScale', kscale)
	
def constrainChr(chr, joint, object=''):
	''' Name of character, joint name of character, object name to act as constraint'''
	bodyLink = phyManager.getJointObj(chr, joint)
	bodyLink.setBoolAttribute('constraint', True)
	bodyLink = phyManager.getJointObj(chr, joint)
	bodyLink.setStringAttribute('constraintTarget', object)
	
def togglePhysics(chr, power=''):
	if power == '':
		isOn = phyManager.getPhysicsCharacter(chr).getAttribute('enable').getValue()
		phyManager.getPhysicsCharacter(chr).setBoolAttribute('enable', not isOn)
		phyManager.getPhysicsCharacter(chr).setBoolAttribute('usePD', not isOn)
	elif power == 'on':
		phyManager.getPhysicsCharacter(chr).setBoolAttribute('enable', True)
		phyManager.getPhysicsCharacter(chr).setBoolAttribute('usePD', True)
	elif power == 'off':
		phyManager.getPhysicsCharacter(chr).setBoolAttribute('enable', False)
		phyManager.getPhysicsCharacter(chr).setBoolAttribute('usePD', False)
	
def togglePawnPhysics(chr):
	isOn = scene.getPawn(chr).getAttribute('enablePhysics').getValue()
	scene.getPawn(chr).setBoolAttribute('enablePhysics', not isOn)
	