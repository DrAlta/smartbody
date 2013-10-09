#include <gl/gl.h>     // The GL Header File
#include <gl/glut.h>   // The GL Utility Toolkit (Glut) Header
#include "vhcl.h"
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBCharacterListener.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPython.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBTypes.h>
#include <AS3/AS3.h>
#include <Flash++.h>
#include <GL/gl.h>
#include <sr/sr.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_camera.h>
#include <sr/sr_gl.h>
#include <sr/sr_color.h>
#include <sr/sr_light.h>
#include <sr/sr_gl_render_funcs.h>
#include <sb/sbm_pawn.hpp>
#include <sbm/GPU/SbmTexture.h>
#include <stdio.h>
#include <AS3/AS3++.h> // using AS3 var wrapper class
#include <pthread.h>
#include <sb/SBAttribute.h>



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
std::vector<SrLight> _lights;

//---- static flash variables
flash::text::TextField console;
bool showLog;
flash::text::TextField logLabel;
flash::text::TextField initbtn;
flash::text::TextField initbtn2;
bool sbInited;

//--loading varible
bool loading=false;
int frameNeeded=60;
int framePassed=0;


class FlashListener : public SmartBody::SBCharacterListener
{
public:
	
	FlashListener() :SBCharacterListener(){}
	virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass )
	{
		LOG("flash listener: OnCharacterCreate!");
		SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
		if (!character)
			return;

		// remove any existing scene
		if (character->scene_p)
		{
			if( SmartBody::SBScene::getScene()->getRootGroup() )
			{
				SmartBody::SBScene::getScene()->getRootGroup()->remove( character->scene_p ); 
			}
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


		if( SmartBody::SBScene::getScene()->getRootGroup() )
		{
			SmartBody::SBScene::getScene()->getRootGroup()->add( character->scene_p ); 
		}

		// remove any existing deformable mesh
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

		SmartBody::SBSkeleton* sbSkel = character->getSkeleton();
		character->dMesh_p->setSkeleton(sbSkel);
		character->dMeshInstance_p->setSkeleton(sbSkel);
	}

	  virtual void OnCharacterDelete( const std::string & name )
	  {
		  LOG("flash listener: OnCharacterDelete!");
		SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
		if (!character)
			return;

		// remove any existing scene
		if (character->scene_p)
		{
			if( SmartBody::SBScene::getScene()->getRootGroup() )
			{
				SmartBody::SBScene::getScene()->getRootGroup()->remove( character->scene_p ); 
			}
			character->scene_p->unref();
			character->scene_p = NULL;
		}
		// remove any existing deformable mesh
		if (character->dMesh_p)
		{
			for (size_t i = 0; i < character->dMesh_p->dMeshDynamic_p.size(); i++)
			{
				SmartBody::SBScene::getScene()->getRootGroup()->remove( character->dMesh_p->dMeshDynamic_p[i] );
			}
			//delete character->dMesh_p; // AS 1/28/13 causing crash related to mesh instances
			character->dMesh_p = NULL;
		}
	  }

	  virtual void OnCharacterUpdate( const std::string & name, const std::string & objectClass )
	  {
		  LOG("flash listener: OnCharacterDelete!");
		  OnCharacterDelete(name);
			OnCharacterCreate(name, objectClass);
	  }

	  virtual void OnCharacterChanged( const std::string& name )
	  {
		   LOG("flash listener: OnCharacterChanged!");
		  SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
			if (!character)
				return;

			OnCharacterDelete(name);
			OnCharacterCreate(name, character->getClassType());
	  }

	virtual void OnLogMessage(const std::string& message) { std::cout << message << std::endl; }

};


void init ( void )     // Create Some Everyday Functions
{
	printf("init\n");
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
		SmartBody::SBCharacter* character = dynamic_cast<SmartBody::SBCharacter*> (pawn);
		if(pawn->dMesh_p && pawn->dMeshInstance_p)
		{
			pawn->dMeshInstance_p->update();
			SrGlRenderFuncs::renderDeformableMesh(pawn->dMeshInstance_p, false);
			character->scene_p->set_visibility(0,1,0,0);
			render_action.apply(character->scene_p);
		}
	}
	//render_action.apply(smartbody::sbscene::getscene()->getrootgroup());
}


