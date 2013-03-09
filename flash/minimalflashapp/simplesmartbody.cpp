#include <gl/gl.h>     // The GL Header File
#include <gl/glut.h>   // The GL Utility Toolkit (Glut) Header
#include "vhcl.h"
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPython.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <AS3/AS3.h>
#include <Flash++.h>
#include <GL/gl.h>
#include <sr/sr.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_camera.h>
#include <sr/sr_gl.h>
#include <sr/sr_color.h>
#include <sr/sr_light.h>
#include <sb/sbm_pawn.hpp>
#include <sbm/GPU/SbmTexture.h>
using namespace AS3::ui;

//---- main loop for flascc
extern "C"
{
	extern void glutMainLoopBody();
};

//---- static rendering variables for smartbody
SrSaGlRender render_action;
SrLight light1;
SrLight light2;
int windowWidth = 800;
int windowHeight = 600;

//---- static flash variables
flash::text::TextField console;
bool showLog;
flash::text::TextField logLabel;


void init ( void )     // Create Some Everyday Functions
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.63f, 0.63, 0.63, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable ( GL_COLOR_MATERIAL );
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

// draw characters
void drawCharacters()
{
	const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		pawnIter != pawns.end();
		pawnIter++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
		if(pawn->dMesh_p && pawn->dMeshInstance_p)
		{
			pawn->dMeshInstance_p->update();
		}
	}
	render_action.apply(SmartBody::SBScene::getScene()->getRootGroup());
}

void display ( void )   // Create The Display Function
{
	// texture
	SbmTextureManager::singleton().updateTexture();
	
	// clear background
	glClearColor(0.63f, 0.63, 0.63, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	// camera
	SrCamera* mainCamera = SmartBody::SBScene::getScene()->getCamera("mainCamera");
	SrMat mat(SrMat::NotInitialized);
	glMatrixMode   ( GL_PROJECTION );  // Select The Projection Matrix
	glLoadMatrix(mainCamera->get_perspective_mat(mat));                // Reset The Projection Matrix

	glMatrixMode   ( GL_MODELVIEW );  // Select The Model View Matrix
	glLoadMatrix(mainCamera->get_view_mat(mat));

	glScalef(mainCamera->getScale(), mainCamera->getScale(), mainCamera->getScale());	
	
	// lighting
	glEnable(GL_LIGHTING);
	glLight(0, light1);
	glLight(1, light2);

	// draw characters
	drawCharacters();

	// Swap The Buffers To Not Be Left With A Clear Screen
	glutSwapBuffers ( );
}

void reshape ( int w, int h )   // Create The Reshape Function (the viewport)
{
	glViewport     ( 0, 0, w, h );
	glMatrixMode   ( GL_PROJECTION );  // Select The Projection Matrix
	glLoadIdentity ( );                // Reset The Projection Matrix
	glMatrixMode   ( GL_MODELVIEW );  // Select The Model View Matrix
	glLoadIdentity ( );    // Reset The Model View Matrix
}

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
	switch ( key ) {
	case 27:        // When Escape Is Pressed...
		exit ( 0 );   // Exit The Program
		break;        // Ready For Next Case
	default:        // Now Wrap It Up
		break;
	}
}

void arrow_keys ( int a_keys, int x, int y )  // Create Special Function (required for arrow keys)
{
	switch ( a_keys ) {
	case GLUT_KEY_UP:     // When Up Arrow Is Pressed...
		glutFullScreen ( ); // Go Into Full Screen Mode
		break;
	case GLUT_KEY_DOWN:               // When Down Arrow Is Pressed...
		glutReshapeWindow ( windowWidth, windowHeight); // Go Into A 500 By 500 Window
		break;
	default:
		break;
	}
}

static void idle()
{
	glutPostRedisplay();
}

static double lastPrint;
static double lastPrint1;
extern "C"
void smartbodyLoop()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
	sim->update();
	if (sim->getTime() > lastPrint)
	{
		LOG("Simulation is at time: %5.2f", sim->getTime());
		lastPrint = sim->getTime() + 10;
	}
	scene->update();

	const std::vector<std::string>& pawns = SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator pawnIter = pawns.begin();
		pawnIter != pawns.end();
		pawnIter++)
	{
		SmartBody::SBPawn* pawn = SmartBody::SBScene::getScene()->getPawn((*pawnIter));
		if (pawn->scene_p)
			pawn->scene_p->update();	
	}
}

extern "C"
void mainLoop()
{
	smartbodyLoop();
	glutMainLoopBody();
}

