

def retargetMotion(motionName, srcSkelName, tgtSkelName, outDir) :	
	testMotion = scene.getMotion(motionName);
	outMotionName = tgtSkelName + motionName		
	existMotion = scene.getMotion(outMotionName)
	if existMotion != None : # do nothing if the retargeted motion is already there
		return	
		
	tgtSkel = scene.getSkeleton(tgtSkelName)
	if (tgtSkel == None) :
		return
		
	if not os.path.exists(outDir):
		os.makedirs(outDir)			
		
	offsetJoints = VecMap();	
	endJoints = StringVec();
	#endJoints.append('l_ankle')
	endJoints.append('l_forefoot')
	endJoints.append('l_toe')
	endJoints.append('l_acromioclavicular')
	#endJoints.append('r_ankle')		
	endJoints.append('r_forefoot')	
	endJoints.append('r_toe')
	endJoints.append('r_acromioclavicular')
	
	relativeJoints = StringVec();
	relativeJoints.append('spine1')
	relativeJoints.append('spine2')
	relativeJoints.append('spine3')
	relativeJoints.append('spine4')
	relativeJoints.append('spine5')
	
	
	effectorJoints = StringVec();	
	if tgtSkel.getJointByName('r_toe') != None:
		effectorJoints.append('r_toe')
		effectorJoints.append('l_toe')
	else:
		effectorJoints.append('r_ankle')
		effectorJoints.append('l_ankle')		
		
	if tgtSkel.getJointByName('r_forefoot') != None:
		effectorJoints.append('r_forefoot')
		effectorJoints.append('l_forefoot')		
	#effectorJoints.append('l_toe')		
	effectorRoots = StringVec();
	effectorRoots.append('r_hip')
	effectorRoots.append('l_hip')
	effectorRoots.append('r_hip')
	effectorRoots.append('l_hip')
	
	print 'Retarget motion = ' + motionName;
	outMotion = testMotion.retarget(outMotionName,srcSkelName,tgtSkelName, endJoints, relativeJoints, offsetJoints);	
	cleanMotion = testMotion.constrain(outMotionName, srcSkelName, tgtSkelName, outMotionName, effectorJoints, effectorRoots);
	saveCommand = 'animation ' + outMotionName + ' save ' + outDir + outMotionName + '.skm';
	print 'Save command = ' + saveCommand;
	scene.command(saveCommand)
