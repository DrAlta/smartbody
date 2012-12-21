#include "vhcl.h"
#include "JointMapViewer.h"
#include <sb/SBJointMapManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sk/sk_joint.h>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <sstream>
#include <cstring>
#include <FL/Fl.H>
#include <FL/gl.h>
#include <GL/glu.h>
#include <FL/glut.H>
#include <sr/sr_gl.h>
#include <sr/sr_gl_render_funcs.h>
#include <sr/sr_sphere.h>
#include <sr/sr_sn_shape.h>

#ifndef WIN32
#define _strdup strdup
#endif


SkeletonViewer::SkeletonViewer( int x, int y, int w, int h, char* name ) : Fl_Gl_Window(x,y,w,h,name)
{
	skeletonScene = NULL;
	skeleton = NULL;
	jointMapName = "";
	focusJointName = "";
}

SkeletonViewer::~SkeletonViewer()
{

}


void SkeletonViewer::setJointMap( std::string mapName )
{
	jointMapName = mapName;
}


void SkeletonViewer::setFocusJointName( std::string focusName )
{
	focusJointName = focusName;

	if (skeleton)
	{
		float defaultRadius, defaultLen;
		skeletonScene->get_defaults(defaultRadius,defaultLen);
		//skeletonScene->set_skeleton_radius(defaultRadius*10.f);

		std::vector<std::string> skelJointNames = skeleton->getJointNames();
		for (unsigned int i=0;i<skelJointNames.size();i++)
		{
			std::string jname = skelJointNames[i];
			SkJoint* joint = skeleton->search_joint(jname.c_str());
			if (focusJointName == jname)
			{
				//LOG("focus joint name = %s",jname.c_str());
				skeletonScene->setJointColor(joint, SrColor(1.f,0.f,0.f));
				skeletonScene->setJointRadius(joint, defaultRadius*2.5f);
			}
			else
			{
				//skeletonScene->setJointColor(joint, SrColor(1.f,1.f,1.f));
				skeletonScene->setJointColor(joint, SrColor(1.f,1.f,1.f));
				skeletonScene->setJointRadius(joint, defaultRadius);
			}
		}
	}
	
}

void SkeletonViewer::updateLights()
{
	SrLight light;		
	light.directional = true;
	light.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
	light.position = SrVec( 100.0, 250.0, 400.0 );
	//	light.constant_attenuation = 1.0f/cam.scale;
	light.constant_attenuation = 1.0f;
	lights.push_back(light);

	SrLight light2 = light;
	light2.directional = true;
	light2.diffuse = SrColor( 0.8f, 0.8f, 0.8f );
	light2.position = SrVec( 100.0, 500.0, -1000.0 );
	//	light2.constant_attenuation = 1.0f;
	//	light2.linear_attenuation = 2.0f;
	lights.push_back(light2);
}

void SkeletonViewer::setSkeleton( std::string skelName )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBSkeleton* sk = scene->getSkeleton(skelName);
	if (!sk) return;

	if (skeletonScene)
	{
		delete skeletonScene;
		skeletonScene = NULL;
	}
	skeleton = sk;
	skeletonScene = new SkScene();
	skeletonScene->ref();
	skeletonScene->init(sk);
	skeletonScene->set_visibility(true,false,false,false);
	focusOnSkeleton();
}

void SkeletonViewer::init_opengl()
{
	// valid() is turned on by fltk after draw() returns
	glViewport ( 0, 0, w(), h() );
	glEnable ( GL_DEPTH_TEST );
	glEnable ( GL_LIGHT0 ); 
	glEnable ( GL_LIGHTING );

	//glEnable ( GL_BLEND ); // for transparency
	//glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glCullFace ( GL_BACK );
	glDepthFunc ( GL_LEQUAL );
	glFrontFace ( GL_CCW );

	glEnable ( GL_POLYGON_SMOOTH );

	//glEnable ( GL_LINE_SMOOTH );
	//glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST );

	glEnable ( GL_POINT_SMOOTH );
	glPointSize ( 2.0 );

	glShadeModel ( GL_SMOOTH );		
}


