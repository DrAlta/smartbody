def gaze(chrName, target, speed=1000):
	''' Character name, Target to gaze at, Speed of action '''
	if target in scene.getCharacterNames() or target in scene.getPawnNames():
		bml.execBML(chrName, '<gaze sbm:joint-range="EYES NECK" sbm:joint-speed="' + str(speed) + '" target="' + target + '"/>')
	#Else is vector position
	else:
		bml.execBML(chrName, '<gaze sbm:joint-range="EYES NECK" sbm:joint-speed="' + str(speed) + '" sbm:target-pos="' + target + '"/>')
		#bml.execBML(chrName, '<gaze sbm:joint-range="EYES NECK" sbm:joint-speed="' + str(speed) + '" sbm:target-pos="' + target + '" target="' + chrName + '"/>')
	
def stopGaze(chrName, start, speed=1000):
	''' Name of character to stop gazing, Delay of action, Speed of action '''
	bml.execBML(chrName, '<gaze sbm:joint-range="EYES NECK" start="' + str(start) + '" sbm:joint-speed="' + str(speed) + '" target="' + chrName + '"/>')