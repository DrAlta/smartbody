def sad(chrName):
	bml.execBML(chrName, '<face type="facs" au="5" amount="1"/><face type="facs" au="7" amount="0.6"/> + \
						  <face type="facs" au="15" amount="1"/><face type="facs" au="23" amount="0.5"/> + \
						  <face type="facs" au="25" amount="1"/>')
	
def happy(chrName):
	bml.execBML(chrName, '<face type="facs" au="6" amount="1"/><face type="facs" au="12" amount="1"/>')
	
def angry(chrName):
	bml.execBML(chrName, '<face type="facs" au="9" amount="1"/><face type="facs" au="10" amount="0.45"/> + \
						  <face type="facs" au="15" amount="0.45"/>')
	
def shock(chrName):
	bml.execBML(chrName, '<face type="facs" au="10" amount="0.45"/><face type="facs" au="1_left" amount="0.5"/> + \
						  <face type="facs" au="1_right" amount="0.5"/><face type="facs" au="2_left" amount="0.6"/> + \
						  <face type="facs" au="2_right" amount="0.6"/>')
				
def fear(chrName):
	bml.execBML(chrName, '<face type="facs" au="1_left" amount="0.6"/><face type="facs" au="1_right" amount="0.6"/> + \
						  <face type="facs" au="5" amount="0.7"/><face type="facs" au="26" amount="0.25"/> + \
						  <face type="facs" au="38" amount="1"/>')
						  
def disgust(chrName):
	bml.execBML(chrName, '<face type="facs" au="5" amount="1"/><face type="facs" au="6" amount="0.16"/> + \
						  <face type="facs" au="9" amount="0.44"/><face type="facs" au="10" amount="1"/> + \
						  <face type="facs" au="15" amount="0.44"/><face type="facs" au="23" amount="0.16"/> + \
						  <face type="facs" au="27" amount="0.09"/>')
						  
def sleepy(chrName):
	bml.execBML(chrName, '<face type="facs" au="6" amount="0.42"/><face type="facs" au="7" amount="0.09"/> + \
						  <face type="facs" au="45" side="BOTH" amount="0.12"/>')
						  