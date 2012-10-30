def initGestureMap():
	gMapManager = getScene().getGestureMapManager()
	for name in scene.getCharacterNames():
		gMap = gMapManager.createGestureMap(name)
		# gMap.addGestureMapping(animationName, lexeme, type, hand, style, posture)
		# NOTE: Can't loop because last index will overwrite the ones in front
		# Determine the posture to be used
		if 'utah' in name:
			posture = 'ChrUtah_Idle003'
		if 'brad' in name:
			posture = 'HandsAtSide_Motex'
		if 'elder' in name or 'doctor' in name:
			posture = 'LHandOnHip_Motex'
		else:
			print 'Character not found'
			break
		# Add gesture mapping
		gMap.addGestureMapping('HandsAtSide_RArm_GestureYou', 'YOU', '', 'RIGHT_HAND', '', posture)
		gMap.addGestureMapping('HandsAtSide_Arms_Sweep', 'SWEEP', '', 'BOTH_HANDS', '', posture)
		gMap.addGestureMapping('LHandOnHip_Arms_GestureWhy', 'WHY', '', 'BOTH_HANDS', '', posture)
		gMap.addGestureMapping('LHandOnHip_RArm_GestureOffer', 'OFFER', '', 'RIGHT_HAND', '', posture)
		gMap.addGestureMapping('ChrUtah_IndicateThereLeft001', 'LEFT', '', 'LEFT_HAND', '', posture)
		gMap.addGestureMapping('ChrUtah_IndicateThereRight001', 'RIGHT', '', 'RIGHT_HAND', '', posture)
		gMap.addGestureMapping('LHandOnHip_RArm_SweepRight', 'SWEEPRIGHT', '', 'RIGHT_HAND', '', posture)

def playGesture(chrName, gestureName, hand, start=0, ready=0, stroke=0, stroke_end=0):
	''' Chararacter name to play gesture, lexeme of gesture, hand to use, start point, ready point, stroke, stroke end
		Example: playGesture('brad', 'WHY', 'BOTH_HANDS', 0, 2)
				 playGesture('brad', 'YOU', 'RIGHT_HAND', stroke=2, stroke_end=4) '''
	# No pausing
	if stroke_end == 0:
		bml.execBML(chrName, '<gesture lexeme="' + gestureName + '" mode="' + hand + '" start="' + str(start) + '" ready="' + str(ready) + '"/>')
	# Pauses in between and perlin noise
	if stroke_end != 0:
		scale = 1.5
		bml.execBML(chrName, '<gesture lexeme="' + gestureName + '" mode="' + hand + '" stroke="' + str(stroke) + '" stroke_end="' + str(stroke_end) + \
					'" sbm:joint-range="l_shoulder r_shoulder spine1 spine4" sbm:frequency="' + str(0.03 * scale) + '" sbm:scale="' + str(0.02 * scale) +'"/>')
	#bml.execBML('brad', '<gesture id="a" lexeme="YOU" mode="RIGHT_HAND" stroke="0" stroke_end="2" sbm:joint-range="l_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')
	#bml.execBML('brad', '<gesture id="b" lexeme="RIGHT" mode="RIGHT_HAND" stroke="3" stroke_end="5" sbm:joint-range="l_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')
	#bml.execBML('brad', '<gesture id="c" lexeme="OFFER" mode="RIGHT_HAND" stroke="b:stroke_end" stroke_end="7" sbm:joint-range="r_shoulder" sbm:frequency="0.03" sbm:scale="0.02"/>')