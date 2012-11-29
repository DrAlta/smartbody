scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.run('zebra2-map.py')
zebra2Map = scene.getJointMapManager().getJointMap('zebra2')

scene.addAssetPath('motion', 'ChrBrad')
scene.loadAssets()

bradSkeleton = scene.getSkeleton('ChrBrad.sk')
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse('ChrBrad')

brad = scene.createCharacter('ChrBrad', 'ChrBrad')
bradSkeleton = scene.createSkeleton('ChrBrad.sk')
brad.setSkeleton(bradSkeleton)
brad.createStandardControllers()
brad.setDoubleAttribute('deformableMeshScale', .01)
brad.setStringAttribute('deformableMesh', 'ChrBrad')

if scene.getNumCharacters() != 1:
	print 'getNumCharacters() failed...'

pawn = scene.createPawn('pawn1')
if scene.getNumPawns() != 1:
	print 'getNumPawns() failed...'
	
scene.commandAt(5, 'quit')