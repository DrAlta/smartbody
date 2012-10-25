def sad(chrName, duration=1):
	#bml.execBML(chrName, '<face type="facs" au="1" amount="1"/><face type="facs" au="4" amount="1"/><face type="facs" au="15" amount="1" start="0" end="' + str(duration) + '"/>')
	#bml.execBML(chrName, '<face type="facs" au="5" amount="1"/><face type="facs" au="7" amount="0.6"/><face type="facs" au="15" amount="1"/><face type="facs" au="23 amount="0.5"/><face type="facs" au="25" amount="1" start="0" end="' + str(duration) + '"/>')
	bml.execBML(chrName, '<face type="facs" au="5" amount="1"/><face type="facs" au="7" amount="0.6"/><face type="facs" au="15" amount="1"/> + \
						  <face type="facs" au="23" amount="0.5"/><face type="facs" au="25" amount="1" start="0" end="' + str(duration) + '"/>')
	
def happy(chrName, duration=1):
	bml.execBML(chrName, '<face type="facs" au="6" amount="1"/><face type="facs" au="12" amount="1" start="0" end="' + str(duration) + '"/>')
	
def angry(chrName, duration=1):
	bml.execBML(chrName, '<face type="facs" au="9" amount="1"/><face type="facs" au="10" amount="0.45"/><face type="facs" au="15" amount="0.45" start="0" end="' + str(duration) + '"/>')
	
def shock(chrName, duration=1):
	bml.execBML(chrName, '<face type="facs" au="10" amount="0.45"/><face type="facs" au="1_left" amount="0.5"/><face type="facs" au="1_right" amount="0.5"/> + \
						  <face type="facs" au="2_left" amount="0.6"/><face type="facs" au="2_right" amount="0.6"/> +\
						  <face type="facs" au="26" amount="0.35" start="0" end="' + str(duration) + '"/>')