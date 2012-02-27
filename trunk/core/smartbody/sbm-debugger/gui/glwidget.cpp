
#include "vhcl.h"

#include "glwidget.h"
#include "SbmDebuggerForm.h"
#include "vhmsg-tt.h"
#include <math.h>


using std::string;
using std::vector;


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


//! [0]
GLWidget::GLWidget(Scene* scene, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_GLMode = RENDER;
    m_quadric = gluNewQuadric();
    SetScene(scene);
    
    m_fPawnSize = 1.0f;
    m_fJointRadius = 1.25f;
    m_nPickingOffset = 0;
    m_fAxisLength = 5;
    m_fEyeBeamLength = 100;
    m_RenderFlags = 0;

    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

    setFocusPolicy(Qt::StrongFocus);

    timer.start(10, this);

    m_msSinceLastFrame = m_StopWatch.GetTime();

    SetMinimumSize(400, 400);
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
   return m_MinSize;
}

QSize GLWidget::sizeHint() const
{
   return QSize(400, 400);
}

void GLWidget::OnCloseSettingsDialog(const SettingsDialog* dlg, int result)
{
   if (result == QDialog::Accepted)
   {
      m_Camera.SetCameraType(dlg->ui.cameraControlBox->currentText().toStdString());
      m_Camera.SetMovementSpeed(dlg->ui.cameraMovementSpeedBox->value());
      m_Camera.SetRotationSpeed(dlg->ui.cameraRotationSpeedBox->value());
   }
}

void GLWidget::ToggleFreeLook()
{
   m_Camera.SetCameraType(m_Camera.FollowRenderer() ? Camera::Free_Look : Camera::Follow_Renderer);
}

void GLWidget::sceneTreeCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
   if (!current)
      return;

   Pawn* entity = SbmDebuggerForm::FindSbmEntityFromTreeSelection(current, m_pScene);
   if (entity)
      SetSelectedObject(entity, entity->FindJoint(current->text(0).toStdString()));
}

void GLWidget::sceneTreeItemDoubleClicked(QTreeWidgetItem * item, int column)
{
   if (!item)
      return;
   
   Pawn* entity = SbmDebuggerForm::FindSbmEntityFromTreeSelection(item, m_pScene);
   if (entity)
   {
      SetSelectedObject(entity, entity->FindJoint(item->text(0).toStdString()));
      Vector3 pos = entity->GetWorldPosition();
      m_Camera.LookAt(QVector3D(pos.x, pos.y, pos.z));
   }
}

void GLWidget::ToggleShowAxes(bool enabled)
{
   m_RenderFlags ^= Show_Axes;
}

void GLWidget::ToggleShowEyeBeams(bool enabled)
{
   m_RenderFlags ^= Show_EyeBeams;
}

void GLWidget::ToggleAllowBoneUpdates(bool enabled)
{
   
}