void SkeletonViewer::focusOnSkeleton()
{
	SrBox sceneBox;
	if (!skeleton) return;

	sceneBox = skeleton->getBoundingBox();	
	camera.view_all(sceneBox, camera.fovy);	
	float scale = skeleton->getCurrentHeight();
	float znear = 0.01f*scale;
	float zfar = 100.0f*scale;
	camera.setNearPlane(znear);
	camera.setFarPlane(zfar);

}

void SkeletonViewer::draw()
{
	make_current();

	if (!visible()) 
		return;
	if (!valid()) 
	{
		init_opengl();
		valid(1);
	}

	glViewport ( 0, 0, w(), h() );		
	SrMat mat ( SrMat::NotInitialized );
// 	static int counter = 0;
// 	counter = counter%1000;
// 	float color = ((float)counter)/1000.f;
// 	counter++;
	glClearColor ( SrColor(0.5f,0.5f,0.5f));
	//glClearColor(SrColor(color,color,color));
	//----- Clear Background --------------------------------------------	
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable ( GL_LIGHTING );
	for (size_t x = 0; x < lights.size(); x++)
	{
		glLight ( x, lights[x] );		
	}

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.08f);	

	static GLfloat mat_emissin[] = { 0.0,  0.0,    0.0,    1.0 };
	static GLfloat mat_ambient[] = { 0.0,  0.0,    0.0,    1.0 };
	static GLfloat mat_diffuse[] = { 1.0,  1.0,    1.0,    1.0 };
	static GLfloat mat_speclar[] = { 0.0,  0.0,    0.0,    1.0 };
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emissin );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_speclar );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 0.0 );
	glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	//glEnable( GL_COLOR_MATERIAL );
	glEnable( GL_NORMALIZE );

	//----- Set Projection ----------------------------------------------
	camera.aspect = (float)w()/(float)h();

	glMatrixMode ( GL_PROJECTION );
	glLoadMatrix ( camera.get_perspective_mat(mat) );

	//----- Set Visualisation -------------------------------------------
	glMatrixMode ( GL_MODELVIEW );
	glLoadMatrix ( camera.get_view_mat(mat) );

	glScalef ( camera.scale, camera.scale, camera.scale );
	if (skeletonScene)
	{	
		renderFunction.apply(skeletonScene);	
		drawJointMapLabels(jointMapName);
	}	
}

void SkeletonViewer::drawJointMapLabels( std::string jointMapName )
{
	//glPushAttrib(GL_LIGHTING | GL_COLOR_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	float sceneScale = scene->getScale();
	if (sceneScale == 0.0f)
		sceneScale = 1.0f;
	float textSize = skeleton->getCurrentHeight()*0.0002f;
	
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	SmartBody::SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);		
	std::vector<std::string> skelJointNames = skeleton->getJointNames();
	for (unsigned int j = 0; j < skelJointNames.size(); j++)
	{
		bool highLight = false;
		std::string jointName = skelJointNames[j];
		//if (jointMap->getMapTarget(jointName) == "")
		//	continue;
		if (jointName == focusJointName)
			highLight = true;

		if (!highLight)
			continue;

		std::string source = jointName;
		std::string target = jointMap->getMapTarget(jointName);
		SkJoint* joint = skeleton->search_joint(source.c_str());
		if (!joint) continue;		

		const SrMat& mat = joint->gmat();
		glPushMatrix();
		glMultMatrixf((const float*) mat);
		float modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX , modelview);
		// undo all rotations
		// beware all scaling is lost as well 
		for( int a=0; a<3; a++ ) 
		{
			for( int b=0; b<3; b++ ) {
				if ( a==b )
					modelview[a*4+b] = 1.0;
				else
					modelview[a*4+b] = 0.0;
			}
		}
		// set the modelview with no rotations
		glLoadMatrixf(modelview);		
		if (highLight)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			glScalef(textSize*1.5f, textSize*1.5f, textSize*1.5f);				
		}
		else
		{
			glColor3f(1.0f, 1.0f, 0.0f);
			glScalef(textSize, textSize, textSize);			
		}		
		glutStrokeString(GLUT_STROKE_ROMAN, (const unsigned char*) source.c_str());
		glPopMatrix();
		glColor3f(1.0f,1.0f,1.0f);
	}
	//glPopAttrib();	
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