void updateLights()
{
	
	// get any pawns called 'light#' 
	// if none exist, use the standard lights
	_lights.clear();
	const std::vector<std::string>& pawnNames =  SmartBody::SBScene::getScene()->getPawnNames();
	for (std::vector<std::string>::const_iterator iter = pawnNames.begin();
		 iter != pawnNames.end();
	      iter++)
	{
		SmartBody::SBPawn* sbpawn = SmartBody::SBScene::getScene()->getPawn(*iter);
		const std::string& name = sbpawn->getName();
		if (name.find("light") == 0)
		{
			SrLight light;
			light.position = sbpawn->getPosition();
			SmartBody::BoolAttribute* directionalAttr = dynamic_cast<SmartBody::BoolAttribute*>(sbpawn->getAttribute("lightIsDirectional"));
			if (directionalAttr)
			{
				light.directional = directionalAttr->getValue();
			}
			else
			{
				light.directional = true;
			}
			
			SmartBody::Vec3Attribute* diffuseColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightDiffuseColor"));
			if (diffuseColorAttr)
			{
				const SrVec& color = diffuseColorAttr->getValue();
				light.diffuse = SrColor( color.x, color.y, color.z );
			}
			else
			{
				light.diffuse = SrColor( 1.0f, 0.95f, 0.8f );
			}
			SmartBody::Vec3Attribute* ambientColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightAmbientColor"));
			if (ambientColorAttr)
			{
				const SrVec& color = ambientColorAttr->getValue();
				light.ambient = SrColor( color.x, color.y, color.z );
			}
			else
			{
				light.ambient = SrColor( 0.0f, 0.0f, 0.0f );
			}
			SmartBody::Vec3Attribute* specularColorAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightSpecularColor"));
			if (specularColorAttr)
			{
				const SrVec& color = specularColorAttr->getValue();
				light.specular = SrColor( color.x, color.y, color.z );
			}
			else
			{
				light.specular = SrColor( 0.0f, 0.0f, 0.0f );
			}
			SmartBody::DoubleAttribute* spotExponentAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightSpotExponent"));
			if (spotExponentAttr)
			{
				light.spot_exponent = (float) spotExponentAttr->getValue();
			}
			else
			{
				light.spot_exponent = 0.0f;
			}
			SmartBody::Vec3Attribute* spotDirectionAttr = dynamic_cast<SmartBody::Vec3Attribute*>(sbpawn->getAttribute("lightSpotDirection"));
			if (spotDirectionAttr)
			{
				const SrVec& direction = spotDirectionAttr->getValue();
				light.spot_direction = direction;
			}
			else
			{
				light.spot_direction = SrVec( 0.0f, 0.0f, -1.0f );
			}
			SmartBody::DoubleAttribute* spotCutOffAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightSpotCutoff"));
			if (spotExponentAttr)
			{
				light.spot_cutoff = (float) spotCutOffAttr->getValue();
			}
			else
			{
				light.spot_cutoff = 180.0f;
			}
			SmartBody::DoubleAttribute* constantAttentuationAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightConstantAttenuation"));
			if (constantAttentuationAttr)
			{
				light.constant_attenuation = (float) constantAttentuationAttr->getValue();
			}
			else
			{
				light.constant_attenuation = 1.0f;
			}
			SmartBody::DoubleAttribute* linearAttentuationAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightLinearAttenuation"));
			if (linearAttentuationAttr)
			{
				light.linear_attenuation = (float) linearAttentuationAttr->getValue();
			}
			else
			{
				light.linear_attenuation = 0.0f;
			}
			SmartBody::DoubleAttribute* quadraticAttentuationAttr = dynamic_cast<SmartBody::DoubleAttribute*>(sbpawn->getAttribute("lightQuadraticAttenuation"));
			if (quadraticAttentuationAttr)
			{
				light.quadratic_attenuation = (float) quadraticAttentuationAttr->getValue();
			}
			else
			{
				light.quadratic_attenuation = 0.0f;
			}
			
			_lights.push_back(light);
		}
	}
	//LOG("light size = %d\n",_lights.size());
	
	if (_lights.size() == 0)
		//if (true)
	{
		SrLight light;		
		light.directional = true;
		light.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
		light1.position = SrVec( 100.0, 250.0, 400.0 );
	//	light.constant_attenuation = 1.0f/cam.scale;
		light.constant_attenuation = 1.0f;
		_lights.push_back(light);

		SrLight light2 = light;
		light2.directional = true;
		light2.diffuse = SrColor( 0.8f, 0.8f, 0.8f );
		light2.position = SrVec( 100.0, 500.0, -1000.0 );

	//	light2.constant_attenuation = 1.0f;
	//	light2.linear_attenuation = 2.0f;
		_lights.push_back(light2);
	}

	//light1.directional = true;
	//light1.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	//light1.position = SrVec( 100.0, 250.0, 400.0 );
	//light1.constant_attenuation = 1.0f;

	//light2 = light1;
	//light2.directional = true;
	//light2.diffuse = SrColor( 0.8f, 0.8f, 0.8f );
	//light2.position = SrVec( 100.0, 500.0, -1000.0 );
	//
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
	
	updateLights();
	glEnable ( GL_LIGHTING );
	for (size_t x = 0; x < _lights.size(); x++)
	{
		
		glLight ( x, _lights[x] );		
	}


  glPushMatrix();
	//some reason adding sphere the light will be enabled
  glutSolidSphere(0.001, 1, 1);
  glPopMatrix();


//	// draw characters
	drawCharacters();
	glFlush();



//	glutSwapBuffers ( );




  
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
			LOG("Character %s's mesh mode disabled, bone mode enabled.", charNames[i].c_str());
		}
		else
		{
			character->scene_p->set_visibility(0, 0, 0, 0);
			if (character->dMesh_p)
				character->dMesh_p->set_visibility(1);
			if (character->dMeshInstance_p)
				character->dMeshInstance_p->setVisibility(1);
			LOG("Character %s's mesh mode enabled, bone mode disabled.", charNames[i].c_str());
		}
	}
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
	FlashListener* flashListener = new FlashListener();
	scene->setCharacterListener(flashListener);
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
	scene->addAssetPath("script", "sbm-common/scripts");
	scene->runScript("default-init.py");
	sbInited=true;
	//initCharacterScene();
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
	//toggleLog(showLog);
	toggleMeshDrawing(showLog);
	
	return internal::_undefined;
}


