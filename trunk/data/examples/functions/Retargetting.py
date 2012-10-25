# Run retarget script for the functions
scene.run('motion-retarget2.py')

'''
motionName = 'ChrUtah_Relax001_CrouchProtectHead_center'
srcSkelName = 'test_utah.sk'
tgtSkelName = 'common-test.sk'
outDir = '../../../../data/sbm-common/common-sk/retargetMotion/'
scrMapName = ''
tgtMapName = ''
'''
outputName = 'None'

def autoRetarget(motionName, srcSkelName, tgtSkelName, outDir):
	retargetMotionWithGuessMap(motionName, srcSkelName, tgtSkelName, outDir)
	global outputName
	outputName = getOutputName()
	
def getOutput():
	return outputName
	