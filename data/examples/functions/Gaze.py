def gaze(chrName, target):
	''' Character name, target to gaze at '''
	bml.execBML(chrName, '<gaze sbm:joint-range="EYES NECK" target="' + target + '"/>')
	
def stopGaze(chrName):
	''' Name of character to stop gazing '''
	bml.execBML(chrName, '<gaze sbm:joint-range="EYES NECK" target="' + chrName + '"/>')