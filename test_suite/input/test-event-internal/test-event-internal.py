scene.setScale(1.0)
scene.setMediaPath('../../../../data')
scene.addAssetPath('script','sbm-common/scripts')
scene.addAssetPath('mesh', 'mesh')

scene.getSimulationManager().setSimFps(60)

scene.run("zebra2-map.py")
zebra2Map = scene.getJointMapManager().getJointMap("zebra2")

scene.addAssetPath('motion','ChrBrad')
scene.loadAssets()

bradSkeleton = scene.getSkeleton("ChrBrad.sk")
zebra2Map.applySkeleton(bradSkeleton)
zebra2Map.applyMotionRecurse("ChrBrad")

scene.run('init-diphoneDefault.py')

bradFace = scene.createFaceDefinition("ChrBrad")
bradFace.setFaceNeutral("ChrBrad@face_neutral")

bradFace.setAU(1,  "left",  "ChrBrad@001_inner_brow_raiser_lf")
bradFace.setAU(1,  "right", "ChrBrad@001_inner_brow_raiser_rt")
bradFace.setAU(2,  "left",  "ChrBrad@002_outer_brow_raiser_lf")
bradFace.setAU(2,  "right", "ChrBrad@002_outer_brow_raiser_rt")
bradFace.setAU(4,  "left",  "ChrBrad@004_brow_lowerer_lf")
bradFace.setAU(4,  "right", "ChrBrad@004_brow_lowerer_rt")
bradFace.setAU(5,  "both",  "ChrBrad@005_upper_lid_raiser")
bradFace.setAU(6,  "both",  "ChrBrad@006_cheek_raiser")
bradFace.setAU(7,  "both",  "ChrBrad@007_lid_tightener")
bradFace.setAU(10, "both",  "ChrBrad@010_upper_lip_raiser")
bradFace.setAU(12, "left",  "ChrBrad@012_lip_corner_puller_lf")
bradFace.setAU(12, "right", "ChrBrad@012_lip_corner_puller_rt")
bradFace.setAU(25, "both",  "ChrBrad@025_lips_part")
bradFace.setAU(26, "both",  "ChrBrad@026_jaw_drop")
bradFace.setAU(45, "left",  "ChrBrad@045_blink_lf")
bradFace.setAU(45, "right", "ChrBrad@045_blink_rt")

bradFace.setViseme("open",    "ChrBrad@open")
bradFace.setViseme("W",       "ChrBrad@W")
bradFace.setViseme("ShCh",    "ChrBrad@ShCh")
bradFace.setViseme("PBM",     "ChrBrad@PBM")
bradFace.setViseme("FV",      "ChrBrad@FV")
bradFace.setViseme("wide",    "ChrBrad@wide")
bradFace.setViseme("tBack",   "ChrBrad@tBack")
bradFace.setViseme("tRoof",   "ChrBrad@tRoof")
bradFace.setViseme("tTeeth",  "ChrBrad@tTeeth")

brad = scene.createCharacter('ChrBrad', 'ChrBrad')
bradSkeleton = scene.createSkeleton('ChrBrad.sk')
brad.setSkeleton(bradSkeleton)
brad.setFaceDefinition(bradFace)
brad.createStandardControllers()
brad.setDoubleAttribute('deformableMeshScale', .01)
brad.setStringAttribute('deformableMesh', 'ChrBrad')

brad.setStringAttribute("diphoneSetName", "default")
brad.setBoolAttribute("useDiphone", True)
brad.setVoice("remote")
brad.setVoiceCode("MicrosoftAnna")

# Setting camera parameters
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(.0, 1.65, 1.36)
camera.setCenter(.0, 1.54, 1.1)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(0.966897)
scene.getPawn('camera').setPosition(SrVec(0, 1.66, 1.85))

scene.command("char ChrBrad viewer deformableGPU")

pawn1 = scene.createPawn('pawn1')
pawn1.setPosition(SrVec(3.0, 1.2, -2.0))

motion1 = scene.getMotion('ChrBrad@Idle01_ExampleLf01')
motion1.addEvent(0.5, 'motionSpeech', 'example', False)
motion2 = scene.getMotion('ChrBrad@Idle01_MeLf01')
motion2.addEvent(0.3, 'motionSpeech', 'me', False)
motion3 = scene.getMotion('ChrBrad@Idle01_YouLf01')
motion3.addEvent(0.3, 'motionSpeech', 'you', False)

class MyEventHandler(EventHandler):
	def executeAction(self, event):
		params = event.getParameters()
		print params
		if 'you' in params:
			bml.execBML('ChrBrad', '<speech type="text/plain">Who are you? Where are you from?</speech>')
		if 'me' in params:
			bml.execBML('ChrBrad', '<speech type="text/plain">Hello, my name is Brad.</speech>')
		if 'example' in params:
			bml.execBML('ChrBrad', '<speech type="text/plain">A very interesting sentence indeed</speech>')
myHandler = MyEventHandler()
eventManager = scene.getEventManager()
eventManager.addEventHandler('motionSpeech', myHandler)

sim.start()
bml.execBML('ChrBrad', '<body posture="ChrBrad@Idle01"/>')

bml.execBMLAt(2, 'ChrBrad', '<animation name="ChrBrad@Idle01_ExampleLf01"/>')
scene.commandAt(3.5, 'snapshot')
bml.execBMLAt(5, 'ChrBrad', '<animation name="ChrBrad@Idle01_MeLf01"/>')
scene.commandAt(6.3, 'snapshot')
bml.execBMLAt(8, 'ChrBrad', '<animation name="ChrBrad@Idle01_YouLf01"/>')
scene.commandAt(8.8, 'snapshot')

scene.commandAt(9, 'quit')