void *threadProc(void *arg)
{   
	printf("threading");
	printf("threading");
	printf("threading");
	printf("threading");
	printf("threading");
	printf("threading");
	printf("threading");
	printf("threading");
	printf("threading");
	return NULL;
}
flash::display::Loader picture = flash::display::Loader::_new();
flash::display::Loader loader = flash::display::Loader::_new();

var buttonInitSb(void *arg, var as3Args)
{
	//start timer so that loading image will display before it actually start to load
	if(framePassed ==0){

			// stage
	flash::display::Stage stage = internal::get_Stage();

	//flash::display::Loader picture = flash::display::Loader::_new();
	flash::net::URLRequest request = flash::net::URLRequest::_new("loading.jpg");
	loader->load(request);
	stage->addChild(picture);
	//magic number
	loader->x = windowWidth/2.0f -150;
	loader->y = windowHeight/2.0f -150;
	
	
	flash::net::URLRequest request2 = flash::net::URLRequest::_new("download.jpg");
	picture->load(request2);
	stage->addChild(loader);
	//magic number
	picture->x =windowWidth/2.0f -226 ;
	picture->y =windowWidth/2.0f -130-70 ;
	 



	loading = true;
	}
	
  // create a new thread to watch mouse moves!
   // pthread_create(&thread, NULL, threadProc, NULL);
 	
	//pthread_join(thread, NULL);
	return internal::_undefined;
}


var buttonInit3d(void *arg, var as3Args)
{
	// get the simulation object 
	
	return internal::_undefined;
}