void initStage3D(int argc, char** argv)
{
	glutInit            ( &argc, argv ); // Erm Just Write It =)
	init ();
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE ); // Display Mode
	glutInitWindowSize  ( windowWidth, windowHeight ); // If glutFullScreen wasn't called this is the window size
	glutCreateWindow    ( "NeHe's OpenGL Framework" ); // Window Title (argv[0] for current directory as title)
	glutFullScreen      ( );          // Put Into Full Screen
	glutDisplayFunc     ( display );  // Matching Earlier Functions To Their Counterparts
	glutReshapeFunc     ( reshape );
	glutIdleFunc        ( idle    );
	glutKeyboardFunc    ( keyboard );
	glutSpecialFunc     ( arrow_keys );
	glutMainLoop        ( );          // Initialize The Main Loop
}

void toggleMeshDrawing(bool mesh)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> charNames = scene->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); i++)
	{
		SmartBody::SBCharacter* character = scene->getCharacter(charNames[i]);
		if (!character)
			continue;
		if (!character->scene_p)
			continue;
	
		if (!mesh)
		{
			character->scene_p->set_visibility(1, 0, 0, 0);
			if (character->dMesh_p)
				character->dMesh_p->set_visibility(0);
			LOG("Character %s's mesh mode enabled, bone mode disabled.", charNames[i].c_str());
		}
		else
		{
			character->scene_p->set_visibility(0, 0, 0, 0);
			if (character->dMesh_p)
				character->dMesh_p->set_visibility(1);
			if (character->dMeshInstance_p)
				character->dMeshInstance_p->setVisibility(1);
			LOG("Character %s's mesh mode disabled, bone mode enabled.", charNames[i].c_str());
		}
	}
}


void toggleLog(bool flag)
{
	// print out something
	for (int i = 0; i < 100; ++i)
	{
		LOG("");
	}

	vhcl::Log::g_log.RemoveAllListeners();
	if (flag)
	{
		vhcl::Log::StdoutListener* stdoutLog = new vhcl::Log::StdoutListener();
		vhcl::Log::g_log.AddListener(stdoutLog);
	}
}

void initSBSceneSetting()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SrCamera* mainCamera = scene->createCamera("mainCamera");
	// camera
	/*
	mainCamera->setEye(0.719815, 2.0478, 4.69259);
	mainCamera->setCenter(0.759279, 1.60887, 2.75628);
	mainCamera->setUpVector(SrVec(0, 1, 0));
	mainCamera->setScale(1);
	mainCamera->setFov(0.4);
	mainCamera->setFarPlane(100);
	mainCamera->setNearPlane(0.1);
	mainCamera->setAspectRatio(1.39286);
	*/
	mainCamera->setEye(0.0f, 20.0f, 200.0f);
	mainCamera->setCenter(0.0f, 0.0f, 0.0f);
	mainCamera->setUpVector(SrVec(0.0f, 1.0f, 0.0f));
	mainCamera->setScale(1);
	mainCamera->setNearPlane(0.1f);
	mainCamera->setFarPlane(1000.0f);
	mainCamera->setAspectRatio(1.0f);


	// light
	light1.directional = true;
	light1.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	light1.position = SrVec( 100.0, 250.0, 400.0 );
	light1.constant_attenuation = 1.0f;

	light2 = light1;
	light2.directional = true;
	light2.diffuse = SrColor( 0.8f, 0.8f, 0.8f );
	light2.position = SrVec( 100.0, 500.0, -1000.0 );

}