const std::string spineJointNames[7] = {"base","spine1", "spine2", "spine3", "spine4", "spine5", "skullbase" };
const std::string leftArmJointNames[5] = {"l_sternoclavicular","l_shoulder", "l_elbow", "l_forearm", "l_wrist" };
const std::string rightArmJointNames[5] = {"r_sternoclavicular","r_shoulder", "r_elbow", "r_forearm", "r_wrist" };
const std::string leftHandJointNames[20] = {"l_thumb1","l_thumb2", "l_thumb3", "l_thumb4", "l_index1", "l_index2","l_index3","l_index4","l_middle1", "l_middle2", "l_middle3", "l_middle4", "l_ring1", "l_ring2", "l_ring3","l_ring4", "l_pinky1", "l_pinky2", "l_pinky3","l_pinky4"};
const std::string rightHandJointNames[20] = {"r_thumb1","r_thumb2", "r_thumb3", "r_thumb4", "r_index1", "r_index2","r_index3","r_index4","r_middle1", "r_middle2", "r_middle3", "r_middle4", "r_ring1", "r_ring2", "r_ring3","r_ring4","r_pinky1", "r_pinky2", "r_pinky3","r_pinky4"};

const std::string leftLegJointNames[5] = { "l_hip", "l_knee", "l_ankle", "l_forefoot", "l_toe" };
const std::string rightLegJointNames[5] = { "r_hip", "r_knee", "r_ankle", "r_forefoot", "r_toe" };

