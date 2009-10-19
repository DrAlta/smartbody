/*
*  main.cpp - part of SmartBody Project's OgreViewer
*  Copyright (C) 2009  University of Southern California
*
*  SmartBody is free software: you can redistribute it and/or
*  modify it under the terms of the Lesser GNU General Public License
*  as published by the Free Software Foundation, version 3 of the
*  license.
*
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
*/

#include "vhcl.h"

#include <windows.h>

#include <map>
#include <string>

#include "Ogre.h"
#include "OgreTagPoint.h"
#include "ExampleApplication.h"

#include "bonebus.h"

#include "vhmsg-tt.h"


using std::string;
using std::vector;


#define FPS_LIMIT 31

Entity * ent;
SceneNode * mSceneNode;

std::string skeleton[ 114 ];


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
	SkeletalAnimationFrameListener(RenderWindow * win, Camera * cam, const std::string & debugText, SceneManager * mgr, BoneBusServer * bonebus ) : ExampleFrameListener( win, cam )
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

		if(mKeyboard->isKeyDown(OIS::KC_PGUP))
			mTranslateVector.y = mMoveScale;	// Move camera up

		if(mKeyboard->isKeyDown(OIS::KC_PGDOWN))
			mTranslateVector.y = -mMoveScale;	// Move camera down

		if(mKeyboard->isKeyDown(OIS::KC_RIGHT))
			mCamera->yaw(-mRotScale);

		if(mKeyboard->isKeyDown(OIS::KC_LEFT))
			mCamera->yaw(mRotScale);

		if( mKeyboard->isKeyDown(OIS::KC_ESCAPE) || mKeyboard->isKeyDown(OIS::KC_Q) )
			return false;

       	if( mKeyboard->isKeyDown(OIS::KC_F) && mTimeUntilNextToggle <= 0 )
		{
			mStatsOn = !mStatsOn;
			showDebugOverlay(mStatsOn);
			mTimeUntilNextToggle = 1;
		}

		if( mKeyboard->isKeyDown(OIS::KC_T) && mTimeUntilNextToggle <= 0 )
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

		if(mKeyboard->isKeyDown(OIS::KC_R) && mTimeUntilNextToggle <=0)
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
			mSceneMgr->getSceneNode("plane_node")->setVisible(true);
		}
		if(mKeyboard->isKeyDown(OIS::KC_3))
		{
			mSceneMgr->getSceneNode("world_scene_ft")->setVisible(true);
			mSceneMgr->getSceneNode("world_scene_cm")->setVisible(false);
			mSceneMgr->getSceneNode("plane_node")->setVisible(false);
		}
		if(mKeyboard->isKeyDown(OIS::KC_2))
		{
			mSceneMgr->getSceneNode("world_scene_ft")->setVisible(false);
			mSceneMgr->getSceneNode("world_scene_cm")->setVisible(true);
			mSceneMgr->getSceneNode("plane_node")->setVisible(false);
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


		/*
		for ( int i = 0; i < NUM_JAIQUAS; ++i )
		{
		Real inc = evt.timeSinceLastFrame * mAnimationSpeed[ i ]; 

		if ( ( mAnimState[ i ]->getTimePosition() + inc ) >= mAnimChop )
		{
		// Loop
		// Need to reposition the scene node origin since animation includes translation
		// Calculate as an offset to the end position, rotated by the
		// amount the animation turns the character
		Quaternion rot(mAnimationRotation, Vector3::UNIT_Y);
		Vector3 startoffset = mSceneNode[i]->getOrientation() * -mSneakStartOffset;
		Vector3 endoffset = mSneakEndOffset;
		Vector3 offset = rot * startoffset;
		Vector3 currEnd = mSceneNode[i]->getOrientation() * endoffset + mSceneNode[i]->getPosition();
		mSceneNode[i]->setPosition(currEnd + offset);
		mSceneNode[i]->rotate(rot);

		mAnimState[i]->setTimePosition((mAnimState[i]->getTimePosition() + inc) - mAnimChop);
		}
		else
		{
		mAnimState[i]->addTime(inc);
		}
		}
		*/




		//Ogre::Skeleton * skel = ent->getSkeleton();

		//Ogre::Bone * bone = skel->getBone( "spine2" );
		//bone->setManuallyControlled( true );

		//bone->setOrientation( Quaternion( 1, 0, rand(), 0 ) );


		/*
		Ogre::Bone * bone = skel->getBone( "spine3" );
		bone->setOrientation( Quaternion( rand(), 0, rand(), 0 ) );
		Ogre::Bone * bone = skel->getBone( "spine4" );
		bone->setOrientation( Quaternion( rand(), 0, rand(), 0 ) );
		Ogre::Bone * bone = skel->getBone( "l_shoulder" );
		bone->setOrientation( Quaternion( rand(), 0, rand(), 0 ) );
		Ogre::Bone * bone = skel->getBone( "l_elbow" );
		bone->setOrientation( Quaternion( rand(), 0, rand(), 0 ) );
		*/



		/*
		Ogre::Bone * headBone;
		Ogre::Skeleton* skel = head->getSkeleton();
		headBone = skel->getBone( "Head" );
		headBone->setManuallyControlled( true );


		//we want the head to be facing the camera, along the vector between the camera and the head 
		//this is done in world space for simplicity
		Ogre::Vector3 headBonePosition = headBone->getWorldPosition();
		Ogre::Vector3 objectPosition = object->getPosition();
		Ogre::Vector3 between = objectPosition-headBonePosition;



		Ogre::Node * neckBone = headBone->getParent();
		Ogre::Quaternion neckBoneWorldOrientation = neckBone->getWorldOrientation();


		headBone->setOrientation( rot );
		*/

		vhmsg::ttu_poll();

		m_bonebus->Update();

		//Limiting the frames per second as otherwise it takes up entire CPU
		// Setting it to 30, but in effect it comes up to 60 due to granularity issues
		Ogre::Root::getSingleton().setFrameSmoothingPeriod(0);
		Ogre::Real ttW;
		ttW = 1000.0 / FPS_LIMIT - 1000.0 * evt.timeSinceLastFrame;
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

	public:
		OgreViewerApplication() {}

	protected:

		void destroyScene(void)
		{
			// Send vrProcEnd message to ActiveMQ
			vhmsg::ttu_notify2( "vrProcEnd", "renderer" );

			// Close ActiveMQ
			vhmsg::ttu_close();
		}

		// Just override the mandatory create scene method
		void createScene()
		{
			mSceneMgr->setShadowTechnique( SHADOWTYPE_TEXTURE_MODULATIVE );
			mSceneMgr->setShadowTextureSize( 512 );
			mSceneMgr->setShadowColour( ColourValue( 0.5, 0.5, 0.5 ) );

			// Setup animation default
			Animation::setDefaultInterpolationMode( Animation::IM_LINEAR );
			Animation::setDefaultRotationInterpolationMode( Animation::RIM_LINEAR );

			// Set ambient light
			//mSceneMgr->setAmbientLight( ColourValue( 1.0, 1.0, 1.0 ) );
			mSceneMgr->setAmbientLight( ColourValue::Black );

			//mSceneMgr->setSkyBox( true, "Examples/CloudyNoonSkyBox" );
			//mSceneMgr->setSkyBox( true, "Examples/SceneSkyBox2" );

			// Give it a little ambience with lights

	#if 1
			Light * l;
			Vector3 dir;

			l = mSceneMgr->createLight( "GreenLight" );
			l->setType( Light::LT_SPOTLIGHT );
			l->setPosition( 0, 250, 200 );
			//dir = -l->getPosition();
			dir = Vector3( 15, 205, 0 );
			dir.normalise();
			l->setDirection( dir );
			l->setDiffuseColour( 1.0, 1.0, 1.0 );
			l->setSpecularColour(0.9, 0.9, 1);

			l = mSceneMgr->createLight( "GreenLight2" );
			l->setType( Light::LT_SPOTLIGHT );
			l->setPosition( 250, 250, 0 );
			//dir = -l->getPosition();
			dir = Vector3( 15, 205, 0 );
			dir.normalise();
			l->setDirection( dir );
			l->setDiffuseColour( 1.0, 1.0, 1.0 );
			l->setSpecularColour(0.9, 0.9, 1);

			l = mSceneMgr->createLight( "GreenLight3" );
			l->setType( Light::LT_SPOTLIGHT );
			l->setPosition( -250, 250, 0 );
			//dir = -l->getPosition();
			dir = Vector3( 15, 205, 0 );
			dir.normalise();
			l->setDirection( dir );
			l->setDiffuseColour( 1.0, 1.0, 1.0 );
			l->setSpecularColour(0.9, 0.9, 1);

			l = mSceneMgr->createLight( "BlueLight" );
			l->setType( Light::LT_SPOTLIGHT );
			l->setPosition( 0, 250, -250 );
			//dir = -l->getPosition();
			dir = Vector3( 15, 205, 0 );
			dir.normalise();
			l->setDirection( dir );
			l->setDiffuseColour( 1.0, 1.0, 1.0 );
			l->setSpecularColour(0.9, 0.9, 1);



			Light * mSunLight;
			mSunLight = mSceneMgr->createLight("SunLight");
			mSunLight->setType(Light::LT_SPOTLIGHT);
			mSunLight->setPosition(1500,1750,1300);
			mSunLight->setSpotlightRange(Degree(30), Degree(50));
			dir = -mSunLight->getPosition();
			dir.normalise();
			mSunLight->setDirection(dir);
			mSunLight->setDiffuseColour(0.85, 0.85, 0.88);
			mSunLight->setSpecularColour(0.9, 0.9, 1);
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
			plane.d = 100;
			MeshManager::getSingleton().createPlane( "Myplane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane, 1500, 1500, 20, 20, true, 1, 60, 60, Vector3::UNIT_Z );
			Entity * pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
			//pPlaneEnt->setMaterialName( "Examples/Rockwall" );
			pPlaneEnt->setMaterialName( "Rockwall" );
			pPlaneEnt->setCastShadows( false );
			mSceneMgr->getRootSceneNode()->createChildSceneNode("plane_node", Vector3( 0, 0, 0 ) )->attachObject( pPlaneEnt );

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

			







			//Entity * ent;
			//SceneNode * mSceneNode;


			/*
			ent = mSceneMgr->createEntity( "doctor", "OgreDoctor.mesh" );

			// Add entity to the scene node
			mSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			mSceneNode->attachObject( ent );
			//mSceneNode->rotate( q );
			//mSceneNode->translate( mBasePositions[ i ] );

			//mAnimState = ent->getAnimationState( "Sneak" );
			//mAnimState->setEnabled( true );
			//mAnimState->setLoop( false ); // manual loop since translation involved
			//mAnimationSpeed = Math::RangeRandom(0.5, 1.5);

			mSceneNode->scale( 0.25, 0.25, 0.25 );
			mSceneNode->translate( 0, 25, 0 );
			*/










			/*
			Num Bones: 112
			Bone # 1: base
			Bone # 2: spine1
			Bone # 3: spine2
			Bone # 4: spine3
			Bone # 5: spine4
			Bone # 6: spine5
			Bone # 7: skullbase
			Bone # 8: face_top_parent
			Bone # 9: brow_parent_left
			Bone #10: brow01_left
			Bone #11: brow02_left
			Bone #12: brow03_left
			Bone #13: brow04_left
			Bone #14: ear_left
			Bone #15: eyeball_left
			Bone #16: upper_nose_left
			Bone #17: lower_nose_left
			Bone #18: lower_eyelid_left
			Bone #19: upper_eyelid_left
			Bone #20: brow_parent_right
			Bone #21: brow01_right
			Bone #22: brow02_right
			Bone #23: brow03_right
			Bone #24: brow04_right
			Bone #25: upper_eyelid_right
			Bone #26: eyeball_right
			Bone #27: lower_eyelid_right
			Bone #28: upper_nose_right
			Bone #29: lower_nose_right
			Bone #30: ear_right
			Bone #31: joint18
			Bone #32: face_bottom_parent
			Bone #33: Jaw
			Bone #34: Jaw_back
			Bone #35: Jaw_front
			Bone #36: Lip_bttm_mid
			Bone #37: Lip_bttm_left
			Bone #38: Lip_bttm_right
			Bone #39: Tongue_back
			Bone #40: Tongue_mid
			Bone #41: Tongue_front
			Bone #42: Lip_top_left
			Bone #43: cheek_low_left
			Bone #44: Cheek_up_left
			Bone #45: Lip_out_left
			Bone #46: Lip_top_mid
			Bone #47: Cheek_up_right
			Bone #48: cheek_low_right
			Bone #49: Lip_top_right
			Bone #50: Lip_out_right
			Bone #51: l_sternoclavicular
			Bone #52: l_acromioclavicular
			Bone #53: l_shoulder
			Bone #54: l_elbow
			Bone #55: l_forearm
			Bone #56: l_wrist
			Bone #57: l_pinky1
			Bone #58: l_pinky2
			Bone #59: l_pinky3
			Bone #60: l_pinky4
			Bone #61: l_ring1
			Bone #62: l_ring2
			Bone #63: l_ring3
			Bone #64: l_ring4
			Bone #65: l_middle1
			Bone #66: l_middle2
			Bone #67: l_middle3
			Bone #68: l_middle4
			Bone #69: l_index1
			Bone #70: l_index2
			Bone #71: l_index3
			Bone #72: l_index4
			Bone #73: l_thumb1
			Bone #74: l_thumb2
			Bone #75: l_thumb3
			Bone #76: l_thumb4
			Bone #77: r_sternoclavicular
			Bone #78: r_acromioclavicular
			Bone #79: r_shoulder
			Bone #80: r_elbow
			Bone #81: r_forearm
			Bone #82: r_wrist
			Bone #83: r_pinky1
			Bone #84: r_pinky2
			Bone #85: r_pinky3
			Bone #86: r_pinky4
			Bone #87: r_ring1
			Bone #88: r_ring2
			Bone #89: r_ring3
			Bone #90: r_ring4
			Bone #91: r_middle1
			Bone #92: r_middle2
			Bone #93: r_middle3
			Bone #94: r_middle4
			Bone #95: r_index1
			Bone #96: r_index2
			Bone #97: r_index3
			Bone #98: r_index4
			Bone #99: r_thumb1
			Bone #100: r_thumb2
			Bone #101: r_thumb3
			Bone #102: r_thumb4
			Bone #103: l_hip
			Bone #104: l_knee
			Bone #105: l_ankle
			Bone #106: l_forefoot
			Bone #107: l_toe
			Bone #108: r_hip
			Bone #109: r_knee
			Bone #110: r_ankle
			Bone #111: r_forefoot
			Bone #112: r_toe
			*/

			/*
			char blah[ 100 ];


			Ogre::Skeleton * skel = ent->getSkeleton();


			sprintf( blah, "Num Bones: %d\n", skel->getNumBones() );
			OutputDebugString( blah );


			Ogre::Bone * bone = skel->getBone( "spine2" );
			bone->setManuallyControlled( true );




			int i = 0;
			Ogre::Skeleton::BoneIterator it = skel->getBoneIterator();
			while ( it.hasMoreElements() )
			{
			Bone * b = it.getNext();
			i++;
			sprintf( blah, "Bone #%2d: %d - %s\n", i, b->getHandle(), b->getName().c_str() );
			//OutputDebugString( blah );
			}
			*/


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
			skeleton[ 78 ] = ""; // "l_middle3";
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
			m_bonebus.SetOnSetCharacterPositionFunc( OnSetCharacterPosition, this );
			m_bonebus.SetOnSetCharacterRotationFunc( OnSetCharacterRotation, this );
			m_bonebus.SetOnBoneRotationsFunc( OnBoneRotations, this );
			m_bonebus.SetOnBonePositionsFunc( OnBonePositions, this );
			m_bonebus.OpenConnection();
		}


		// Create new frame listener
		void createFrameListener()
		{
			mFrameListener = new SkeletalAnimationFrameListener( mWindow, mCamera, mDebugText, mSceneMgr, &m_bonebus );
			mRoot->addFrameListener( mFrameListener );
		}


	protected:
		static void OnClientConnect( string clientName, void * userData )
		{
			printf( "Client Connected! - %s\n", clientName.c_str() );
		}


		static void OnCreateCharacter( const int characterID, const std::string characterType, const std::string characterName, const int skeletonType, void * userData )
		{
			printf( "Character Create! - %d, %s, %s, %d\n", characterID, characterType.c_str(), characterName.c_str(), skeletonType );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			char charIdStr[ 16 ];
			_itoa( characterID, charIdStr, 10 );
			Entity * ent;

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
				printf( "Unable to create character %s", characterName);
				return;
			}

			Ogre::Skeleton * skel = ent->getSkeleton();
			
			//Number of the skeleton's bones 
			int count = skel->getNumBones(); 
			Ogre::Bone * bone = NULL;

			//Create a map of initial bone postions for the character
			std::map<string,Ogre::Vector3> cachedInitialBonePositions;

			//Iterate each bone in skeleton
			for(int i = 0; i < count; i++) 
			{ 
				bone = skel->getBone(i);
				//Store initial bone position against bone name
				cachedInitialBonePositions[bone->getName().c_str()] = bone->getInitialPosition();
			} 

			//Store the map containing the initial bone position using charachter ID
			app->characterInitBonePosMap[characterID] = cachedInitialBonePositions;

			//ent->setMaterialName("Examples/Grass"); 
			//ent->setVisible( false );
			//ent->setDisplaySkeleton( true );

			//ent->setVisibilityFlags( 1 );

	#if 0
			// testing - create new see-through material
			{
				static bool doctor_alpha_once = false;
				if ( !doctor_alpha_once )
				{
					doctor_alpha_once = true;

					MaterialPtr mat = MaterialManager::getSingleton().create("doctor-alpha", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
					Pass* pass = mat->getTechnique(0)->getPass(0);
					TextureUnitState* tex = pass->createTextureUnitState();
					//tex->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue( 0, 0, 0, 0 ) );
					//tex->setColourOperationEx(LBX_BLEND_MANUAL, LBS_MANUAL, LBS_CURRENT, ColourValue( 0, 0, 0, 1 ) );
					//pass->setLightingEnabled(false);
					//pass->setSceneBlending( SBT_TRANSPARENT_ALPHA );
					pass->setDepthWriteEnabled(false);
				}
			}

			ent->setMaterialName( "doctor-alpha" );
	#endif



			// Add entity to the scene node
			SceneNode * mSceneNode = app->mSceneMgr->getRootSceneNode()->createChildSceneNode( charIdStr );
			mSceneNode->attachObject( ent );

			//mSceneNode->rotate( q );
			//mSceneNode->scale( 0.25, 0.25, 0.25 );
			//mSceneNode->scale( 0.55, 0.55, 0.55 );
			//mSceneNode->translate( 0, 25, 0 );
			//mSceneNode->translate( charId * 25, 25, 0 );  // 25

			//ent->setVisible( false );




	#if 0
			// testing - create geometry for the bones
			{
				static bool once = false;

				if ( !once )
				{
					once = true;

					MaterialPtr mat = MaterialManager::getSingleton().create("red", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
					Pass* pass = mat->getTechnique(0)->getPass(0);
					TextureUnitState* tex = pass->createTextureUnitState();
					tex->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue( 1, 0, 0, 0.9 ) );
					//pass->setLightingEnabled(false);
					//pass->setSceneBlending(SBT_ADD);
					pass->setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA );
					//pass->setDepthWriteEnabled(false);

					MaterialPtr mat2 = MaterialManager::getSingleton().create("blue", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
					Pass* pass2 = mat2->getTechnique(0)->getPass(0);
					TextureUnitState* tex2 = pass2->createTextureUnitState();
					tex2->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue(0, 0, 1, 0.9 ) );
					//pass2->setLightingEnabled(false);
					//pass2->setSceneBlending(SBT_ADD);
					pass2->setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA );
					//pass2->setDepthWriteEnabled(false);

					MaterialPtr mat3 = MaterialManager::getSingleton().create("green", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
					Pass* pass3 = mat3->getTechnique(0)->getPass(0);
					TextureUnitState* tex3 = pass3->createTextureUnitState();
					tex3->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue(0, 1, 0, 0.9 ) );
					//pass2->setLightingEnabled(false);
					//pass2->setSceneBlending(SBT_ADD);
					pass3->setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA );
					//pass2->setDepthWriteEnabled(false);

					//targetEnt->setMaterialName("targeter");
				}




				Skeleton* skeleton; 
				skeleton = ent->getSkeleton(); 

				//Number of the skeleton's bones 
				int count = skeleton->getNumBones(); 

				//Create Ankle 
				for(int i=0; i<count; i++) 
				{ 
					Entity* e; 
					e = app->mSceneMgr->createEntity((string)charIdStr + "_PK_#"+StringConverter::toString(i), "Sphere.mesh"); 
					e->setMaterialName("red"); 
					TagPoint *test = ent->attachObjectToBone(skeleton->getBone(i)->getName(), e, Quaternion::IDENTITY, Vector3::ZERO); 
					test->setScale(0.005, 0.005, 0.005); 
					//mAnkles.push_back(e); 

					e->setVisibilityFlags( 2 );
				} 

				//Create Bones 
				for(int i=0; i<count; i++) 
				{ 
					Bone* currBone = skeleton->getBone(i); 

					if(currBone->numChildren() > 0) 
					{ 
						for(int j=0; j<count; j++) 
						{ 
							Bone* currChild = skeleton->getBone(j); 

							if(currBone == currChild->getParent()) 
							{ 
								Entity* e; 
								e = app->mSceneMgr->createEntity((string)charIdStr + "_PB_#"+StringConverter::toString(i)+"."+StringConverter::toString(j), "cube.mesh"); 
								e->setMaterialName("blue"); 

								Vector3 diff = currBone->getWorldOrientation().Inverse() * (currChild->getWorldPosition() - currBone->getWorldPosition()); 
								Quaternion orient = Vector3::UNIT_Z.getRotationTo(diff.normalisedCopy()); 
								Real length = diff.length(); 

								TagPoint *scaleTP = ent->attachObjectToBone(currBone->getName(), e, orient, diff.midPoint(Vector3::ZERO)); 

								Real bone_scale = 0.002; 
								scaleTP->setScale(bone_scale,bone_scale,length * 0.01); 

								//mBones.push_back(e); 

								e->setVisibilityFlags( 2 );
							} 
						} 
					} 
				}


				for(int i=0; i<count; i++) 
				{ 
					Bone* currBone = skeleton->getBone(i); 

					Matrix3 m = currBone->getLocalAxes();

					Entity* e;
					TagPoint *scaleTP;
					Quaternion orient;

					e = app->mSceneMgr->createEntity((string)charIdStr + "_AxisX_#"+StringConverter::toString(i)+".", "cube.mesh"); 
					e->setMaterialName("red"); 
					orient = Vector3::UNIT_Z.getRotationTo( m.GetColumn( 0 ) );
					scaleTP = ent->attachObjectToBone(currBone->getName(), e, orient, Vector3::ZERO);
					scaleTP->setScale( 0.001, 0.001, 0.06 );

					e = app->mSceneMgr->createEntity((string)charIdStr + "_AxisY_#"+StringConverter::toString(i)+".", "cube.mesh"); 
					e->setMaterialName("green"); 
					orient = Vector3::UNIT_Z.getRotationTo( m.GetColumn( 1 ) );
					scaleTP = ent->attachObjectToBone(currBone->getName(), e, orient, Vector3::ZERO);
					scaleTP->setScale( 0.001, 0.001, 0.06 );

					e = app->mSceneMgr->createEntity((string)charIdStr + "_AxisZ_#"+StringConverter::toString(i)+".", "cube.mesh"); 
					e->setMaterialName("blue"); 
					orient = Vector3::UNIT_Z.getRotationTo( m.GetColumn( 2 ) );
					scaleTP = ent->attachObjectToBone(currBone->getName(), e, orient, Vector3::ZERO);
					scaleTP->setScale( 0.001, 0.001, 0.06 );
				}


			}
	#endif

			//mSceneMgr->setVisibilityMask( 1 );
		}


		static void OnDeleteCharacter( const int characterID, void * userData )
		{
			printf( "Character Delete! - %d\n", characterID );

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
			printf( "Set Character Position! - %d - %5.2f %5.2f %5.2f\n", characterID, x, y, z );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			if ( characterID == -1 )
			{
				// move the camera
				app->mCamera->setPosition( -y, z, x );
			}
			else
			{
				char charIdStr[ 16 ];
				_itoa( characterID, charIdStr, 10 );
				
				Node * node = app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
				node->setPosition( -y, z, x );
			}
		}


		static void OnSetCharacterRotation( const int characterID, const float w, const float x, const float y, const float z, void * userData )
		{
			printf( "Set Character Rotation! - %d - %5.2f %5.2f %5.2f %5.2f\n", characterID, w, x, y, z );

			OgreViewerApplication * app = (OgreViewerApplication *)userData;

			if ( characterID == -1 )
			{
				// move the camera
				app->mCamera->setOrientation( Quaternion( w, -y, z, x ) );
			}
			else
			{
				char charIdStr[ 16 ];
				_itoa( characterID, charIdStr, 10 );

				Node * node = app->mSceneMgr->getRootSceneNode()->getChild( charIdStr );
				node->setOrientation( Quaternion( w, -y, z, x ) );
			}
		}


		static void OnBoneRotations( const BoneBusServer::BulkBoneRotations * bulkBoneRotations, void * userData )
		{
			printf( "Set Bone Rotations! - %d %d %d\n", bulkBoneRotations->packetId, bulkBoneRotations->charId, bulkBoneRotations->numBoneRotations );


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

			Ogre::Skeleton * skel = ent->getSkeleton();


			int i;
			for ( i = 0; i < bulkBoneRotations->numBoneRotations; i++ )
			{
				int id = bulkBoneRotations->bones[ i ].boneId;

				std::string & boneName = skeleton[ id ];

				if ( boneName == "" )
					continue;

				/*
				FQuat quat;
				quat.W = bulkBoneData->bones[ i ].rot_w;
				quat.X = bulkBoneData->bones[ i ].rot_x;
				quat.Y = bulkBoneData->bones[ i ].rot_y;
				quat.Z = bulkBoneData->bones[ i ].rot_z;

				FCoords C_test;
				C_test = FQuaternionCoords(quat);

				FRotator rot;
				rot = C_test.OrthoRotation();

				r[ i ].r = rot;
				*/

				//Stack.Logf(TEXT("****** %d - %d %d %d"), r[ i ].id, r[ i ].r.Yaw, r[ i ].r.Pitch, r[ i ].r.Roll );




				/*
				CStudioHdr * pStudioHdr = pAnimating->GetModelPtr();

				//mstudiobone_t * pBone = pStudioHdr->pBone( id );

				int boneId = pAnimating->LookupBone( boneName.c_str() );
				if ( boneId >= 0 )
				{
				mstudiobone_t * pBone = pStudioHdr->pBone( boneId );

				pBone->quat[ 0 ] = -bulkBoneData->bones[ i ].rot_x;
				pBone->quat[ 1 ] =  bulkBoneData->bones[ i ].rot_y;
				pBone->quat[ 2 ] = -bulkBoneData->bones[ i ].rot_z;
				pBone->quat[ 3 ] =  bulkBoneData->bones[ i ].rot_w;

				//Msg( "BoneName: %s\n", boneName.c_str() );



				//matrix3x4_t & matrix = pAnimating->GetBoneForWrite( boneId );

				CBoneCache * pcache = pAnimating->GetBoneCache();
				matrix3x4_t * pMat = pcache->GetCachedBone( boneId );

				if ( pMat == NULL )
				continue;


				Vector pos;
				MatrixPosition( *pMat, pos );

				Quaternion q;
				q[ 0 ] = -bulkBoneData->bones[ i ].rot_x;
				q[ 1 ] =  bulkBoneData->bones[ i ].rot_y;
				q[ 2 ] = -bulkBoneData->bones[ i ].rot_z;
				q[ 3 ] =  bulkBoneData->bones[ i ].rot_w;

				QuaternionMatrix( q, pos, *pMat );
				}
				else
				{
				Warning( "Bad Bone: %s\n", boneName.c_str() );
				}
				*/



				Ogre::Bone * bone = skel->getBone( boneName.c_str() );

				if ( bone )
				{
					bone->setManuallyControlled( true );

					Quaternion q;

					if ( boneName == "base" )
					{
						//Base bone needs to be manipulated as a special case
						q.x = bulkBoneRotations->bones[ i ].rot_x;
						q.y = -bulkBoneRotations->bones[ i ].rot_y;
						q.z = bulkBoneRotations->bones[ i ].rot_z;
						q.w =  bulkBoneRotations->bones[ i ].rot_w;
					}
					else
					{
						q.x = -bulkBoneRotations->bones[ i ].rot_x;
						q.y = bulkBoneRotations->bones[ i ].rot_y;
						q.z = -bulkBoneRotations->bones[ i ].rot_z;
						q.w =  bulkBoneRotations->bones[ i ].rot_w;
					}

					bone->setOrientation( q );

					//Vector3 v;
					//v.x = bulkBoneData->bones[ i ].
				}
			}
		}


		static void OnBonePositions( const BoneBusServer::BulkBonePositions * bulkBonePositions, void * userData )
		{
			printf( "Set Bone Positions! - %d %d %d\n", bulkBonePositions->packetId, bulkBonePositions->charId, bulkBonePositions->numBonePositions );


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

			Ogre::Skeleton * skel = ent->getSkeleton();
			//Get map of initial bone positions for the character
			std::map<string,Ogre::Vector3> cachedInitialBonePositions =
				app->characterInitBonePosMap[bulkBonePositions->charId];

			int i;
			for ( i = 0; i < bulkBonePositions->numBonePositions; i++ )
			{
				int id = bulkBonePositions->bones[ i ].boneId;

				std::string & boneName = skeleton[ id ];

				if ( boneName == "" )
					continue;

				//char blah[ 1024 ];
				//sprintf( blah, "Bone: %s (%f, %f, %f)\n", boneName.c_str(), bulkBonePositions->bones[ i ].pos_x, bulkBonePositions->bones[ i ].pos_y, bulkBonePositions->bones[ i ].pos_z );
				//OutputDebugString( blah );

				/*
				if ( bulkBonePositions->bones[ i ].pos_x == 0 &&
				bulkBonePositions->bones[ i ].pos_y == 0 && 
				bulkBonePositions->bones[ i ].pos_z == 0 )
				continue;
				*/

				Ogre::Bone * bone = skel->getBone( boneName.c_str() );

				if ( bone )
				{
					bone->setManuallyControlled( true );
					
					//Get the initial bone position for the bone using bone name
					Vector3 initialBonePosition = cachedInitialBonePositions[boneName];
					Vector3 v;

					//if (cachedInitialBonePositions != NULL)
					//{
						//v.x = bulkBonePositions->bones[ i ].pos_x;
						//v.y = -bulkBonePositions->bones[ i ].pos_y;
						//v.z = bulkBonePositions->bones[ i ].pos_z;
					//}
					//else
					//{
						//Add initial bone position to delta
						v.x = initialBonePosition.x + bulkBonePositions->bones[ i ].pos_x;
						v.y = initialBonePosition.y + -bulkBonePositions->bones[ i ].pos_y;
						v.z = initialBonePosition.z + bulkBonePositions->bones[ i ].pos_z;
					//}

					bone->setPosition( v );
				}
			}
		}


		static void tt_client_callback( const char * op, const char * args, void * user_data )
		{
		   OgreViewerApplication * app = (OgreViewerApplication *)user_data;

		   //NILOG( "ActiveMQ message received: '%s %s'\n", op, args );

		   string sOp = op;
		   string sArgs = args;
		   vector< string > splitArgs;
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
