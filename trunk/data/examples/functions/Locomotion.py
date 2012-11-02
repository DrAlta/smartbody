def move(chrName, target, speed):
	''' Character name(string), target(Character/Pawn name or SrVec), Manner of movement
		Available manners: walk, run, jog, sbm:step, sbm:jump '''
	# Set pathfinding mode off
	scene.getCharacter(chrName).setBoolAttribute('steering.pathFollowingMode', False)
	# If target is character or pawn
	if target in scene.getCharacterNames() or target in scene.getPawnNames():
		bml.execBML(chrName, '<locomotion speed="' + str(speed) + '" target="' + target + '"/>')
	if type(target) is SrVec:
		bml.execBML(chrName, '<locomotion speed="' + str(speed) + '" target="' + vec2str(target) + '"/>')
		
# Sample lists
#vecList = [SrVec(-500, 0, 0), SrVec(1000, -500, 0), SrVec(-700, -700, 0), SrVec(0, 500, 0)]
#mixList = [SrVec(-500, 0, 0), SrVec(1000, 500, 0), scene.getCharacter('utah2'), SrVec(0, 500, 0)]
#chrList = [scene.getCharacter('utah0'), scene.getCharacter('utah4'), scene.getCharacter('utah1'), scene.getCharacter('utah3')]
def followPath(chrName, waypoints, manner='run'):
	''' Character name(string), List of Characters/Pawns/SrVec waypoints 
		sampleList = [SrVec(-500, 0, 0), SrVec(1000, 500, 0), scene.getCharacter('utah2'), SrVec(0, 500, 0)] '''
	targets = ''
	# Set pathfinding mode on
	scene.getCharacter(chrName).setBoolAttribute('steering.pathFollowingMode', True)
	for waypoint in waypoints:
		# If is SrVec
		if type(waypoint) is SrVec:
			x = str(waypoint.getData(0))
			y = str(waypoint.getData(1))
			targets = targets + x + ' ' + y + ' '
		if type(waypoint) is SBCharacter or type(waypoint) is SBPawn:
			targets = targets + waypoint.getName() + ' '
	#print targets
	bml.execBML(chrName, '<locomotion manner="' + manner + '" target="' + targets + '"/>')

def printVector(vec, text=''):
	''' Prints SrVec in x: y: z: format, Text is for debugging/tracking purposes '''
	print '%s x: %s y: %s z: %s' % (text, vec.getData(0), vec.getData(1), vec.getData(2))
		
def vec2str(vec):
	''' Converts SrVec to string '''
	x = vec.getData(0)
	y = vec.getData(1)
	z = vec.getData(2)
	if -0.0001 < x < 0.0001: x = 0
	if -0.0001 < y < 0.0001: y = 0
	if -0.0001 < z < 0.0001: z = 0
	return "" + str(x) + " " + str(y) + " " + str(z) + ""