JointMapViewer::JointMapViewer(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	begin();	
	int curY = 10;
	int startY = 10;
	_charName ="";
	_jointMapName = "";
	_skelName = "";

	Fl_Group* leftGroup	= new Fl_Group(10, startY, w - 600, h - startY, "");
	leftGroup->begin();
	_choiceCharacters = new Fl_Choice(110, curY, 150, 20, "Character");
	//choiceCharacters->callback(CharacterCB, this);
	std::vector<std::string> characters = scene->getCharacterNames();
	for (size_t c = 0; c < characters.size(); c++)
	{
		_choiceCharacters->add(characters[c].c_str());
	}	
	_choiceCharacters->callback(SelectCharacterCB,this);
	curY += 25;

	_choiceJointMaps = new Fl_Choice(110, curY, 150, 20, "JointMaps");
	//choiceCharacters->callback(CharacterCB, this);
	//std::vector<std::string> characters = scene->getCharacterNames();
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	std::vector<std::string> jointMapNames = jointMapManager->getJointMapNames();	
	for (size_t c = 0; c < jointMapNames.size(); c++)
	{
		_choiceJointMaps->add(jointMapNames[c].c_str());
	}
	_choiceJointMaps->callback(SelectMapCB,this);

	curY += 25;	
// 
// 
// 	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();
// 	std::map<std::string, SmartBody::SBBehaviorSet*>& behavSets = behavMgr->getBehaviorSets();
// 
	
	for (int i=0;i<7;i++) standardJointNames.push_back(spineJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(leftArmJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(rightArmJointNames[i]);
	for (int i=0;i<20;i++) standardJointNames.push_back(leftHandJointNames[i]);
	for (int i=0;i<20;i++) standardJointNames.push_back(rightHandJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(leftLegJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(rightLegJointNames[i]);

	curY += 25;
	
 	_scrollGroup = new Fl_Scroll(10, curY, this->w() - 600, 450, "");
 	_scrollGroup->type(Fl_Scroll::VERTICAL);
 	_scrollGroup->begin(); 
	scrollY = curY;
	
	
	_scrollGroup->end();
	curY += 450  + 25;
	_buttonApply = new Fl_Button(100, curY, 60, 20, "Apply Map");
	_buttonApply->callback(ApplyMapCB, this);
	_buttonCancel = new Fl_Button(180, curY, 60, 20, "Cancel");
	_buttonCancel->callback(CancelCB, this);
	leftGroup->end();

	Fl_Group* rightGroup = new Fl_Group(w-580 + 20, startY, 580 , h - startY, "");
	rightGroup->begin();
	skeletonViewer = new SkeletonViewer(w-580+20, startY, 580, h-startY, "Skeleton");	
	rightGroup->end();
	rightGroup->resizable(skeletonViewer);
	this->resizable(rightGroup);
	end();

// 	int numChildren = _scrollGroup->children();
// 	for (int i=0;i<numChildren;i++)
// 	{
// 		JointMapInputChoice* input = dynamic_cast<JointMapInputChoice*>(_scrollGroup->child(i));
// 		if (input)
// 			input->setViewer(skeletonViewer);
// 	}

	if (characters.size() > 0)
	{		
		_choiceCharacters->value(0);	
		setCharacterName(_choiceCharacters->text());
		//updateCharacter();
	}

	if (jointMapNames.size() > 0) 
	{
		_choiceJointMaps->value(0);
		setJointMapName(_choiceJointMaps->text());
		//updateSelectMap();
	}		
}


void JointMapViewer::updateJointLists()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBSkeleton* skel = scene->getSkeleton(_skelName);
	int tempY = scrollY;
	std::vector<std::string> skelJointNames = skel->getJointNames();
	for (unsigned int i=0;i<_jointChoiceList.size();i++)
	{
		JointMapInputChoice* input = _jointChoiceList[i];
		delete input;
	}
	_jointChoiceList.clear();
	_scrollGroup->clear();
	_scrollGroup->begin();
	for (unsigned int i=0;i<skelJointNames.size();i++)
	{
		std::string name = skelJointNames[i];
		//LOG("joint name = %s",name.c_str());
		//Fl_Check_Button* check = new Fl_Check_Button(20, curY, 100, 20, _strdup(name.c_str()));
		//Fl_Group* jointMapGroup = new Fl_Group(20, curY , 200, 20, _strdup(name.c_str()));
		//Fl_Input* input = new Fl_Input(100 , scrollY, 150, 20, _strdup(name.c_str()));
		JointMapInputChoice* choice = new JointMapInputChoice(140, tempY, 190, 20, _strdup(name.c_str()));
		_jointChoiceList.push_back(choice);		
		choice->input()->when(FL_WHEN_CHANGED);
		choice->input()->callback(JointNameChange,this);
		choice->menubutton()->when(FL_WHEN_CHANGED);
		choice->menubutton()->callback(JointNameChange,this);
		choice->setViewer(skeletonViewer);
		tempY += 25;
	}
	_scrollGroup->end();
}


JointMapViewer::~JointMapViewer()
{
}

void JointMapViewer::hideButtons()
{
	_buttonApply->hide();
	_buttonCancel->hide();
}

void JointMapViewer::setCharacterName( std::string charName )
{
	_charName = charName;
	for (int c = 0; c < _choiceCharacters->size(); c++)
	{
		if (charName == _choiceCharacters->text(c))
		{
			_choiceJointMaps->value(c);
			break;
		}
	}
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBCharacter* sbChar = scene->getCharacter(charName);
	if (sbChar && skeletonViewer)
	{
		skeletonViewer->setSkeleton(sbChar->getSkeleton()->getName());
		_skelName = sbChar->getSkeleton()->getName();
	}
	updateJointLists();
	updateCharacter();	
}

void JointMapViewer::setJointMapName( std::string jointMapName )
{
	_jointMapName = jointMapName;
	for (int c = 0; c < _choiceJointMaps->size(); c++)
	{
		if (jointMapName == _choiceJointMaps->text(c))
		{
			_choiceJointMaps->value(c);
			break;
		}
	}

	updateSelectMap();
	updateCharacter();	
	if (skeletonViewer)
	{
		skeletonViewer->setJointMap(jointMapName);
		skeletonViewer->standardJointNames = standardJointNames;
	}
}

void JointMapViewer::JointNameChange( Fl_Widget* widget, void* data )
{
	Fl_Input* input = dynamic_cast<Fl_Input*>(widget);
	Fl_Menu_Button* menuButton = dynamic_cast<Fl_Menu_Button*>(widget);

	Fl_Input_Choice* inputChoice = NULL;
	if (input)
		inputChoice = dynamic_cast<JointMapInputChoice*>(input->parent());
	if (menuButton)
		inputChoice = dynamic_cast<JointMapInputChoice*>(menuButton->parent());
	if (inputChoice)
	{
		JointMapViewer* viewer = (JointMapViewer*) data;
		viewer->updateJointName(inputChoice);
	}	
}


void JointMapViewer::updateJointName( Fl_Input_Choice* jointChoice )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string charName = _charName;
	SmartBody::SBCharacter* curChar = scene->getCharacter(charName);
	SmartBody::SBSkeleton* charSk = curChar->getSkeleton();		
	int valueIndex = jointChoice->menubutton()->value();	
	std::string choiceStr = "";
	if (valueIndex >= 0)
	{
		jointChoice->value(valueIndex);
		choiceStr = jointChoice->value();
		if (choiceStr == "--empty--")
			jointChoice->value("");
	}
	jointChoice->clear();
	jointChoice->add("--empty--"); // add empty string as the first choice
	std::string filterLabel = jointChoice->value();
	for (unsigned int i=0;i<standardJointNames.size();i++)
	{
		std::string& jname = standardJointNames[i];
		if (jname.find(filterLabel) != std::string::npos)
		{
			jointChoice->add(jname.c_str());			
		}
	}		
	if (valueIndex >= 0)
	{
		if (choiceStr == "--empty--")
			jointChoice->value("");
		else
			jointChoice->value(choiceStr.c_str());		
	}		
}

void JointMapViewer::updateCharacter()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string charName = _charName;
	SmartBody::SBCharacter* curChar = scene->getCharacter(charName);
	if (!curChar) return;

	SmartBody::SBSkeleton* charSk = curChar->getSkeleton();
	skeletonJointNames = charSk->getJointNames();

	int numChildren = _scrollGroup->children();
	for (int i=0;i<numChildren;i++)
	{
		JointMapInputChoice* input = dynamic_cast<JointMapInputChoice*>(_scrollGroup->child(i));
		if (!input)
		{
			continue;
		}			
		updateJointName(input);	
	}
}

void JointMapViewer::updateSelectMap()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string jointMapName = _jointMapName;
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	SmartBody::SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);
	if (!jointMap) return;

	_jointMapName = jointMapName;
	int numChildren = _scrollGroup->children();
	for (int i=0;i<numChildren;i++)
	{
		Fl_Input_Choice* input = dynamic_cast<JointMapInputChoice*>(_scrollGroup->child(i));
		if (input)
		{
			std::string sourceName = input->label();
			std::string targetName = jointMap->getMapTarget(sourceName);
			if (targetName == "")
			{
				input->value("");
			}
			else
			{
				input->value(targetName.c_str());
			}
		}
	}
}