var buttonInitTest(void *arg, var as3Args)
{
	// get the simulation object 
	/*pthread_t thread;
	pthread_create(&thread, NULL, threadProc, NULL);
	pthread_join(thread, NULL);*/
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
	printf("Hello World initflash\n");
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
	logLabel->text = "mesh on";
	logLabel->autoSize = flash::text::TextFieldAutoSize::CENTER;
	logLabel->x = (button1->width - logLabel->textWidth) * 0.5f;
	logLabel->y = (button1->height- logLabel->textHeight) * 0.5f;
	logLabel->selectable = false;
	button1->addChild(logLabel);

		// init 1 button
	flash::display::Sprite button2 = flash::display::Sprite::_new();
	button2->x = windowWidth - 120;
	button2->y = 50;
	buttonGraphics1 = button2->graphics;
	buttonGraphics1->beginFill(0xC8C8C8);
	buttonGraphics1->drawRoundRect(0, 0, 100, 20, 10, 10);
	buttonGraphics1->endFill();
	button2->addEventListener(flash::events::MouseEvent::CLICK, Function::_new(buttonInitSb, NULL));
	stage->addChild(button2);
	initbtn = flash::text::TextField::_new();
	initbtn->text = "init sb N script";
	initbtn->autoSize = flash::text::TextFieldAutoSize::CENTER;
	initbtn->x = (button2->width - initbtn->textWidth) * 0.5f;
	initbtn->y = (button2->height- initbtn->textHeight) * 0.5f;
	initbtn->selectable = false;
	button2->addChild(initbtn);


	//	// init 2 button
	//flash::display::Sprite button3 = flash::display::Sprite::_new();
	//button3->x = windowWidth - 120;
	//button3->y = 100;
	//buttonGraphics1 = button3->graphics;
	//buttonGraphics1->beginFill(0xC8C8C8);
	//buttonGraphics1->drawRoundRect(0, 0, 100, 20, 10, 10);
	//buttonGraphics1->endFill();
	//button3->addEventListener(flash::events::MouseEvent::CLICK, Function::_new(buttonInit3d, NULL));
	//stage->addChild(button3);
	//initbtn2 = flash::text::TextField::_new();
	//initbtn2->text = "init script";
	//initbtn2->autoSize = flash::text::TextFieldAutoSize::CENTER;
	//initbtn2->x = (button3->width - initbtn2->textWidth) * 0.5f;
	//initbtn2->y = (button3->height- initbtn2->textHeight) * 0.5f;
	//initbtn2->selectable = false;
	//button3->addChild(initbtn2);


	//	// init 3 button
	//flash::display::Sprite button4 = flash::display::Sprite::_new();
	//button4->x = windowWidth - 120;
	//button4->y = 150;
	//buttonGraphics1 = button4->graphics;
	//buttonGraphics1->beginFill(0xC8C8C8);
	//buttonGraphics1->drawRoundRect(0, 0, 100, 20, 10, 10);
	//buttonGraphics1->endFill();
	//button4->addEventListener(flash::events::MouseEvent::CLICK, Function::_new(buttonInitTest, NULL));
	//stage->addChild(button4);
	//initbtn2 = flash::text::TextField::_new();
	//initbtn2->text = "init test";
	//initbtn2->autoSize = flash::text::TextFieldAutoSize::CENTER;
	//initbtn2->x = (button4->width - initbtn2->textWidth) * 0.5f;
	//initbtn2->y = (button4->height- initbtn2->textHeight) * 0.5f;
	//initbtn2->selectable = false;
	//button4->addChild(initbtn2);



}




extern "C"
	void mainLoop()
{

	//start timer to display loading before actual load
	if(loading){
	framePassed+=1;
	}


	if(framePassed>frameNeeded)
	{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	FlashListener* flashListener = new FlashListener();
	scene->setCharacterListener(flashListener);
	toggleLog(true);
	LOG("Loading python...");
	initPython("../../Python26/Libs");
	initSBSceneSetting();
	SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
	sim->setupTimer();
	LOG("Set up timer ...");

	// start up script (the data folder has follow the hierarchy)
	scene->setMediaPath("/root/data");
	scene->addAssetPath("script", "sbm-common/scripts");
	scene->addAssetPath("script", "sbm-simcoach");
	
	scene->runScript("default-init.py");
	sbInited=true;
	printf("ininiscript\n");
	
	framePassed=0;
	loading =false;


	flash::display::Stage stage = internal::get_Stage();
	stage->removeChild(picture);
	stage->removeChild(loader);

	//sbInited=true;
	}


	 if(sbInited){
	//printf("inloop\n");
	smartbodyLoop();
    glutMainLoopBody();
	 }
}


int main(int argc, char **argv)
{
	sbInited=false;
    // flascc comes with a normal BSD libc so everything you would expect to
    // be present should work out-of-the-box. This example just shows a
    // simple message formatted using printf.
    //
    // When compiled as a projector this message will be to stdout just like
    // a normal commandline application. When compiled to a SWF it will be
    // displayed in a textfield on the stage. This behavior is overrideable
    // as you will see in later samples.
	initFlash();

	/*initSB();*/
	initStage3D(argc, argv);



    
    printf("Hello World\n");
}