void initCharacterScene()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	// for some reason couldn't get flash listener working properly, following code is supposed to replace character listener
	std::vector<std::string> charNames = scene->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); i++)
	{
		SmartBody::SBCharacter* character = scene->getCharacter(charNames[i]);
		if (!character)
			continue;

		// add skeletons and mesh
		if (character->scene_p)
		{
			SmartBody::SBScene::getScene()->getRootGroup()->remove(character->scene_p);
			character->scene_p->unref();
			character->scene_p = NULL;
		}
		character->scene_p = new SkScene();
		character->scene_p->ref();
		character->scene_p->init(character->getSkeleton());
		bool visible = character->getBoolAttribute("visible");
		if (visible)
			character->scene_p->visible(true);
		else
			character->scene_p->visible(false);
		scene->getRootGroup()->add(character->scene_p);
		LOG("Character %s's skeleton added to the scene.", charNames[i].c_str());

		if (character->dMesh_p)
		{
			for (size_t i = 0; i < character->dMesh_p->dMeshDynamic_p.size(); i++)
			{
				SmartBody::SBScene::getScene()->getRootGroup()->remove( character->dMesh_p->dMeshDynamic_p[i] );
			}
			delete character->dMesh_p;
			character->dMesh_p = NULL;
		}
		character->dMesh_p = new DeformableMesh();
		character->dMeshInstance_p =  new DeformableMeshInstance();
		character->dMesh_p->setSkeleton(character->getSkeleton());
		character->dMeshInstance_p->setSkeleton(character->getSkeleton());
		LOG("Character %s's deformable mesh reset.", charNames[i].c_str());

		std::string dMeshAttrib = character->getStringAttribute("deformableMesh");
		character->setStringAttribute("deformableMesh", dMeshAttrib);
		LOG("Character %s's deformable mesh %s added to the scene.", charNames[i].c_str(), dMeshAttrib.c_str());
	}
}


void initSB()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	toggleLog(true);
	LOG("Loading python...");
	initPython("../../Python26/Libs");
	initSBSceneSetting();

	// get the simulation object 
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
	sim->setupTimer();
	LOG("Set up timer ...");

	// start up script (the data folder has follow the hierarchy)
	scene->setMediaPath("/root/data");
	scene->addAssetPath("seq", "sbm-common/scripts");
	scene->runScript("default-init.py");

	initCharacterScene();
	toggleMeshDrawing(true);
}



var buttonSendCommand(void *arg, var as3Args)
{
	std::string textInput = AS3::sz2stringAndFree(internal::utf8_toString(console->text));
	LOG("%s\n", textInput.c_str());
	SmartBody::SBScene::getScene()->run(textInput);
	return internal::_undefined;
}


var buttonToggleLog(void *arg, var as3Args)
{
	showLog = !showLog;
	if (showLog)
	{
		logLabel->text = "log on";
	}
	else
	{
		logLabel->text = "log off";
	}
	toggleLog(showLog);
	return internal::_undefined;
}


double prevX;
double prevY;
double deltaX;
double deltaY;
bool firstTime = true;
var mouseMoveHandler(void *arg, var as3Args)
{
	SrCamera* mainCamera = SmartBody::SBScene::getScene()->getCamera("mainCamera");

	// get the event object
	flash::events::MouseEvent event = var(as3Args[0]);
	double x = event->stageX;
	double y = event->stageY;
	double deltaX = x - prevX;
	double deltaY = y - prevY;
	if (firstTime)
	{
		deltaX = 0.0;
		deltaY = 0.0;
		firstTime = false;
	}
	prevX = x;
	prevY = y;
	bool altKey = event->altKey;
	bool ctrlKey = event->ctrlKey;
	bool shiftKey = event->shiftKey;
	bool leftMouse = event->buttonDown;

	float dx = deltaX * mainCamera->getAspectRatio();
	float dy = deltaY * mainCamera->getAspectRatio();

	if (altKey && leftMouse)	// zoom
	{
		float tmpFov = mainCamera->getFov() + (-dx + dy);
		mainCamera->setFov(SR_BOUND(tmpFov, 0.001f, srpi));
	}
	/*
	if (shiftKey && leftMouse)	// rotate
	{
		// put coordinates inside [-1,1] with (0,0) in the middle :
		SrVec origUp = mainCamera->getUpVector();
		SrVec origCenter = mainCamera->getCenter();
		SrVec origCamera = mainCamera->getEye();

		SrVec forward = origCenter - origCamera; 		   
		SrQuat q = SrQuat(origUp, vhcl::DEG_TO_RAD()*deltaX*150.f);			   
		forward = forward*q;
		SrVec tmp = mainCamera->getEye() + forward;
		mainCamera->setCenter(tmp.x, tmp.y, tmp.z);

		SrVec cameraRight = cross(forward,origUp);
		cameraRight.normalize();		   
		q = SrQuat(cameraRight, vhcl::DEG_TO_RAD()*deltaY*150.f);	
		mainCamera->setUpVector(origUp*q);
		forward = forward*q;
		SrVec tmpCenter = mainCamera->getEye() + forward;
		mainCamera->setCenter(tmpCenter.x, tmpCenter.y, tmpCenter.z);
	}
	if (ctrlKey && leftMouse)	// dolly
	{
		float amount = dx-dy;
		SrVec cameraPos(mainCamera->getEye());
		SrVec targetPos(mainCamera->getCenter());
		SrVec diff = targetPos - cameraPos;
		float distance = diff.len();
		diff.normalize();

		if (amount >= distance)
			amount = distance - .000001f;

		SrVec diffVector = diff;
		SrVec adjustment = diffVector * distance * amount;
		cameraPos += adjustment;
		SrVec oldEyePos = mainCamera->getEye();
		mainCamera->setEye(cameraPos.x, cameraPos.y, cameraPos.z);
		SrVec cameraDiff = mainCamera->getEye() - oldEyePos;
		SrVec tmpCenter = mainCamera->getCenter();
		tmpCenter += cameraDiff;
		mainCamera->setCenter(tmpCenter.x, tmpCenter.y, tmpCenter.z);
	}
	*/
	glFlush();
	return internal::_undefined;
}