void JointMapViewer::applyJointMap()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string jointMapName = _jointMapName;
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	SmartBody::SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);

	SmartBody::SBCharacter* curChar = scene->getCharacter(_choiceCharacters->text());
	if (!curChar || !jointMap) return;
	int numChildren = _scrollGroup->children();
	jointMap->clearMapping();
	for (int i=0;i<numChildren;i++)
	{
		Fl_Input_Choice* input = dynamic_cast<JointMapInputChoice*>(_scrollGroup->child(i));
		if (input)
		{
			std::string sourceName = input->label();
			std::string targetName = input->value();		
			jointMap->setMapping(sourceName, targetName);
		}
	}	
	SmartBody::SBSkeleton* sceneSk = scene->getSkeleton(curChar->getSkeleton()->getName());
	jointMap->applySkeleton(sceneSk);
	jointMap->applySkeleton(curChar->getSkeleton());
	// in addition to update the skeleton, we also need to update the character controllers so all joint names are mapped correctly.
	//curChar->ct_tree_p->child_channels_updated(NULL);
}

void JointMapViewer::SelectMapCB( Fl_Widget* widget, void* data )
{
	Fl_Choice* mapChoice = dynamic_cast<Fl_Choice*>(widget);	
	JointMapViewer* viewer = (JointMapViewer*) data;
	viewer->setJointMapName(mapChoice->text());
	//viewer->updateSelectMap();
	//viewer->updateCharacter();	
}


