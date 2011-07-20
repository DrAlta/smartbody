/*
*  main.cpp - part of SmartBody Project's OgreViewer
*  Copyright (C) 2009  University of Southern California
*
*  SmartBody is free software: you can redistribute it and/or
*  modify it under the terms of the Lesser GNU General Public License
*  as published by the Free Software Foundation, version 3 of the
*  license.
*s
*  SmartBody is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  Lesser GNU General Public License for more details.
*
*  You should have received a copy of the Lesser GNU General Public
*  License along with SmartBody.  If not, see:
*      http://www.gnu.org/licenses/lgpl-3.0.txt
*
*  CONTRIBUTORS:
*      Ed Fast, USC
*      Andrew n marshall, USC
*	   Deepali Mendhekar, USC
*	   Shridhar Ravikumar, USC
*      Arno Hartholt, USC
*/

#include "vhcl.h"
#include <windows.h>
#include "bonebus.h"
#include "vhmsg-tt.h"

#include <Ogre.h>
#include <OgreTagPoint.h>
#include <ExampleApplication.h>

#include <map>
#include <string>

using std::string;
using std::vector;
using namespace bonebus;


#define FPS_LIMIT 31

Entity * ent;
SceneNode * mSceneNode;

std::string skeleton[ 115 ];

// Event handler to animate
class SkeletalAnimationFrameListener : public ExampleFrameListener
{
private:
	SceneManager * mSceneMgr;
	BoneBusServer * m_bonebus;

	bool m_ogreMouseEnabled;
	
protected:
	bool mQuit;

public:
	SkeletalAnimationFrameListener(RenderWindow * win, Camera * cam, const std::string & debugText, SceneManager * mgr, BoneBusServer * bonebus) : ExampleFrameListener( win, cam )
	{
		mDebugText = debugText;
		mSceneMgr = mgr;
		m_bonebus = bonebus;
		mQuit = false;
		m_ogreMouseEnabled = true;


		// turn off mouse look by default
		SetOgreMouse( false );
	}


	void windowFocusChange( RenderWindow * rw )
	{
		rw->setActive( true );
	}

