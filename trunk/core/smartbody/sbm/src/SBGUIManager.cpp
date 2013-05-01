#include <CEGUI.h>
#include "SBGUIManager.h"
#include <FL/Fl.H>
#include <sb/SBScene.h>
#include "RendererModules/OpenGL/CEGUIOpenGLRenderer.h"

SBGUIManager* SBGUIManager::_singleton = NULL;
SBGUIManager::SBGUIManager()
{
	initialized = false;

}

SBGUIManager::~SBGUIManager()
{

}

void SBGUIManager::update()
{
	CEGUI::System::getSingleton().renderGUI();
}

void SBGUIManager::handleEvent(int eventID)
{	
	if (!initialized) return; 
	CEGUI::MouseButton ceguiButtons[3] = {CEGUI::LeftButton, CEGUI::RightButton, CEGUI::MiddleButton };

	int button = Fl::event_button();
	if (eventID == FL_PUSH) // mouse button push
	{
		//LOG("Button %d is pushed", button);
		CEGUI::System::getSingleton().injectMouseButtonDown(ceguiButtons[button-1]);				
	}
	else if (eventID == FL_RELEASE) // mouse button release
	{
		//LOG("Button %d is relased", button);
		CEGUI::System::getSingleton().injectMouseButtonUp(ceguiButtons[button-1]);		
	}
	else if (eventID == FL_MOVE || eventID == FL_DRAG) // mouse move
	{	
		//if (eventID == FL_MOVE) LOG("Mouse is moved to %d %d", Fl::event_x(), Fl::event_y());
		//else if (eventID == FL_DRAG) LOG("Mouse is dragged to %d %d", Fl::event_x(), Fl::event_y());
		CEGUI::System::getSingleton().injectMousePosition((float)Fl::event_x(), (float)Fl::event_y());
	}
}

void SBGUIManager::init()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();	
	scene->run("from PyCEGUI import *");
	scene->run("from PyCEGUIOpenGLRenderer import *");

	guiRenderer = &CEGUI::OpenGLRenderer::bootstrapSystem();

	// initialize all resources
	CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>
		(CEGUI::System::getSingleton().getResourceProvider());
	std::string mediaPath = scene->getMediaPath();
	rp->setResourceGroupDirectory("schemes", mediaPath+"/cegui/datafiles/schemes/");
	rp->setResourceGroupDirectory("imagesets", mediaPath+"/cegui/datafiles/imagesets/");
	rp->setResourceGroupDirectory("fonts", mediaPath+"/cegui/datafiles/fonts/");
	rp->setResourceGroupDirectory("layouts", mediaPath+"/cegui/datafiles/layouts/");
	rp->setResourceGroupDirectory("looknfeels", mediaPath+"/cegui/datafiles/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts", mediaPath+"/cegui/datafiles/lua_scripts/");

	CEGUI::Imageset::setDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

	
	// create a default window 
	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
	//CEGUI::Scheme& ogreScheme = CEGUI::SchemeManager::getSingleton().create( "OgreTray.scheme" );	
	CEGUI::Scheme& ogreScheme = CEGUI::SchemeManager::getSingleton().create( "TaharezLook.scheme" );	
	//CEGUI::Scheme& ogreScheme = CEGUI::SchemeManager::getSingleton().create( "WindowsLook.scheme" );	
	
	//CEGUI::Imageset& imageSet = CEGUI::ImagesetManager::getSingleton().get("OgreTrayImages");
	//imageSet.setAutoScalingEnabled(false);
	//CEGUI::Font& font = CEGUI::FontManager::getSingleton().get("DejaVuSans-10");
	//font.setAutoScaled(false);
	
	//CEGUI::Font& font = CEGUI::FontManager.getSingleton().get("")
	//CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
	CEGUI::Window *sheet = winMgr.createWindow("DefaultWindow", "SBGUI");
	//sheet->setMinSize(CEGUI::UVector2(CEGUI::UDim(0.0, 1920), CEGUI::UDim(0.0, 1200)));
	CEGUI::System::getSingleton().setGUISheet( sheet );
	initialized = true;
	// create other widgets on screen
	
	//Create the quit button
	/*
	CEGUI::Window *quit = winMgr.createWindow("OgreTray/Button", "CEGUIDemo/QuitButton");
	quit->setText("Quit");
	quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.0, 200), CEGUI::UDim(0.0, 60)));
	sheet->addChildWindow(quit);
	*/
	/*
	CEGUI::Window *button = winMgr.createWindow("OgreTray/StaticText", "CEGUIDemo/Button2");
	button->setText("Button2 Text");	
	button->setProperty("TextColours","FFFF0000");
	button->setProperty("BackgroundEnabled", "false"); 
	button->setProperty("FrameEnabled", "false"); 		
	button->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25, 0), CEGUI::UDim(0.10, 0.0)));
	button->setSize(CEGUI::UVector2(CEGUI::UDim(0.25, 0), CEGUI::UDim(0.10, 0)));
	sheet->addChildWindow(button);
	*/
	//quit->subscribeEvent(CEGUI::PushButton::EventClicked, &testCEGUIButtonPush);
	//LOG("Finish create CEGUI");
	
}

void SBGUIManager::resize( int w, int h )
{
	CEGUI::System::getSingleton().notifyDisplaySizeChanged(CEGUI::Size((float)w,(float)h));
}
