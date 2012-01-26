
#include "vhcl.h"

#include "glwidget.h"

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

    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

    setFocusPolicy(Qt::StrongFocus);

    timer.start(100, this);

    m_msSinceLastFrame = m_StopWatch.GetTime();
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
   return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
   return QSize(1183, 758);
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

   Character* character = FindCharacterFromTreeSelection(current);
   if (character)
      SetSelectedObject(character, character->FindJoint(current->text(0).toStdString()));
}

void GLWidget::sceneTreeItemDoubleClicked(QTreeWidgetItem * item, int column)
{
   if (!item)
      return;
   
   Character* character = FindCharacterFromTreeSelection(item);
   if (character)
   {
      SetSelectedObject(character, character->FindJoint(item->text(0).toStdString()));
      Vector3 pos = character->GetWorldPosition();
      m_Camera.LookAt(QVector3D(pos.x, pos.y, pos.z));
   }
}

Character* GLWidget::FindCharacterFromTreeSelection(QTreeWidgetItem* treeWidget)
{
   QTreeWidgetItem* parent = treeWidget;
   while (parent)
   {
      for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
      {
         if (parent->text(0).toStdString() == m_pScene->m_characters[i].m_name)
            return &m_pScene->m_characters[i];     
      }
      parent = parent->parent();
   }

   return NULL;
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glPushMatrix();
       m_Camera.Draw(); 
       DrawFloor();
       DrawScene();
    glPopMatrix();
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
   glPushMatrix();
   glTranslated(0, 0, 0);
   glRotated(180, 1, 0, 0);
   glBegin(GL_QUADS);  
      glVertex3f(-1, 0, 1);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(-1, 0, -1);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(1, 0, -1);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(1, 0, 1);
      glNormal3f(0, 1, 0);
   glEnd();
   glPopMatrix();
}

void GLWidget::DrawScene()
{
   if (m_GLMode == SELECT)
      StartPicking();

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

   if (m_GLMode == SELECT)
      StopPicking(); 
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

   if (joint == m_SelData.m_pJoint)
   {
      // axis drawing
      // orientation gizmo
      glBegin(GL_LINES);
         // x axis
         glColor3f(255, 0, 0);
         glVertex3f(0, 0, 0);
         glVertex3f(10, 0, 0); 
      glEnd();

      glBegin(GL_LINES);
         // y axis
         glColor3f(0, 255, 0);
         glVertex3f(0, 0, 0);
         glVertex3f(0, 10, 0); 
      glEnd();

      glBegin(GL_LINES);
         // z axis
         glColor3f(0, 0, 255);
         glVertex3f(0, 0, 0);
         glVertex3f(0, 0, 10); 
       glEnd();
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
