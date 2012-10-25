def initGestureMap():
	gMapManager = getScene().getGestureMapManager()
	for name in scene.getCharacterNames():
		gMap = gMapManager.createGestureMap(name)
		# gMap.addGestureMapping(animationName, type, lexeme, posture, hand)
		gMap.addGestureMapping('HandsAtSide_Arms_Sweep', '', 'SWEEP', 'HandsAtSide_Motex', 'BOTH_HANDS', '')
		gMap.addGestureMapping('HandsAtSide_RArm_GestureYou', '', 'YOU', 'LHandsAtSide_Motex', 'RIGHT_HAND', '')
		gMap.addGestureMapping('LHandOnHip_Arms_GestureWhy', '', 'WHY', 'LHandOnHip_Motex', 'BOTH_HANDS', '')
		gMap.addGestureMapping('LHandOnHip_RArm_GestureOffer', '', 'OFFER', 'LHandOnHip_Motex', 'RIGHT_HAND', '')
		gMap.addGestureMapping('LHandOnHip_RArm_SweepRight', '', 'SWEEP', 'LHandOnHip_Motex', 'RIGHT_HAND', '')
		gMap.addGestureMapping('LHandOnHip_RArm_SweepRight', '', 'YOU', 'ChrUtah_Idle003', 'RIGHT_HAND', '')

def playGesture(chrName, gestureName):
	bml.execBML(chrName, '<gesture lexeme="' + gestureName + '"/>')
	