var keyHandler(void *arg, var as3Args)
{
	SrCamera* mainCamera = SmartBody::SBScene::getScene()->getCamera("mainCamera");

	// get the event object
	flash::events::KeyboardEvent event = (flash::events::KeyboardEvent) as3Args[0];

	// pull some information out of it
	String type = event->type;
	Object target = event->target;
	int key = event->keyCode;

	/*
	if (key == 82)
	{
		LOG("reset camera");
		mainCamera->setEye(0.719815, 2.0478, 4.69259);
		mainCamera->setCenter(0.759279, 1.60887, 2.75628);
		mainCamera->setUpVector(SrVec(0, 1, 0));
		mainCamera->setScale(1);
		mainCamera->setFov(0.4);
		mainCamera->setFarPlane(100);
		mainCamera->setNearPlane(0.1);
		mainCamera->setAspectRatio(1.39286);
	}
	*/
	glFlush();
	return internal::_undefined;
}

void initFlash()
{
	// initialize
	showLog = true;

	// stage
	flash::display::Stage stage = internal::get_Stage();
	stage->addEventListener(flash::events::MouseEvent::MOUSE_MOVE, Function::_new(mouseMoveHandler, NULL));
	stage->addEventListener(flash::events::KeyboardEvent::KEY_DOWN, Function::_new(keyHandler, NULL));

	// command console
	console = flash::text::TextField::_new();
	console->background = true;
	console->type = flash::text::TextFieldType::INPUT;
	console->x = 10;
	console->y = windowHeight - 30;
	console->width = 400;
	console->height = 20;
	console->appendText("Enter command here");
	stage->addChild(console);

	// sending command button
	flash::display::Sprite button = flash::display::Sprite::_new();
	button->x = 410;
	button->y = windowHeight - 30;
	flash::display::Graphics buttonGraphics = button->graphics;
	buttonGraphics->beginFill(0xC8C8C8);
	buttonGraphics->drawRoundRect(0, 0, 100, 20, 10, 10);
	buttonGraphics->endFill();
	button->addEventListener(flash::events::MouseEvent::CLICK, Function::_new(buttonSendCommand, NULL));
	stage->addChild(button);
	flash::text::TextField sendLabel = flash::text::TextField::_new();
	sendLabel->text = "send";
	sendLabel->autoSize = flash::text::TextFieldAutoSize::CENTER;
	sendLabel->x = (button->width -sendLabel->textWidth) * 0.5f;
	sendLabel->y = (button->height- sendLabel->textHeight) * 0.5f;
	sendLabel->selectable = false;
	button->addChild(sendLabel);

	// toggle mesh button
	flash::display::Sprite button1 = flash::display::Sprite::_new();
	button1->x = windowWidth - 120;
	button1->y = 10;
	flash::display::Graphics buttonGraphics1 = button1->graphics;
	buttonGraphics1->beginFill(0xC8C8C8);
	buttonGraphics1->drawRoundRect(0, 0, 100, 20, 10, 10);
	buttonGraphics1->endFill();
	button1->addEventListener(flash::events::MouseEvent::CLICK, Function::_new(buttonToggleLog, NULL));
	stage->addChild(button1);
	logLabel = flash::text::TextField::_new();
	logLabel->text = "log on";
	logLabel->autoSize = flash::text::TextFieldAutoSize::CENTER;
	logLabel->x = (button1->width - logLabel->textWidth) * 0.5f;
	logLabel->y = (button1->height- logLabel->textHeight) * 0.5f;
	logLabel->selectable = false;
	button1->addChild(logLabel);
}

int main ( int argc, char** argv )
{
	initFlash();
	initSB();
	initStage3D(argc, argv);
	return 0;
}