void GLWidget::initializeGL()
{
    qglClearColor(qtPurple.dark());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void GLWidget::paintGL()
{
   if (m_GLMode == SELECT)
      StartPicking();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glPushMatrix();
       m_Camera.Draw(); 
       DrawFloor();
       DrawScene();
    glPopMatrix();

    if (m_GLMode == SELECT)
      StopPicking(); 
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    //glViewport((width - side) / 2, (height - side) / 2, side, side);
    glViewport(geometry().x() - 50, geometry().y() - 35, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (double)width / (double)height, 0.1f, 1000);

    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::StartPicking()
{
   GLint viewport[4];
	float ratio;

	glSelectBuffer(SELECT_BUFF_SIZE, selectBuf);

	glGetIntegerv(GL_VIEWPORT, viewport);

	glRenderMode(GL_SELECT);

	glInitNames();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

   //if (!m_pScene->m_rendererIsRightHanded)
   //{
   //   // this fixes the mirroring problem in left handed coordinate systems
   //   QMatrix4x4 mat;
   //   mat.setToIdentity();
   //   mat.setColumn(0, QVector4D(-1, 0, 0, 0));
   //   glMultMatrixd(mat.data());
   //}

	gluPickMatrix(lastPos.x(), viewport[3] - lastPos.y(), 10, 10, viewport);
	ratio = (viewport[2] + 0.0) / viewport[3];
   DebuggerCamera cam = m_pScene->m_camera;
	gluPerspective(cam.fovY, cam.aspect, cam.zNear, cam.zFar);
	glMatrixMode(GL_MODELVIEW);
}

Joint* GLWidget::FindPickedJoint(int pickIndex)
{
   unsigned int characterIndex = pickIndex / PICKING_OFFSET;

   if (characterIndex >= 0 && characterIndex< m_pScene->m_characters.size())
   {
      m_nPickingOffset = pickIndex % PICKING_OFFSET;
      Joint* retVal = FindPickedJointRecursive(m_pScene->m_characters[characterIndex].m_joints[0]);
      SetSelectedObject(&m_pScene->m_characters[characterIndex], retVal);
      emit JointPicked(m_SelData.m_pObj, m_SelData.m_pJoint);
   }
   
   return m_SelData.m_pJoint;
}

Joint* GLWidget::FindPickedJointRecursive(const Joint* joint)
{
   if (m_nPickingOffset-- == 0)
   {
      return const_cast<Joint*>(joint);
   }

   for (unsigned int i = 0; i < joint->m_joints.size(); i++)
   {
      Joint* j = FindPickedJointRecursive(joint->m_joints[i]);
      if (j)
         return j;
   }

   return NULL;
}

QVector3D GLWidget::GetWorldPositionFromScreenCoords(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
 
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
 
    return QVector3D(posX, posY, posZ);
}

void GLWidget::ProcessHits(GLint hits, GLuint buffer[])
{
   GLint i, numberOfNames;
   GLuint names, *ptr, minZ, *ptrNames;

   // sort the picked objects closest to furthest
   ptr = (GLuint *) buffer;
   minZ = 0xffffffff;
   for (i = 0; i < hits; i++)
   {	
     names = *ptr;
	  ptr++;
	  if (*ptr < minZ)
     {
		  numberOfNames = names;
		  minZ = *ptr;
		  ptrNames = ptr+2;
	  }
	  
	  ptr += names+2;
	}

   if (numberOfNames > 0) 
   {
	  // hit
	  ptr = ptrNames;
     while (*ptr == 0/*UINT_MAX*/ || *ptr == -1 || *ptr == 1) // TODO: figure out why these are bad numbers
         ptr++;

     // first check if we are picking the floor
     if (*ptr == FLOOR_LAYER && m_SelData.m_pObj != NULL)
     { 
         QVector3D camPos = m_Camera.GetPosition();
         QVector3D endPoint = GetWorldPositionFromScreenCoords(lastPos.x(), lastPos.y());
         QVector3D intersection = camPos + endPoint;
         //printf("\nintersectionPoint: x: %.2f,   y: %.2f,   z: %.2f", intersection.x(), intersection.y(), intersection.z());

         // draw ray
         glPushMatrix();
            glLoadIdentity();
            glColor3d(1.0, 1.0, 1.0);
            glBegin(GL_LINES);
            glVertex3d(camPos.x(), camPos.y(), camPos.z());
            glVertex3d(endPoint.x(), endPoint.y(), endPoint.z());
            glEnd();
         glPopMatrix();

         std::string msg = vhcl::Format("sbm bml char %s <locomotion target=\"%d %d\" type=\"basic\"/>",
            m_SelData.m_pObj->m_name.c_str(), (int)intersection.x(), (int)intersection.z());

         // currently locomotion is disabled
         //vhmsg::ttu_notify1(msg.c_str());
     }
     else
     {
         m_SelData.m_pJoint = FindPickedJoint(*ptr);
         if (m_SelData.m_pJoint)
         {
            printf("Picked Joint: %s\n", m_SelData.m_pJoint->m_name.c_str());
         }

         for (int j = 0; j < numberOfNames; j++,ptr++) 
         { 
            printf ("%d ", *ptr);
         }
	   }
   }
   else
   {
	   // no hit
   }
   
}

void GLWidget::StopPicking()
{
   glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	GLint hits = glRenderMode(GL_RENDER);
	if (hits != 0)
   {
		ProcessHits(hits, selectBuf);
	}
	m_GLMode = RENDER;
}

void GLWidget::SetSelectedObject(Pawn* obj, Joint* joint)
{
   if (!obj)
   {
      m_SelData.m_pObj = NULL;
      m_SelData.m_pJoint = NULL;
   }
   else
   {
      m_SelData.m_pObj = obj;
      m_SelData.m_pJoint = joint ? joint : m_SelData.m_pObj->GetWorldOffset();
   }
}

void GLWidget::DrawCylinder(float baseRadius, float topRadius, float height, int slices, int stacks)
{
   gluCylinder(m_quadric, baseRadius, topRadius, height, slices, stacks);
}

void GLWidget::DrawSphere(double radius, int slices, int stacks)
{
   gluSphere(m_quadric, radius, slices, stacks);
}

void GLWidget::DrawFloor()
{
   const float Size = 1000;
   const float LineHeightOffset = 1.0;
   glPushMatrix();
   glPushName(FLOOR_LAYER);

   // draw axis
   glColor3f(0.1f, 0.1f, 0.1f);
   glBegin(GL_LINES);
      glVertex3f(-Size, LineHeightOffset, 0); // x
      glVertex3f(Size, LineHeightOffset, 0);

      glVertex3f(0, LineHeightOffset, -Size); // z
      glVertex3f(0, LineHeightOffset, Size);
   glEnd();

   // draw floor
   glColor3f(0.5f, 0.5f, 0.5f);
   glTranslated(0, 0, 0);
   glRotated(180, 1, 0, 0);
   glBegin(GL_QUADS);  
      glVertex3f(-Size, 0, Size);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(-Size, 0, -Size);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(Size, 0, -Size);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(Size, 0, Size);
      glNormal3f(0, 1, 0);
   glPopName();
   glEnd();
   glPopMatrix();
}

void GLWidget::DrawScene()
{
   // draw characters
   vector<Character> characters = m_pScene->m_characters;
   for (unsigned int i = 0; i < characters.size(); i++)
   {
      glPushName(i);
      glPushMatrix();
      m_nPickingOffset = i* PICKING_OFFSET;
      DrawCharacter(&characters[i]);
      glPopMatrix();
      glPopName();
   }
   
   // draw pawns
   vector<Pawn> pawns = m_pScene->m_pawns;
   for (unsigned int i = 0; i < pawns.size(); i++)
   {
      glPushMatrix();
      DrawPawn(&pawns[i]);
      glPopMatrix();
   }
}

QMatrix4x4 GetLocalRotation(Joint* joint)
{
   QQuaternion locationRotation = QQuaternion(joint->rotOrig.w, joint->rotOrig.x, joint->rotOrig.y, joint->rotOrig.z) 
      * QQuaternion(joint->rot.w, joint->rot.x, joint->rot.y, joint->rot.z);

   QMatrix4x4 mat;
   mat.setToIdentity();
   mat.rotate(locationRotation);
   return mat;
}

void GLWidget::DrawCharacter(const Character* character)
{
   for (unsigned int i = 0; i < character->m_joints.size(); i++)
   {
      DrawJoint(character->m_joints[i]);
   }
}

void GLWidget::DrawJoint(Joint* joint)
{
   glPushMatrix();
            
   // calulate position and rotation
   Vector3 jointPos = (joint->posOrig + joint->pos);
   glTranslated(jointPos.x, jointPos.y, jointPos.z);
   QMatrix4x4 rotationMat = GetLocalRotation(joint);
   glMultMatrixd(rotationMat.data());  
 
   glPushName(m_nPickingOffset++);  // for picking
   glColor3f(0.5f, 0.5f, 0.5f);
   DrawSphere(m_fJointRadius);  
   glPopName();

   if (joint == m_SelData.m_pJoint
      || (m_RenderFlags & Show_Axes))
   {
      // axis drawing
      // orientation gizmo
      glBegin(GL_LINES);
         // x axis
         glColor3f(255, 0, 0);
         glVertex3f(0, 0, 0);
         glVertex3f(m_fAxisLength, 0, 0); 
      glEnd();

      glBegin(GL_LINES);
         // y axis
         glColor3f(0, 255, 0);
         glVertex3f(0, 0, 0);
         glVertex3f(0, m_fAxisLength, 0); 
      glEnd();

      glBegin(GL_LINES);
         // z axis
         glColor3f(0, 0, 255);
         glVertex3f(0, 0, 0);
         glVertex3f(0, 0, m_fAxisLength); 
       glEnd();
   }

   if (m_RenderFlags & Show_EyeBeams)
   {
      if (joint->m_name.find("eyeball") != string::npos)
      {
         glBegin(GL_LINES);
            // z axis
            glColor3f(255, 0, 0);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, m_fEyeBeamLength); 
         glEnd();
      }
   }
        
   // draw a connecting bone between the 2 joints
   if (joint->m_parent)
   {
      glPushMatrix();
      glPushName(UINT_MAX);
      /*Vector3 parentJointPos = (joint->m_parent->posOrig + joint->m_parent->pos);
      double jointLength = (jointPos - parentJointPos).Magnitude();
      // draw the bone
      DrawCylinder(0.01f, 0.01f, jointLength, 10, 10);*/

      rotationMat = rotationMat.transposed();
      glMultMatrixd(rotationMat.data());
      glBegin(GL_LINES);
         glColor3f(0, 0, 0);
         glVertex3f(0, 0, 0);
         glVertex3f(-jointPos.x, -jointPos.y, -jointPos.z);  
      glEnd();

      glPopName();
      glPopMatrix();
   } 

   for (unsigned int i = 0; i < joint->m_joints.size(); i++)
   {   
      DrawJoint(joint->m_joints[i]);
   }
   glPopMatrix();
}

void GLWidget::DrawPawn(const Pawn* pawn)
{
   Vector3 pos = pawn->GetWorldPosition();
   glTranslated(pos.x, pos.y, pos.z);
   
   QMatrix4x4 rotationMat = GetLocalRotation(pawn->m_joints[0]);
   glMultMatrixd(rotationMat.data());  

   DrawSphere(m_fPawnSize);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();

    /*int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    float winZ = 0;
    glReadPixels( lastPos.x(), viewport[3] - lastPos.y(), 1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    GLenum error = glGetError();*/

    m_GLMode = SELECT;
}

//void GLWidget::mouseReleaseEvent(QMouseEvent *event)
//{
//   //lastPos.setX(0); lastPos.setY(0);
//}

void GLWidget::wheelEvent(QWheelEvent *event)
{
   if (event->delta() > 0)
   {
      m_Camera.MoveZ(-m_Camera.GetMovementSpeed());
   }
   else
   { 
      m_Camera.MoveZ(m_Camera.GetMovementSpeed());
   }
   updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) 
    {
       m_Camera.Rotate(QVector3D(m_Camera.GetRotationSpeed() * dy, 0, 0));
       m_Camera.Rotate(QVector3D(0, (m_Camera.GetRotationSpeed() * dx), 0));
    } 
    lastPos = event->pos();
}

