def initGestureMap():
	gMapManager = getScene().getGestureMapManager()
	for name in scene.getCharacterNames():
		gMap = gMapManager.createGestureMap(name)
		# gMap.addGestureMapping(animationName, lexeme, type, hand, style, posture)
		
		gMap.addGestureMapping('HandsAtSide_Arms_Sweep', 'SWEEP', '', 'BOTH_HANDS', '', 'HandsAtSide_Motex')
		gMap.addGestureMapping('HandsAtSide_RArm_GestureYou', 'YOU', '', 'RIGHT_HAND', '', 'HandsAtSide_Motex')
		gMap.addGestureMapping('LHandOnHip_Arms_GestureWhy', 'WHY', '', 'BOTH_HANDS', '', 'LHandOnHip_Motex')
		gMap.addGestureMapping('LHandOnHip_RArm_GestureOffer', 'OFFER', '', 'RIGHT_HAND', '', 'LHandOnHip_Motex')
		gMap.addGestureMapping('LHandOnHip_RArm_SweepRight', 'SWEEP', '', 'RIGHT_HAND', '', 'LHandOnHip_Motex')
		
		gMap.addGestureMapping('HandsAtSide_RArm_GestureYou', 'YOU', '', 'RIGHT_HAND', '', 'ChrUtah_Idle003')
		gMap.addGestureMapping('LHandOnHip_RArm_SweepRight', 'SWEEP', '', 'RIGHT_HAND', '', 'ChrUtah_Idle003')
		gMap.addGestureMapping('LHandOnHip_Arms_GestureWhy', 'WHY', '', 'BOTH_HANDS', '', 'ChrUtah_Idle003')
		gMap.addGestureMapping('LHandOnHip_RArm_GestureOffer', 'OFFER', '', 'RIGHT_HAND', '', 'ChrUtah_Idle003')
		gMap.addGestureMapping('ChrUtah_IndicateThereLeft001', 'LEFT', '', 'LEFT_HAND', '', 'ChrUtah_Idle003')
		gMap.addGestureMapping('ChrUtah_IndicateThereRight001', 'RIGHT', '', 'RIGHT_HAND', '', 'ChrUtah_Idle003')

def playGesture(chrName, gestureName, hand):
	''' Chararacter name to play gesture, lexeme of gesture, hand to use 
		Example: playGesture('brad', 'YOU', 'RIGHT_HAND') '''
	bml.execBML(chrName, '<gesture lexeme="' + gestureName + '" mode="' + hand + '"/>')
	