void JointMapViewer::SelectCharacterCB( Fl_Widget* widget, void* data )
{
	JointMapViewer* viewer = (JointMapViewer*) data;	
	Fl_Choice* charChoice = dynamic_cast<Fl_Choice*>(widget);	
	viewer->setCharacterName(charChoice->text());
	//viewer->updateCharacter();	
}


void JointMapViewer::ApplyMapCB(Fl_Widget* widget, void* data)
{
	JointMapViewer* viewer = (JointMapViewer*) data;
	viewer->applyJointMap();
	viewer->updateCharacter();

// 	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();
// 
// 	// run the script associated with any checked behavior sets
// 	int numChildren = viewer->_scrollGroup->children();
// 	for (int c = 0; c < numChildren; c++)
// 	{
// 		Fl_Check_Button* check = dynamic_cast<Fl_Check_Button*>(viewer->_scrollGroup->child(c));
// 		if (check)
// 		{
// 			if (check->value())
// 			{
// 				SmartBody::SBBehaviorSet* behavSet = behavMgr->getBehaviorSet(check->label());
// 				if (behavSet)
// 				{
// 					LOG("Retargetting %s...", check->label());
// 					const std::string& script = behavSet->getScript();
// 					SmartBody::SBScene::getScene()->runScript(script.c_str());
// 					std::stringstream strstr;
// 					strstr << "setupBehaviorSet()";
// 					SmartBody::SBScene::getScene()->run(strstr.str());
// 					std::stringstream strstr2;
// 					strstr2 << "retargetBehaviorSet('" << viewer->getCharacterName() << "', '" << viewer->getSkeletonName() << "')";
// 					SmartBody::SBScene::getScene()->run(strstr2.str());
// 				}
// 			}
// 		}
// 	}
	viewer->hide();
}

void JointMapViewer::CancelCB(Fl_Widget* widget, void* data)
{
	JointMapViewer* viewer = (JointMapViewer*) data;

	viewer->hide();
}

void JointMapViewer::draw()
{
	if (skeletonViewer)
		skeletonViewer->redraw();
// 	int numChildren = _scrollGroup->children();
// 	for (int i=0;i<numChildren;i++)
// 	{
// 		Fl_Input_Choice* input = dynamic_cast<Fl_Input_Choice*>(_scrollGroup->child(i));
// 		
// 	}
	Fl_Double_Window::draw();
}


JointMapInputChoice::JointMapInputChoice( int x, int y, int w, int h, char* name ) : Fl_Input_Choice(x,y,w,h,name)
{
	skelViewer = NULL;
}

JointMapInputChoice::~JointMapInputChoice()
{

}

int JointMapInputChoice::handle( int event )
{
	int ret = -1;
	switch ( event )	
	{  
	case FL_PUSH:
		//LOG("widget %s is in focus",label());	
		ret = 1;
		break;
	}
	if (ret == 1 && skelViewer)
	{
		skelViewer->setFocusJointName(label());				
	}
	
	return Fl_Input_Choice::handle(event);
}

void JointMapInputChoice::setViewer( SkeletonViewer* viewer )
{
	skelViewer = viewer;
}