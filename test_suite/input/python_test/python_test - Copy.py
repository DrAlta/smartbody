scene.addAssetPath("seq", "../../../../data/sbm-common/scripts")
scene.loadAssets()

character = scene.createCharacter("character1", "test")
skeleton = scene.createSkeleton("common.sk")
character.setSkeleton(skeleton)

numCharacters = scene.getNumCharacters()
if (numCharacters != 1):
	print "getNumCharacters() failed..."


pawn = scene.createPawn("pawn1")
numPawns = scene.getNumPawns()
if (numPawns != 1):
	print "numPawns() failed..."

quit()