	bool processUnbufferedKeyInput(const FrameEvent& evt)
	{
		if(mKeyboard->isKeyDown(OIS::KC_A))
			mTranslateVector.x = -mMoveScale;	// Move camera left

		if(mKeyboard->isKeyDown(OIS::KC_D))
			mTranslateVector.x = mMoveScale;	// Move camera RIGHT

		if(mKeyboard->isKeyDown(OIS::KC_UP) || mKeyboard->isKeyDown(OIS::KC_W) )
			mTranslateVector.z = -mMoveScale;	// Move camera forward

		if(mKeyboard->isKeyDown(OIS::KC_DOWN) || mKeyboard->isKeyDown(OIS::KC_S) )
			mTranslateVector.z = mMoveScale;	// Move camera backward

		if(mKeyboard->isKeyDown(OIS::KC_PGUP) || mKeyboard->isKeyDown(OIS::KC_Q) )
			mTranslateVector.y = mMoveScale;	// Move camera up

		if(mKeyboard->isKeyDown(OIS::KC_PGDOWN) || mKeyboard->isKeyDown(OIS::KC_E) )
			mTranslateVector.y = -mMoveScale;	// Move camera down

		if(mKeyboard->isKeyDown(OIS::KC_RIGHT))
			mCamera->yaw(-mRotScale);

		if(mKeyboard->isKeyDown(OIS::KC_LEFT))
			mCamera->yaw(mRotScale);

		if( mKeyboard->isKeyDown(OIS::KC_ESCAPE))
			return false;

       	if( mKeyboard->isKeyDown(OIS::KC_Z) && mTimeUntilNextToggle <= 0 )
		{
			mStatsOn = !mStatsOn;
			showDebugOverlay(mStatsOn);
			mTimeUntilNextToggle = 1;
		}

		if( mKeyboard->isKeyDown(OIS::KC_M) && mTimeUntilNextToggle <= 0 )
		{
			switch(mFiltering)
			{
			case TFO_BILINEAR:
				mFiltering = TFO_TRILINEAR;
				mAniso = 1;
				break;
			case TFO_TRILINEAR:
				mFiltering = TFO_ANISOTROPIC;
				mAniso = 8;
				break;
			case TFO_ANISOTROPIC:
				mFiltering = TFO_BILINEAR;
				mAniso = 1;
				break;
			default: break;
			}
			MaterialManager::getSingleton().setDefaultTextureFiltering(mFiltering);
			MaterialManager::getSingleton().setDefaultAnisotropy(mAniso);

			showDebugOverlay(mStatsOn);
			mTimeUntilNextToggle = 1;
		}

		if(mKeyboard->isKeyDown(OIS::KC_SYSRQ) && mTimeUntilNextToggle <= 0)
		{
			std::ostringstream ss;
			ss << "screenshot_" << ++mNumScreenShots << ".png";
			mWindow->writeContentsToFile(ss.str());
			mTimeUntilNextToggle = 0.5;
			mDebugText = "Saved: " + ss.str();
		}

		if(mKeyboard->isKeyDown(OIS::KC_N) && mTimeUntilNextToggle <=0)
		{
			mSceneDetailIndex = (mSceneDetailIndex+1)%3 ;
			switch(mSceneDetailIndex) {
				case 0 : mCamera->setPolygonMode(PM_SOLID); break;
				case 1 : mCamera->setPolygonMode(PM_WIREFRAME); break;
				case 2 : mCamera->setPolygonMode(PM_POINTS); break;
			}
			mTimeUntilNextToggle = 0.5;
		}

		static bool displayCameraDetails = false;
		if(mKeyboard->isKeyDown(OIS::KC_P) && mTimeUntilNextToggle <= 0)
		{
			displayCameraDetails = !displayCameraDetails;
			mTimeUntilNextToggle = 0.5;
			if (!displayCameraDetails)
				mDebugText = "";
		}

		// display appropriate scenes
		if(mKeyboard->isKeyDown(OIS::KC_1))
		{
			mSceneMgr->getSceneNode("world_scene_ft")->setVisible(false);
			mSceneMgr->getSceneNode("world_scene_cm")->setVisible(false);
			mSceneMgr->getSceneNode("plane_node")->setVisible(false); 
		    mSceneMgr->getSceneNode("world_scene_vh")->setVisible(true);
		}
		if(mKeyboard->isKeyDown(OIS::KC_2))
		{
			mSceneMgr->getSceneNode("world_scene_ft")->setVisible(false);
			mSceneMgr->getSceneNode("world_scene_cm")->setVisible(true);
			mSceneMgr->getSceneNode("plane_node")->setVisible(false);
			mSceneMgr->getSceneNode("world_scene_vh")->setVisible(false);
		}
		if(mKeyboard->isKeyDown(OIS::KC_3))
		{
			mSceneMgr->getSceneNode("world_scene_ft")->setVisible(true);
			mSceneMgr->getSceneNode("world_scene_cm")->setVisible(false);
			mSceneMgr->getSceneNode("plane_node")->setVisible(false);
			mSceneMgr->getSceneNode("world_scene_vh")->setVisible(false);
		}
		if(mKeyboard->isKeyDown(OIS::KC_4))
		{
			mSceneMgr->getSceneNode("world_scene_ft")->setVisible(false);
			mSceneMgr->getSceneNode("world_scene_cm")->setVisible(false);
			mSceneMgr->getSceneNode("plane_node")->setVisible(true);
			mSceneMgr->getSceneNode("world_scene_vh")->setVisible(false);
		}


		if ( mKeyboard->isKeyDown( OIS::KC_J ) )
		{
			SetOgreMouse( !m_ogreMouseEnabled );
		}


		// Print camera details
		if(displayCameraDetails)
			mDebugText = "P: " + StringConverter::toString(mCamera->getDerivedPosition()) +
						 " " + "O: " + StringConverter::toString(mCamera->getDerivedOrientation());

		// Return true to continue rendering
		return true;
	}


	virtual bool processUnbufferedMouseInput( const FrameEvent & evt )
	{
		if ( m_ogreMouseEnabled )
		{
			return ExampleFrameListener::processUnbufferedMouseInput( evt );
		}

		return true;
	}


	bool frameStarted( const FrameEvent & evt )
	{
		if (mQuit)
		{
			return false;
		}

		if ( ExampleFrameListener::frameStarted( evt ) == false )
			return false;
		


		char temp[ 1024 ];
		sprintf( temp, "%5.2f %5.2f %5.2f", mCamera->getPosition().x, mCamera->getPosition().y, mCamera->getPosition().z );
		mDebugText = temp;


		vhmsg::ttu_poll();

		m_bonebus->Update();

		
		//Limiting the frames per second as otherwise it takes up entire CPU
		// Setting it to 30, but in effect it comes up to 60 due to granularity issues
		Ogre::Root::getSingleton().setFrameSmoothingPeriod(0);
		Ogre::Real ttW;
		ttW = 1000.0f / FPS_LIMIT - 1000.0f * evt.timeSinceLastFrame;
		if (ttW > 0)
			Sleep(ttW);

		return true;
	}


	void scheduleQuit(void)
	{
		mQuit = true;
	}



