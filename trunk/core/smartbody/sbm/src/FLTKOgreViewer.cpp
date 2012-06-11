#include <sbm/mcontrol_util.h>
#include "FLTKListener.h"

#ifdef INTMAX_C 
#undef INTMAX_C
#endif
#ifdef UINTMAX_C
#undef UINTMAX_C
#endif

#include "EmbeddedOgre.h"
#include "FLTKOgreViewer.h"
#include "SBOgreListener.h"
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <FL/x.H>
#include <sr/sr_gl.h>


FLTKOgreWindow::FLTKOgreWindow( int x, int y, int w, int h, const char *label/*=0 */ ) : FltkViewer(x,y,w,h,label)//Fl_Gl_Window(x,y,w,h,label), SrViewer(x, y, w, h)
{
	ogreInterface = NULL;
}

FLTKOgreWindow::~FLTKOgreWindow()
{

}

void FLTKOgreWindow::show_viewer()
{
	show();
	if (!ogreInterface)
		initOgreWindow();


}

void FLTKOgreWindow::hide_viewer()
{

}

void FLTKOgreWindow::initOgreWindow()
{
	//fl_open_display();
	//GLChoiceType* glChoice = GLChoiceType::find( CAP_DEPTH_BUFFER | CAP_DOUBLE_BUFFER, NULL );
	//Fl_X::make_xid( this, glChoice->vis, glChoice->colormap );	
	void* flHwnd = (void*)fl_xid(this); // get hwnd	
	void* flParentHwnd = NULL;
	if (parent())
		flParentHwnd = (void*)fl_xid(dynamic_cast<Fl_Window*>(parent()));
	make_current();
	//printf("ogreWindow, GLContext = %d\n",context());
	ogreInterface = new EmbeddedOgre();
	ogreInterface->createOgreWindow(flHwnd, flParentHwnd, w(), h(), "OgreWindow");	

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	FLTKListener* fltkListener = dynamic_cast<FLTKListener*>(mcu.sbm_character_listener);
	OgreListener* ogreListener = NULL;
	if (fltkListener)
	{
		ogreListener = new OgreListener(ogreInterface);
		fltkListener->setOtherListener(ogreListener);
	}
	if (ogreListener)
	{
		SBScene* sbScene = SBScene::getScene();
		std::vector<std::string> charNames = sbScene->getCharacterNames();
		for (unsigned int i=0;i<charNames.size();i++)
		{
			ogreListener->OnCharacterChangeMesh(charNames[i]);
		}
	}	
	//fl_set_gl_context(this,ogreInterface->getGLContext());	
}

void FLTKOgreWindow::draw()
{	
	//fl_set_gl_context(this,ogreInterface->getGLContext());
	make_current();
	if (ogreInterface)
	{
 		//glPushAttrib( GL_ALL_ATTRIB_BITS );
 		glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
 		glMatrixMode( GL_COLOR );
		glPushMatrix();
		glMatrixMode( GL_TEXTURE );
		glPushMatrix();
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();		
		updateOgreCamera();
		ogreInterface->update(); // do Ogre rendering for deformable characters
		// pop everything!
		glMatrixMode( GL_COLOR );
		glPopMatrix();
		glMatrixMode( GL_TEXTURE );
		glPopMatrix();
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
 		glPopClientAttrib();
 		//glPopAttrib();
 		
 		fltkRender(); // let old fltk viewer render rest of stuffs
 		ogreInterface->finishRender();
	}
	Ogre::WindowEventUtilities::messagePump();
}


void FLTKOgreWindow::updateOgreCamera()
{
	Ogre::Camera* ogreCam = ogreInterface->getCamera();
	SrCamera& cam = FltkViewer::_data->camera;
	// override the ogre camera with fltk camera
	SrMat viewMat;
	cam.get_view_mat(viewMat);	
	SrQuat q = SrQuat(viewMat);
	SrVec  p = cam.eye;
	ogreCam->setOrientation(Ogre::Quaternion(q.w,q.x,q.y,q.z).Inverse());
	ogreCam->setPosition(Ogre::Vector3(p.x,p.y,p.z));			
	//ogreCam->setOrientation()	
	//ogreCam->setFarClipDistance(cam.zfar);
	//ogreCam->setNearClipDistance(cam.znear);
	cam.zfar = ogreCam->getFarClipDistance();
	cam.znear = ogreCam->getNearClipDistance();
	ogreCam->setFOVy(Ogre::Radian(cam.fovy));
}