void GLWidget::keyPressEvent(QKeyEvent *key)
{
   if (key->key() == Qt::Key_A // left
      || key->key() == Qt::Key_Left) 
   {
      m_Camera.MoveX(-m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_D // right
      || key->key() == Qt::Key_Right) 
   {
      m_Camera.MoveX(m_Camera.GetMovementSpeed());
   }
   if (key->key() == Qt::Key_W // forward
      || key->key() == Qt::Key_Up)  
   {
      m_Camera.MoveZ(-m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_S // back
      || key->key() == Qt::Key_Down) 
   {
      m_Camera.MoveZ(m_Camera.GetMovementSpeed());
   }
   if (key->key() == Qt::Key_Q) // up
   {
      m_Camera.MoveY(m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_E) // down
   {
      m_Camera.MoveY(-m_Camera.GetMovementSpeed());
   }

   updateGL();
}

void GLWidget::timerEvent(QTimerEvent * event)
{
   if (event->timerId() == timer.timerId())
   {
      Update();
   }
   else
   {
      QWidget::timerEvent(event);
   }
}

void GLWidget::Update()
{
   if (m_Camera.FollowRenderer())
   {
      DebuggerCamera cam = m_pScene->m_camera;
      m_Camera.SetRightHanded(m_pScene->m_rendererIsRightHanded);
     
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      if (!m_pScene->m_rendererIsRightHanded)
      {
         // this fixes the mirroring problem in left handed coordinate systems
         QMatrix4x4 mat;
         mat.setToIdentity();
         mat.setColumn(0, QVector4D(-1, 0, 0, 0));
         glMultMatrixd(mat.data());
      }
      
      gluPerspective(cam.fovY, cam.aspect, cam.zNear, cam.zFar);
      glMatrixMode(GL_MODELVIEW);

      m_Camera.SetPosition(QVector3D(cam.pos.x * m_Camera.CoordConverter(), cam.pos.y, cam.pos.z));
      m_Camera.SetRotation(QQuaternion(cam.rot.w * m_Camera.CoordConverter(), cam.rot.x, cam.rot.y, cam.rot.z));
   }

   // calculate fps
   m_msSinceLastFramePrev = m_msSinceLastFrame;
   m_msSinceLastFrame = m_StopWatch.GetTime();
   updateGL();
}

string GLWidget::GetCameraPositionAsString()
{
  QVector3D pos = m_Camera.GetPosition();
  return vhcl::Format("Camera Position:\nx: %.2f   y: %.2f   z: %.2f", pos.x(), pos.y(), pos.z());
}

double GLWidget::GetFps()
{
   return (1 / (m_msSinceLastFrame - m_msSinceLastFramePrev)); 
}

string GLWidget::GetFpsAsString()
{
   return vhcl::Format("Renderer Fps: %.2f", GetFps());
}