	void SetOgreMouse( const bool enabled )
	{
		m_ogreMouseEnabled = enabled;


		// There's no interface in OIS to set the coop level on individual devices at runtime.
		// There's also no interface in OIS to set the coop level on individual devices at startup,
		// so you have to tear down the entire input manager and start from scratch.
		// Hopefully there's no side-effects from this behavior

		// http://www.wreckedgames.com/forum/index.php/topic,1149.msg6170.html#msg6170


		// taken from ExampleFrameListener::windowClosed()
		LogManager::getSingletonPtr()->logMessage( "*** Destroying OIS ***" );
		if ( mInputManager )
		{
			mInputManager->destroyInputObject( mMouse );
			mInputManager->destroyInputObject( mKeyboard );
			mInputManager->destroyInputObject( mJoy );

			OIS::InputManager::destroyInputSystem( mInputManager );
			mInputManager = 0;

			mMouse = 0;
			mKeyboard = 0;
			mJoy = 0;
		}


		// taken from ExampleFrameListener::ExampleFrameListener()
		LogManager::getSingletonPtr()->logMessage( "*** Initializing OIS ***" );
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;

		mWindow->getCustomAttribute( "WINDOW", &windowHnd );
		windowHndStr << windowHnd;
		pl.insert( std::make_pair( std::string( "WINDOW" ), windowHndStr.str() ) );


		if ( m_ogreMouseEnabled )
		{
			pl.insert( std::make_pair( std::string( "w32_mouse" ), std::string( "DISCL_EXCLUSIVE" ) ) );
		}
		else
		{
			pl.insert( std::make_pair( std::string( "w32_mouse" ), std::string( "DISCL_NONEXCLUSIVE" ) ) );
		}

		pl.insert( std::make_pair( std::string( "w32_mouse" ), std::string( "DISCL_FOREGROUND" ) ) );


		mInputManager = OIS::InputManager::createInputSystem( pl );

		//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
		mKeyboard = static_cast<OIS::Keyboard *>( mInputManager->createInputObject( OIS::OISKeyboard, false ) );
		mMouse = static_cast<OIS::Mouse *>( mInputManager->createInputObject( OIS::OISMouse, false ) );
		try
		{
			mJoy = static_cast<OIS::JoyStick *>( mInputManager->createInputObject( OIS::OISJoyStick, false ) );
		}
		catch(...)
		{
			mJoy = 0;
		}
	}
};


class OgreViewerApplication : public ExampleApplication
{
	private:
		Ogre::SceneNode * sceneNode;
		Ogre::Entity * sceneEntity;
	protected:
		//Map used to store each characters initial bone positions to be 
		//used in 'OnBonePosition' function to add up with the deltas.
		std::map<int,std::map<string,Ogre::Vector3>> characterInitBonePosMap;

		std::map<std::string, std::vector<int>*> m_lastPosTimes;
		std::map<std::string, std::vector<int>*> m_lastRotTimes;
		
	public:
		OgreViewerApplication()
		{
	
		}

	protected:

		void destroyScene(void)
		{
			// Send vrProcEnd message to ActiveMQ
			vhmsg::ttu_notify2( "vrProcEnd", "renderer" );

			// Close ActiveMQ
			vhmsg::ttu_close();
		}

