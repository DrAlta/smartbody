# Retrieve blend manager
blendManager = scene.getBlendManager()

def create0DBlend(blendID, motionList):
	''' Activated using <blend name="blendID"/>'''
	blend0 = blendManager.createBlend0D(blendID)
	blend0.addMotion(motionList[0])

def create1DBlend(blendID, motionList):
	''' Blend ID, Motion list(String motion name) 
		Activated using <blend name="blendID" x="15"/> '''
	motion1 = motionList[0]
	motion2 = motionList[1]
	
	m1 = scene.getMotion(motion1)
	m2 = scene.getMotion(motion2)
	
	blend1 = blendManager.createBlend1D(blendID)
	blend1.addMotion(motion1, 0)
	blend1.addMotion(motion2, 1)
	
	motions = StringVec()
	motions.extend([motion1, motion2])
	
	points = DoubleVec()
	points.extend([0, 0, 0.5, 0.7])
	points.extend([m1.getDuration(), m2.getDuration()])
	
	blend1.addCorrespondencePoints(motions, points)
	
def create2DBlend(blendID, motionList):
	''' BlendID, Motion list(String motion name)
		Activated using <blend name="blendID" x=".4" y=".7"/> '''
	motion1 = motionList[0]
	motion2 = motionList[1]
	motion3 = motionList[2]
	motion4 = motionList[3]
	
	m1 = scene.getMotion(motion1)
	m2 = scene.getMotion(motion2)
	m3 = scene.getMotion(motion3)
	m4 = scene.getMotion(motion4)
	
	blend2 = blendManager.createBlend2D(blendID)
	blend2.addMotion(motion1, 0, 0)
	blend2.addMotion(motion2, 1, 0)
	blend2.addMotion(motion3, 0, 1)
	blend2.addMotion(motion4, 1, 1)
	
	motions = StringVec()
	motions.extend([motion1, motion2, motion3, motion4])
	
	points = DoubleVec()
	points.extend([0, 0, 0, 0])
	points.extend([m1.getDuration(), m2.getDuration(), m3.getDuration(), m4.getDuration()])
	
	blend2.addCorrespondencePoints(motions, points)
	
	blend2.addTriangle(motion1, motion2, motion3)
	blend2.addTriangle(motion3, motion2, motion4)
	
def create3DBlend(blendID, motionList):
	''' BlendID, Motion list(String motion name)
		Activated using <blend name="blendID" x=".4" y=".7" z=".8"/> '''
	motion1 = motionList[0]
	motion2 = motionList[1]
	motion3 = motionList[2]
	motion4 = motionList[3]
	
	m1 = scene.getMotion(motion1)
	m2 = scene.getMotion(motion2)
	m3 = scene.getMotion(motion3)
	m4 = scene.getMotion(motion4)
	
	blend3 = blendManager.createBlend3D(blendID)
	blend3.addMotion(motion1, 0, 0, 0)
	blend3.addMotion(motion2, 0, 0, 1)
	blend3.addMotion(motion3, 1, 0, 1)
	blend3.addMotion(motion4, 0, 1, 0)
	
	motions = StringVec()
	motions.extend([motion1, motion2, motion3, motion4])
	
	points = DoubleVec()
	points.extend([0, 0, 0, 0])
	points.extend([m1.getDuration(), m2.getDuration(), m3.getDuration(), m4.getDuration()])
	
	blend3.addCorrespondencePoints(motions, points)
	blend3.addTetrahedron(motion1, motion2, motion3, motion4)