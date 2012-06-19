print "|-------------------------------------------------|"
print "|  data/sbm-common/scripts/characterTest.py     |"
print "|-------------------------------------------------|"
import time

def testRetargetCharacter(charName, skelName):
	# test gesture
	bml.execBML(charName, '<body posture="'+ skelName +'HandsAtSide_Motex"/>')
	
	bml.execBML(charName, '<animation name="'+ skelName +'HandsAtSide_RArm_GestureYou"/>')		
	
	# test head nod
	bml.execBML(charName, '<head type="NOD" start="2"/>')
	bml.execBML(charName, '<head type="SHAKE" start="4"/>')
	bml.execBML(charName, '<head type="TOSS" start="6"/>')
	bml.execBML(charName, '<animation name="'+ skelName +'HandsAtSide_RArm_GestureYou"/>')	
	# test gaze
	pawnTargetName = charName+"target"
	testPawn = scene.getPawn(pawnTargetName)
	if (testPawn == None):
		testPawn = scene.createPawn(pawnTargetName);
	sbChar = scene.getCharacter(charName)	 
	position = sbChar.getPosition()
	print 'sbChar.getPosition=' + str(position.getData(0)) + ' ,' + str(position.getData(1)) + ', ' + str(position.getData(2))
	gazePos = SrVec(position.getData(0), sbChar.getHeight()*1.8 ,position.getData(2)+100);
	print 'sbChar.getPosition=' + str(gazePos.getData(0)) + ' ,' + str(gazePos.getData(1)) + ', ' + str(gazePos.getData(2))
	testPawn.setPosition(gazePos)
	bml.execBML(charName, '<gaze target="'+ pawnTargetName+ '" sbm:joint-speed="5200" start="8"/>')
	
	#gazePos.setData(1,0.0)
	#gazePawn.setPosition(gazePos)
	scene.commandAt(10.0, 'set pawn '+pawnTargetName+' world_offset y 0')
	#bml.execBML(charName, '<gaze target="'+ gazeTargetName+ '"/>')
	scene.commandAt(12.0, 'char ' + charName + ' gazefade out 1.5')
	# test reach
	reachPos = SrVec(position.getData(0)-30, sbChar.getHeight()*0.8 ,position.getData(2)+40);
	print 'sbChar.getPosition=' + str(reachPos.getData(0)) + ' ,' + str(reachPos.getData(1)) + ', ' + str(reachPos.getData(2))
	scene.commandAt(14.0, 'set pawn '+pawnTargetName+' world_offset x ' + str(reachPos.getData(0)) +' y ' + str(reachPos.getData(1)) + ' z ' + str(reachPos.getData(2)))
	scene.commandAt(14.1, 'bml char '+ charName+ ' <sbm:reach sbm:action="touch" sbm:reach-duration="0.5" target="'+pawnTargetName+'"/>')
	#bml.execBML(charName, '<sbm:reach sbm:action="touch" sbm:reach-duration="0.5" target="'+pawnTargetName+'" start="14"/>')
	
	reachPos = SrVec(position.getData(0)-30, sbChar.getHeight()*0.1 ,position.getData(2)+40);
	scene.commandAt(18.0, 'set pawn '+pawnTargetName+' world_offset x ' + str(reachPos.getData(0)) +' y ' + str(reachPos.getData(1)) + ' z ' + str(reachPos.getData(2)))	
	scene.commandAt(18.1, 'bml char '+ charName+ ' <sbm:reach sbm:action="touch" sbm:reach-duration="0.5" target="'+pawnTargetName+'"/>')	
	#/bml.execBML(charName, '<sbm:reach sbm:action="touch" sbm:reach-duration="0.5" target="'+pawnTargetName+'" start="18"/>')
	bml.execBML(charName, '<animation name="'+ skelName +'LHandOnHip_Arms_GestureWhy" start="21"/>')	
	# test locomotion
	bml.execBML(charName, '<locomotion target="'+str(position.getData(0)+500) +' '+ str(position.getData(2)+500) + '" start="22"/>')
	#scene.removePawn(pawnTargetName)	
	
	

	

	