		void createDefaultScene()
		{
			mSceneMgr->setShadowTechnique( SHADOWTYPE_TEXTURE_MODULATIVE );
			mSceneMgr->setShadowTextureSize( 4048 );
			mSceneMgr->setShadowColour( ColourValue( 0.3f, 0.3f, 0.3f ) );

			// Setup animation default
			Animation::setDefaultInterpolationMode( Animation::IM_LINEAR );
			Animation::setDefaultRotationInterpolationMode( Animation::RIM_LINEAR );

			// Set ambient light
			mSceneMgr->setAmbientLight( ColourValue( 0.2f, 0.2f, 0.2f ) );
			//mSceneMgr->setAmbientLight( ColourValue::Black );
			

			//mSceneMgr->setSkyBox( true, "Examples/CloudyNoonSkyBox" );
			//mSceneMgr->setSkyBox( true, "Examples/SceneSkyBox2" );

			// Give it a little ambience with lights

      #if 1
                  Light * l;
                  Vector3 dir;

                  l = mSceneMgr->createLight( "WhiteLight" );
                  l->setType( Light::LT_SPOTLIGHT );
                  l->setPosition( -150.0f, 450.0f, 200.0f );
                  l->setCastShadows( true );
                  l->setPowerScale( 1.0 );
                  
                  dir = -l->getPosition();
                  //dir = Vector3( 15, 50, 0 );
                  dir.normalise();
                  l->setDirection( dir );
                  l->setDiffuseColour( 1.24f, 1.22f, 1.15f );
                  l->setSpecularColour(0.8f, 0.8f, 0.9f);

                  Light * mR_FillLight;
                  mR_FillLight = mSceneMgr->createLight("R_FillLight");
                  mR_FillLight->setType(Light::LT_SPOTLIGHT);
                  mR_FillLight->setPosition(1500,100,200);
                  mR_FillLight->setSpotlightRange(Degree(30), Degree(50));
                  dir = -mR_FillLight->getPosition();
                  dir.normalise();
                  mR_FillLight->setDirection(dir);
                  mR_FillLight->setDiffuseColour(0.32f, 0.37f, 0.4f);
                  mR_FillLight->setSpecularColour(0.32f, 0.37f, 0.4f);

                  Light * mL_FillLight;
                  mL_FillLight = mSceneMgr->createLight("L_FillLight");
                  mL_FillLight->setType(Light::LT_SPOTLIGHT);
                  mL_FillLight->setPosition(-1500.0f,100.0f,-100.0f);
                  mL_FillLight->setSpotlightRange(Degree(30.0f), Degree(50.0f));
                  dir = -mL_FillLight->getPosition();
                  dir.normalise();
                  mL_FillLight->setDirection(dir);
                  mL_FillLight->setDiffuseColour(0.45f, 0.42f, 0.40f);
                  mL_FillLight->setSpecularColour(0.45f, 0.42f, 0.40f);

                  Light * mBounceLight;
                  mBounceLight = mSceneMgr->createLight("BounceLight");
                  mBounceLight->setType(Light::LT_SPOTLIGHT);
                  mBounceLight->setPosition(-50.0f,-500.0f,400.0f);
                  mBounceLight->setSpotlightRange(Degree(30.0f), Degree(50.0f));
                  dir = -mBounceLight->getPosition();
                  dir.normalise();
                  mBounceLight->setDirection(dir);
                  mBounceLight->setDiffuseColour(0.37f, 0.37f, 0.36f);
                  mBounceLight->setSpecularColour(0.37f, 0.37f, 0.36f);
                  
      #endif

			// Position the camera
			mCamera->setPosition( 0, 140, 225 );
			mCamera->lookAt( 0, 92, 0 );

			// Open ActiveMQ
			vhmsg::ttu_set_client_callback( &OgreViewerApplication::tt_client_callback, this );

			int err = vhmsg::ttu_open();
			if ( err != vhmsg::TTU_SUCCESS )
			{
				printf("%s", "ttu_open failed!\n" );
			}

			// Register with ActiveMQ
			vhmsg::ttu_register( "vrAllCall" );
			vhmsg::ttu_register( "vrKillComponent" );

			// Send vrComponent message to ActiveMQ
			vhmsg::ttu_notify2( "vrComponent", "renderer Ogre" );


			//adding plane entity to the scene
			Plane plane;
			plane.normal = Vector3::UNIT_Y;
			plane.d = 0;
			MeshManager::getSingleton().createPlane( "Myplane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane, 1500, 1500, 20, 20, true, 1, 60, 60, Vector3::UNIT_Z );
			Entity * pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
			//pPlaneEnt->setMaterialName( "Examples/Rockwall" );
			pPlaneEnt->setMaterialName( "Rockwall" );
			pPlaneEnt->setCastShadows( false );
			mSceneMgr->getRootSceneNode()->createChildSceneNode("plane_node", Vector3( 0, 0, 0 ) )->attachObject( pPlaneEnt );
			mSceneMgr->getSceneNode("plane_node")->setVisible(false);

			// adding diagnostic scene to measure in feet
			sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("world_scene_ft");
			sceneEntity = mSceneMgr->createEntity("world_entity_ft","Diagnostic_Level(ft).mesh");
			sceneNode->attachObject(sceneEntity);
			sceneNode->setVisible(false);

			// adding diagnostic scene to measure in cm
			sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("world_scene_cm");
			sceneEntity = mSceneMgr->createEntity("world_entity_cm","Diagnostic_Level(meter_in_cm).mesh");
			sceneNode->attachObject(sceneEntity);
			sceneNode->setVisible(false);

			// adding generic VH scene 
			sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("world_scene_vh");
			sceneEntity = mSceneMgr->createEntity("world_entity_vh","vh_basic_level.mesh");
			sceneNode->attachObject(sceneEntity);
			sceneNode->setVisible(true);
		}

		// Just override the mandatory create scene method
		void createScene()
		{

			createDefaultScene();

			skeleton[ 0 ] = ""; // unused
			skeleton[ 1 ] = ""; //"skeleton";
			skeleton[ 2 ] = "base";
			skeleton[ 3 ] = "l_hip";
			skeleton[ 4 ] = "l_knee";
			skeleton[ 5 ] = "l_ankle";
			skeleton[ 6 ] = "l_forefoot";
			skeleton[ 7 ] = "l_toe";
			skeleton[ 8 ] = "r_hip";
			skeleton[ 9 ] = "r_knee";
			skeleton[ 10 ] = "r_ankle";
			skeleton[ 11 ] = "r_forefoot";
			skeleton[ 12 ] = "r_toe";
			skeleton[ 13 ] = "spine1";
			skeleton[ 14 ] = "spine2";
			skeleton[ 15 ] = "spine3";
			skeleton[ 16 ] = "spine4";
			skeleton[ 17 ] = "spine5";
			skeleton[ 18 ] = "skullbase";
			skeleton[ 19 ] = "face_top_parent";
			skeleton[ 20 ] = "brow_parent_left";
			skeleton[ 21 ] = "brow01_left";
			skeleton[ 22 ] = "brow02_left";
			skeleton[ 23 ] = "brow03_left";
			skeleton[ 24 ] = "brow04_left";
			skeleton[ 25 ] = "brow_parent_right";
			skeleton[ 26 ] = "brow01_right";
			skeleton[ 27 ] = "brow02_right";
			skeleton[ 28 ] = "brow03_right";
			skeleton[ 29 ] = "brow05_right";
			skeleton[ 30 ] = "ear_left";
			skeleton[ 31 ] = "eyeball_left";
			skeleton[ 32 ] = "upper_nose_left";
			skeleton[ 33 ] = "lower_nose_left";
			skeleton[ 34 ] = "upper_nose_right";
			skeleton[ 35 ] = "lower_nose_right";
			skeleton[ 36 ] = "lower_eyelid_right";
			skeleton[ 37 ] = "upper_eyelid_right";
			skeleton[ 38 ] = "eyeball_right";
			skeleton[ 39 ] = "ear_right";
			skeleton[ 40 ] = "lower_eyelid_left";
			skeleton[ 41 ] = "upper_eyelid_left";
			skeleton[ 42 ] = "joint18";
			skeleton[ 43 ] = "face_bottom_parent";
			skeleton[ 44 ] = "Jaw";
			skeleton[ 45 ] = "Jaw_back";
			skeleton[ 46 ] = "Jaw_front";
			skeleton[ 47 ] = "Lip_bttm_mid";
			skeleton[ 48 ] = "Lip_bttm_right";
			skeleton[ 49 ] = "Lip_bttm_left";
			skeleton[ 50 ] = "Tongue_back";
			skeleton[ 51 ] = "Tongue_mid";
			skeleton[ 52 ] = "Tongue_front";
			skeleton[ 53 ] = "Lip_top_left";
			skeleton[ 54 ] = "Lip_top_right";
			skeleton[ 55 ] = "Cheek_low_right";
			skeleton[ 56 ] = "Cheek_up_right";
			skeleton[ 57 ] = "cheek_low_left";
			skeleton[ 58 ] = "Cheek_up_left";
			skeleton[ 59 ] = "Lip_out_left";
			skeleton[ 60 ] = "Lip_out_right";
			skeleton[ 61 ] = "Lip_top_mid";
			skeleton[ 62 ] = "l_sternoclavicular";
			skeleton[ 63 ] = "l_acromioclavicular";
			skeleton[ 64 ] = "l_shoulder";
			skeleton[ 65 ] = "l_elbow";
			skeleton[ 66 ] = "l_forearm";
			skeleton[ 67 ] = "l_wrist";
			skeleton[ 68 ] = "l_pinky1";
			skeleton[ 69 ] = "l_pinky2";
			skeleton[ 70 ] = "l_pinky3";
			skeleton[ 71 ] = "l_pinky4";
			skeleton[ 72 ] = "l_ring1";
			skeleton[ 73 ] = "l_ring2";
			skeleton[ 74 ] = "l_ring3";
			skeleton[ 75 ] = "l_ring4";
			skeleton[ 76 ] = "l_middle1";
			skeleton[ 77 ] = "l_middle2";
			skeleton[ 78 ] = "l_middle3";
			skeleton[ 79 ] = "l_middle4";
			skeleton[ 80 ] = "l_index1";
			skeleton[ 81 ] = "l_index2";
			skeleton[ 82 ] = "l_index3";
			skeleton[ 83 ] = "l_index4";
			skeleton[ 84 ] = "l_thumb1";
			skeleton[ 85 ] = "l_thumb2";
			skeleton[ 86 ] = "l_thumb3";
			skeleton[ 87 ] = "l_thumb4";
			skeleton[ 88 ] = "r_sternoclavicular";
			skeleton[ 89 ] = "r_acromioclavicular";
			skeleton[ 90 ] = "r_shoulder";
			skeleton[ 91 ] = "r_elbow";
			skeleton[ 92 ] = "r_forearm";
			skeleton[ 93 ] = "r_wrist";
			skeleton[ 94 ] = "r_pinky1";
			skeleton[ 95 ] = "r_pinky2";
			skeleton[ 96 ] = "r_pinky3";
			skeleton[ 97 ] = "r_pinky4";
			skeleton[ 98 ] = "r_ring1";
			skeleton[ 99 ] = "r_ring2";
			skeleton[ 100 ] = "r_ring3";
			skeleton[ 101 ] = "r_ring4";
			skeleton[ 102 ] = "r_middle1";
			skeleton[ 103 ] = "r_middle2";
			skeleton[ 104 ] = "r_middle3";
			skeleton[ 105 ] = "r_middle4";
			skeleton[ 106 ] = "r_index1";
			skeleton[ 107 ] = "r_index2";
			skeleton[ 108 ] = "r_index3";
			skeleton[ 109 ] = "r_index4";
			skeleton[ 110 ] = "r_thumb1";
			skeleton[ 111 ] = "r_thumb2";
			skeleton[ 112 ] = "r_thumb3";
			skeleton[ 113 ] = "r_thumb4";
			
			m_bonebus.SetOnClientConnectCallback( OnClientConnect, this );
			m_bonebus.SetOnCreateCharacterFunc( OnCreateCharacter, this );
			m_bonebus.SetOnDeleteCharacterFunc( OnDeleteCharacter, this );
			m_bonebus.SetOnUpdateCharacterFunc( OnUpdateCharacter, this );
			m_bonebus.SetOnSetCharacterPositionFunc( OnSetCharacterPosition, this );
			m_bonebus.SetOnSetCharacterRotationFunc( OnSetCharacterRotation, this );
			m_bonebus.SetOnBoneRotationsFunc( OnBoneRotations, this );
			m_bonebus.SetOnBonePositionsFunc( OnBonePositions, this );
			m_bonebus.OpenConnection();

			// ask SmartBody to connect to this server if it hasn't already done so
			vhmsg::ttu_notify2("sbm", "net_check");
		}


		// Create new frame listener
		void createFrameListener()
		{
			mFrameListener = new SkeletalAnimationFrameListener( mWindow, mCamera, mDebugText, mSceneMgr, &m_bonebus );
			mRoot->addFrameListener( mFrameListener );
		}


	protected:
		static void OnClientConnect( const string & clientName, void * userData )
		{
			//printf( "Client Connected! - %s\n", clientName.c_str() );
		}


		static void OnCreateCharacter( const int characterID, const std::string & characterType, const std::string & characterName, const int skeletonType, void * userData )
		{
			//printf( "Character Create! - %d, %s, %s, %d\n", characterID, characterType.c_str(), characterName.c_str(), skeletonType );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			char charIdStr[ 16 ];
			_itoa( characterID, charIdStr, 10 );
			Entity * ent;

			if (app->mSceneMgr->hasEntity(charIdStr))
			{
				// id exists - remove it before creating the character again
				OnDeleteCharacter(characterID, userData);
			}

			try
			{
				//Create character from characterType
				ent = app->mSceneMgr->createEntity( charIdStr, characterType + ".mesh" );
			}
			catch( Ogre::ItemIdentityException& )
			{
				//Default to existing Brad character
				ent = app->mSceneMgr->createEntity( charIdStr, "Brad.mesh" );
			}
			catch( Ogre::Exception& e )
			{
				if( e.getNumber() == Ogre::Exception::ERR_FILE_NOT_FOUND ) {
					//Default to existing Brad character
					ent = app->mSceneMgr->createEntity( charIdStr, "Brad.mesh" );
				} else {
					// Re-throw exception for outer catch block
					throw;
				}
			}

			if (ent == NULL)
			{
				//printf( "Unable to create character %s", characterName);
				return;
			}

			std::vector<int>* lastPosTimes = new std::vector<int>();
			lastPosTimes->resize(115);
			std::vector<int>* lastRotTimes = new std::vector<int>();
			lastRotTimes->resize(115);
			for (int x = 0; x < 115; x++)
			{
				(*lastPosTimes)[x] = -1;
				(*lastRotTimes)[x] = -1;
			}
			app->m_lastPosTimes[charIdStr] = lastPosTimes;
			app->m_lastRotTimes[charIdStr] = lastRotTimes;

			Ogre::Skeleton* skel = NULL;

			skel = ent->getSkeleton();
		
			//Number of the skeleton's bones 
			int count = skel->getNumBones(); 
			Ogre::Bone * bone = NULL;

			//Create a map of initial bone postions for the character
			std::map<string,Ogre::Vector3> cachedInitialBonePositions;

			//Iterate each bone in skeleton
			Ogre::Skeleton::BoneIterator boneIter = skel->getBoneIterator();
			while (boneIter.hasMoreElements())
			{
				Ogre::Bone* bone = boneIter.getNext();
				cachedInitialBonePositions[bone->getName()] = bone->getInitialPosition();
			}

			//Store the map containing the initial bone position using charachter ID
			app->characterInitBonePosMap[characterID] = cachedInitialBonePositions;
			

			// Add entity to the scene node
			SceneNode * mSceneNode = app->mSceneMgr->getRootSceneNode()->createChildSceneNode( charIdStr );
			mSceneNode->attachObject( ent );
		}


		static void OnUpdateCharacter( const int characterID, const std::string & characterType, const std::string & characterName, const int skeletonType, void * userData )
		{
			//printf( "Character Update! - %d, %s, %s, %d\n", characterID, characterType.c_str(), characterName.c_str(), skeletonType );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			char charIdStr[ 16 ];
			_itoa( characterID, charIdStr, 10 );

			if (!app->mSceneMgr->hasEntity(charIdStr))
				OnCreateCharacter(characterID, characterType, characterName, skeletonType, userData);			
		}

		static void OnDeleteCharacter( const int characterID, void * userData )
		{
			//printf( "Character Delete! - %d\n", characterID );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			char charIdStr[ 16 ];
			_itoa( characterID, charIdStr, 10 );
			SceneNode * node = (SceneNode *)app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
			node->detachAllObjects();
			app->mSceneMgr->destroyEntity( charIdStr );
			app->mSceneMgr->getRootSceneNode()->removeAndDestroyChild( charIdStr );
			//Remove initial bone positions for the character
			app->characterInitBonePosMap.erase(characterID);

		
		}


		static void OnSetCharacterPosition( const int characterID, const float x, const float y, const float z, void * userData )
		{
			//printf( "Set Character Position! - %d - %5.2f %5.2f %5.2f\n", characterID, x, y, z );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			if ( characterID == -1 )
			{
				// move the camera
				app->mCamera->setPosition( x, y, z );
			}
			else
			{
				char charIdStr[ 16 ];
				_itoa( characterID, charIdStr, 10 );
				
				Node * node = app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
				node->setPosition( x, y, z );
			}
		}


		static void OnSetCharacterRotation( const int characterID, const float w, const float x, const float y, const float z, void * userData )
		{
			//printf( "Set Character Rotation! - %d - %5.2f %5.2f %5.2f %5.2f\n", characterID, w, x, y, z );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			if ( characterID == -1 )
			{
				// move the camera
				app->mCamera->setOrientation( Quaternion( w, x, y, z ) );
			}
			else
			{
				char charIdStr[ 16 ];
				_itoa( characterID, charIdStr, 10 );

				Node * node = app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
				node->setOrientation( Quaternion( w, x, y, z ) );
			}
		}


		static void OnBoneRotations( const BulkBoneRotations * bulkBoneRotations, void * userData )
		{
			//printf( "Set Bone Rotations! - %d %d %d\n", bulkBoneRotations->packetId, bulkBoneRotations->charId, bulkBoneRotations->numBoneRotations );


			OgreViewerApplication * app = (OgreViewerApplication *)userData;


			for ( int z = 0; z < app->mSceneMgr->getRootSceneNode()->numChildren(); z++ )
			{
				//OutputDebugString( string( mSceneMgr->getRootSceneNode()->getChild( z )->getName() + "\n" ).c_str() );
				//OutputDebugString( "--------------------\n" );
			}


			char charIdStrBuff[ 36 ];
			string charIdStr = string( _itoa( bulkBoneRotations->charId, charIdStrBuff, 10 ) );

			SceneNode * n;

			try
			{
				n = (SceneNode *)app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
			}
			catch ( Exception )
			{
				return;
			}

			Entity * ent = (Entity *)n->getAttachedObject( charIdStr );
			if ( ent == NULL )
			{
				return;
			}

			Ogre::Skeleton* skel = ent->getSkeleton();

			std::map<std::string, std::vector<int>*>::iterator iter = app->m_lastRotTimes.find(charIdStr);
			if (iter == app->m_lastRotTimes.end())
				return;
			std::vector<int>* lastTimes = (*iter).second;


			int i;
			for ( i = 0; i < bulkBoneRotations->numBoneRotations; i++ )
			{
				int id = bulkBoneRotations->bones[ i ].boneId;
				if ((*lastTimes)[i] >= bulkBoneRotations->time)
				{
					continue;
				}
				(*lastTimes)[i] = bulkBoneRotations->time;

				std::string & boneName = skeleton[ id ];

				if ( boneName == "" )
					continue;

				try {
					Ogre::Bone * bone = skel->getBone( boneName.c_str() );

					if ( bone )
					{
						bone->setManuallyControlled( true );

						Quaternion q;

						q.w = bulkBoneRotations->bones[ i ].rot_w;
						q.x = bulkBoneRotations->bones[ i ].rot_x;
						q.y = bulkBoneRotations->bones[ i ].rot_y;
						q.z = bulkBoneRotations->bones[ i ].rot_z;

						bone->setOrientation( q );

						//Vector3 v;
						//v.x = bulkBoneData->bones[ i ].
					}
				} catch (ItemIdentityException&) {
					printf("Could not find bone name %s", boneName.c_str());
				}
			}
		}


		static void OnBonePositions( const BulkBonePositions * bulkBonePositions, void * userData )
		{
			//printf( "Set Bone Positions! - %d %d %d\n", bulkBonePositions->packetId, bulkBonePositions->charId, bulkBonePositions->numBonePositions );


			OgreViewerApplication * app = (OgreViewerApplication *)userData;


			char charIdStrBuff[ 36 ];
			string charIdStr = string( _itoa( bulkBonePositions->charId, charIdStrBuff, 10 ) );

			SceneNode * n;

			try
			{
				n = (SceneNode *)app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
			}
			catch ( Exception )
			{
				return;
			}

			Entity * ent = (Entity *)n->getAttachedObject( charIdStr );
			if ( ent == NULL )
			{
				return;
			}

			Ogre::Skeleton* skel = ent->getSkeleton();

			std::map<std::string, std::vector<int>*>::iterator iter = app->m_lastPosTimes.find(charIdStr);
			if (iter == app->m_lastPosTimes.end())
				return;
			std::vector<int>* lastTimes = (*iter).second;

			//Get map of initial bone positions for the character
			std::map<string,Ogre::Vector3> cachedInitialBonePositions =
				app->characterInitBonePosMap[bulkBonePositions->charId];

			int i;
			for ( i = 0; i < bulkBonePositions->numBonePositions; i++ )
			{
				int id = bulkBonePositions->bones[ i ].boneId;
				if ((*lastTimes)[i] >= bulkBonePositions->time)
				{
					continue;
				}
				(*lastTimes)[i] = bulkBonePositions->time;

				std::string & boneName = skeleton[ id ];

				if ( boneName == "" )
					continue;

				try {
					Ogre::Bone * bone = skel->getBone( boneName.c_str() );

					if ( bone )
					{
						bone->setManuallyControlled( true );
						
						//Get the initial bone position for the bone using bone name
						Vector3 initialBonePosition = cachedInitialBonePositions[boneName];
						Vector3 v;

						
						//Add initial bone position to delta
						v.x = initialBonePosition.x + bulkBonePositions->bones[ i ].pos_x;
						v.y = initialBonePosition.y + bulkBonePositions->bones[ i ].pos_y;
						v.z = initialBonePosition.z + bulkBonePositions->bones[ i ].pos_z;
						

						bone->setPosition( v );
					}
				} catch (ItemIdentityException&) {
					printf("Could not find bone name %s", boneName.c_str());
				}
			}
		}


		static void tt_client_callback( const char * op, const char * args, void * user_data )
		{
		   OgreViewerApplication * app = (OgreViewerApplication *)user_data;

		   //NILOG( "ActiveMQ message received: '%s %s'\n", op, args );

		   string sOp = op;
		   string sArgs = args;
		   std::vector< string > splitArgs;
		   vhcl::Tokenize( sArgs, splitArgs );

		   if ( sOp == "vrAllCall" )
		   {
			  vhmsg::ttu_notify2( "vrComponent", "renderer Ogre" );
		   }
		   else if ( sOp == "vrKillComponent" )
		   {
			  if ( splitArgs.size() > 0 )
			  {
				 if ( splitArgs[ 0 ] == "renderer" ||
					  splitArgs[ 0 ] == "all" )
				 {
					 ((SkeletalAnimationFrameListener*)app->mFrameListener)->scheduleQuit();
				 }
			  }
		   }
		}

	protected:
		std::string mDebugText;

		BoneBusServer  m_bonebus;
};








int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// Create application object
	OgreViewerApplication app;

	try
	{
		app.go();
	}
	catch ( Exception & e )
	{
		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}

	return 0;
}
