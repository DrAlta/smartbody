def point(chrName, target):
	''' Character name, Character/Pawn name '''
	bml.execBML(chrName, '<sbm:reach sbm:action="point-at" sbm:reach-duration="1" target="' + target + '"/>')
	
def touch(chrName, target, locomotion=False):
	''' Character name, Character/Pawn name, Whether to move to target '''
	# Left/right hand
	bml.execBML(chrName, '<sbm:reach sbm:action="touch" sbm:reach-duration="0.1" target="' 
						  + target + '" sbm:use-locomotion="' + str(locomotion).lower() + '"/>')

def pickUp(chrName, target, locomotion=False, finish=True):
	''' Character name, Character/Pawn name, Whether to move to target, whether to complete the reach '''
	bml.execBML(chrName, '<sbm:reach sbm:action="pick-up" sbm:reach-finish="' + str(finish) + '" target="' 
						  + target + '" sbm:use-locomotion="' + str(locomotion).lower() + '"/>')
	
def putDown(chrName, target='', locomotion=False):
	''' Character name, Character/Pawn name/target='100 100 100' '''
	''' Only SrVec, not characters'''
	if target in scene.getCharacterNames() or target in scene.getPawnNames():
		print 'exists'
		bml.execBML(chrName, '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" target="' 
							 + target + '" sbm:use-locomotion="true"/>')
	elif target != '':
		bml.execBML(chrName, '<sbm:reach sbm:action="put-down" sbm:reach-finish="true" sbm:target-pos="' 
							 + target + '" sbm:use-locomotion="' + str(locomotion).lower() + '"/>')
	else:
		bml.execBML(chrName, '<sbm:reach sbm:action="put-down" sbm:reach-finish="true"/>')
	