void FLTKOgreWindow::fltkRender()
{
	//FltkViewer::draw();
	glEnable(GL_DEPTH_TEST);
	if (_objManipulator.hasPicking())
	{
		SrVec2 pick_loc = _objManipulator.getPickLoc();
		_objManipulator.picking(pick_loc.x,pick_loc.y, _data->camera);	   
	}  

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	glViewport ( 0, 0, w(), h() );
	SrLight &light = _data->light;
	SrCamera &cam  = _data->camera;
	SrMat mat ( SrMat::NotInitialized );

	//----- Set Projection ----------------------------------------------
	cam.aspect = (float)w()/(float)h();

	glMatrixMode ( GL_PROJECTION );
	glLoadMatrix ( cam.get_perspective_mat(mat) );

	//----- Set Visualisation -------------------------------------------
	glMatrixMode ( GL_MODELVIEW );
	glLoadMatrix ( cam.get_view_mat(mat) );

	glScalef ( cam.scale, cam.scale, cam.scale );

	updateLights();
	glEnable ( GL_LIGHTING );
	for (size_t x = 0; x < _lights.size(); x++)
	{
		glLight ( x, _lights[x] );		
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
	glEnable( GL_COLOR_MATERIAL );
	glEnable( GL_NORMALIZE );

	if (_data->terrainMode == FltkViewer::ModeTerrain)
		mcu.render_terrain(0);
	else if (_data->terrainMode == FltkViewer::ModeTerrainWireframe)
		mcu.render_terrain(1);

	glDisable( GL_COLOR_MATERIAL );
	

	//drawAllGeometries();		
 	if( _data->root )	{		
 		_data->render_action.apply ( _data->root );
 	}
	drawPawns();
	// draw the grid
	//   if (gridList == -1)
	//	   initGridList();

	drawGrid();

	drawSteeringInfo();
	drawEyeBeams();
	drawEyeLids();
	drawDynamics();
	drawLocomotion();
	if (_data->showcollisiongeometry)
		drawCharacterPhysicsObjs();
	drawInteractiveLocomotion();

	//_posControl.Draw();
	_objManipulator.draw(cam);
	// feng : debugging draw for reach controller
	drawReach();


}

void FLTKOgreWindow::render()
{
	redraw(); 
}

void FLTKOgreWindow::resize( int x, int y, int w, int h )
{
	make_current();
	Fl_Gl_Window::resize(x,y,w,h);
	Ogre::RenderWindow* win = ogreInterface->getRenderWindow();
	if (win) 
	{
		win->resize(w,h);		
		win->windowMovedOrResized();
	}
	redraw();
}

void FLTKOgreWindow::menu_cmd( MenuCmd c, const char* label )
{
	// override the GPU deformable model to let Ogre3D do the rendering
	if (c == CmdCharacterShowDeformableGeometryGPU)
	{
		SbmDeformableMeshGPU::disableRendering = true;
		_data->charactermode = ModeShowDeformableGeometryGPU;				
		_data->showgeometry = false;
		_data->showcollisiongeometry = false;
		_data->showdeformablegeometry = false;
		_data->showbones = false;
		_data->showaxis = false;
		ogreInterface->setCharacterVisibility(true);
		applyToCharacters();
	}	
	else
	{
		if (c ==  CmdCharacterShowGeometry || c == 	CmdCharacterShowCollisionGeometry || c == CmdCharacterShowDeformableGeometry
			|| c == CmdCharacterShowBones || c == CmdCharacterShowAxis)
		{
			SbmDeformableMeshGPU::disableRendering = false;
			ogreInterface->setCharacterVisibility(false);
		}
		FltkViewer::menu_cmd(c,label);
	}
	//FltkViewer::menu_cmd(c,label);
}
// Ogre viewer factory
SrViewer* OgreViewerFactory::s_viewer = NULL;
OgreViewerFactory::OgreViewerFactory()
{
	s_viewer = NULL;
}
SrViewer* OgreViewerFactory::create(int x, int y, int w, int h)
{
	if (!s_viewer)
		s_viewer = new FLTKOgreWindow(x, y, w, h, "SmartBodyOgre");
	return s_viewer;
}
void OgreViewerFactory::remove(SrViewer* viewer)
{
	if (viewer && (viewer == s_viewer))
	{
		viewer->hide_viewer();
	}
}
