#include <fltk/gl.h>
#include <GL/glu.h>
#include <sr/sr_plane.h>
#include <sr/sr_sphere.h>
#include <sr/sr_sn.h>
#include <sr/sr_sn_group.h>
# include <SR/sr_sa_gl_render.h>
# include <SR/sr_gl_render_funcs.h>
#include "PositionControl.h"

PositionControl::PositionControl(void)
{
	dragging = false;
	active = false;
	base=80;
	r=6;
	len=r*4;
	s_len=10;
	ss_len=3;
	opdir = 3;

	colors[3]=SrVec(100.0/255.0, 220.0/255.0, 1.);
	colors[0]=SrVec(1,0,0);
	colors[1]=SrVec(0,154.0/255.0,82.0/255.0);
	colors[2]=SrVec(0,0,1);
	worldPt = SrVec(0,170,100);
}

PositionControl::~PositionControl(void)
{
}

void PositionControl::setColor(const SrVec &color)
{
	glDisable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glColor3fv(color);
	glEnable(GL_LIGHTING);
}

SrVec PositionControl::getWorldPt()
{
	return worldPt;
}

void PositionControl::setWorldPt(SrVec& newPt)
{
	worldPt = newPt;	
}

void PositionControl::drawCenter()
{	
	SrVec center= getWorldPt();
	drawSphere(center);
}

void PositionControl::drawSphere(SrVec& pos, float fRadius)
{
	glColor3f(1.0, 0.0, 0.0);
	SrSnSphere sphere;
	sphere.shape().center = pos;//SrPnt(0, 0, 0);
	sphere.shape().radius = fRadius;
	sphere.render_mode(srRenderModeLines);
	SrGlRenderFuncs::render_sphere(&sphere);
}

void PositionControl::hitTest()
{
	glDisable(GL_LIGHTING);
	glPushName(0xffffffff);

	glLoadName(0);
	drawCenter();	
	glPopName();	
	glEnable(GL_LIGHTING);
}

void PositionControl::hitOPS()
{
	glDisable(GL_LIGHTING);
	glPushName(0xffffffff);

	glLoadName(3);

	//draw center square
	SrVec center= getWorldPt();

	SrVec dirx,diry;
	//ScreenParallelPlane(viewer,center,dirx,diry);
	float ratio=0.3;//norm(dirx);
	/*
	glBegin(GL_QUADS);
	glVertex3fv(center-s_len*dirx-s_len*diry);
	glVertex3fv(center+s_len*dirx-s_len*diry);
	glVertex3fv(center+s_len*dirx+s_len*diry);
	glVertex3fv(center-s_len*dirx+s_len*diry);
	glEnd();
	*/
	drawCenter();	

	/*
	glPushMatrix();
	glTranslatef(center[0],center[1],center[2]);

	glLoadName(0);
	glPushMatrix();
	glTranslatef(base*ratio,0,0);
	glRotatef(90,0,1,0);
	GLUquadricObj *arx=gluNewQuadric();
	gluCylinder(arx,r*ratio,0,len*ratio,10,10);
	gluDeleteQuadric(arx);
	glPopMatrix();

	glLoadName(1);
	glPushMatrix();
	glTranslatef(0,base*ratio,0);
	glRotatef(-90,1,0,0);
	GLUquadricObj *ary=gluNewQuadric();
	gluCylinder(ary,r*ratio,0,len*ratio,10,10);
	gluDeleteQuadric(ary);
	glPopMatrix();

	glLoadName(2);
	glPushMatrix();
	glTranslatef(0,0,base*ratio);
	GLUquadricObj *arz=gluNewQuadric();
	gluCylinder(arz,r*ratio,0,len*ratio,10,10);
	gluDeleteQuadric(arz);
	glPopMatrix();

	glPopMatrix();
	*/

	glPopName();
}

void PositionControl::draw()
{
	//if (active)
	{
		SrVec center= getWorldPt();
		SrVec dirx,diry;
		//ScreenParallelPlane(viewer,center,dirx,diry);
		float ratio=0.1;//norm(dirx);

		//Vec3f dirx0,diry0;
		//ScreenParallelPlane(viewer,p_center0,dirx0,diry0);
		//float ratio0=norm(dirx0);

		drawCenter();


		//draw center square
		glDisable(GL_LIGHTING);
		glColor3fv(colors[3]);
		//drawShadowSquare(center[0],center[1],center[2],dirx,diry,s_len,GL_LINE_LOOP);

		if (!dragging){
			glPushMatrix();
			glTranslatef(center[0],center[1],center[2]);

			//draw axis
			glDisable(GL_LIGHTING);
			glBegin(GL_LINES);
			glColor3fv(colors[0]);
			glVertex3f(0,0,0);
			glVertex3f(base*ratio,0,0);
			glColor3fv(colors[1]);
			glVertex3f(0,0,0);
			glVertex3f(0,base*ratio,0);
			glColor3fv(colors[2]);
			glVertex3f(0,0,0);
			glVertex3f(0,0,base*ratio);
			glEnd();


			//draw arrow
			glEnable(GL_LIGHTING);
			setColor(colors[0]);
			//glColor3fv(colors[0]);
			glPushMatrix();
			glTranslatef(base*ratio,0,0);
			glRotatef(90,0,1,0);
			GLUquadricObj *arx=gluNewQuadric();
			gluCylinder(arx,r*ratio,0,len*ratio,10,10);
			gluDeleteQuadric(arx);
			glPopMatrix();

			setColor(colors[1]);
			//glColor3fv(colors[1]);
			glPushMatrix();
			glTranslatef(0,base*ratio,0);
			glRotatef(-90,1,0,0);
			GLUquadricObj *ary=gluNewQuadric();
			gluCylinder(ary,r*ratio,0,len*ratio,10,10);
			gluDeleteQuadric(ary);
			glPopMatrix();

			setColor(colors[2]);
			//glColor3fv(colors[2]);
			glPushMatrix();
			glTranslatef(0,0,base*ratio);
			GLUquadricObj *arz=gluNewQuadric();
			gluCylinder(arz,r*ratio,0,len*ratio,10,10);
			gluDeleteQuadric(arz);
			glPopMatrix();

			glPopMatrix();
		
		}else{
			//draw the tail shadow square
			glDisable(GL_LIGHTING);
			glColor3f(.3,.3,.3);
			//drawShadowSquare(p_center0[0],p_center0[1],p_center0[2],dirx,diry,ss_len,GL_QUADS);

			//draw the moving axis
			glPushMatrix();
			glTranslatef(center[0],center[1],center[2]);
			switch(opdir)
			{
			case 0:
				{
					//line
					glDisable(GL_LIGHTING);
					glBegin(GL_LINES);
					glColor3fv(colors[0]);
					glVertex3f(0,0,0);
					glVertex3f(base*ratio,0,0);
					glEnd();
					//head
					glEnable(GL_LIGHTING);
					setColor(colors[0]);
					glPushMatrix();
					glTranslated(base*ratio,0,0);
					glRotatef(90,0,1,0);
					GLUquadricObj *arx=gluNewQuadric();
					gluCylinder(arx,r*ratio,0,len*ratio,10,10);
					gluDeleteQuadric(arx);
					glPopMatrix();
					break;
				}
			case 1:
				{
					//lines
					glDisable(GL_LIGHTING);
					glBegin(GL_LINES);
					glColor3fv(colors[1]);
					glVertex3f(0,0,0);
					glVertex3f(0,base*ratio,0);
					glEnd();
					//head
					glEnable(GL_LIGHTING);
					setColor(colors[1]);
					glPushMatrix();
					glTranslatef(0,base*ratio,0);
					glRotated(-90,1,0,0);
					GLUquadricObj *ary=gluNewQuadric();
					gluCylinder(ary,r*ratio,0,len*ratio,10,10);
					gluDeleteQuadric(ary);
					glPopMatrix();
					break;
				}
			case 2:
				{
					//line
					glDisable(GL_LIGHTING);
					glBegin(GL_LINES);
					glColor3fv(colors[2]);
					glVertex3f(0,0,0);
					glVertex3f(0,0,base*ratio);
					glEnd();
					//head
					glEnable(GL_LIGHTING);
					setColor(colors[2]);
					glPushMatrix();
					glTranslated(0,0,base*ratio);
					GLUquadricObj *arz=gluNewQuadric();
					gluCylinder(arz,r*ratio,0,len*ratio,10,10);
					gluDeleteQuadric(arz);
					glPopMatrix();
					break;
				}
			case 3:
				{
					//line
					glDisable(GL_LIGHTING);
					glBegin(GL_LINES);
					glColor3fv(colors[0]);
					glVertex3f(0,0,0);
					glVertex3f(base*ratio,0,0);
					glColor3fv(colors[1]);
					glVertex3f(0,0,0);
					glVertex3f(0,base*ratio,0);
					glColor3fv(colors[2]);
					glVertex3f(0,0,0);
					glVertex3f(0,0,base*ratio);
					glEnd();
					break;
				}
			}
			glPopMatrix();
			//draw shadow
			/*
			glPushMatrix();
			glTranslatef(p_center0[0],p_center0[1],p_center0[2]);
			glDisable(GL_LIGHTING);
			glColor3f(0.3,0.3,0.3);
			switch(opdir)
			{
			case 3:
				glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(base*ratio0,0,0);
				glVertex3f(0,0,0);
				glVertex3f(0,base*ratio0,0);
				glVertex3f(0,0,0);
				glVertex3f(0,0,base*ratio0);
				glEnd();
				//drawShadowSquare(base*ratio0,0,0,dirx,diry,ss_len,GL_QUADS);
				//drawShadowSquare(0,base*ratio0,0,dirx,diry,ss_len,GL_QUADS);
				//drawShadowSquare(0,0,base*ratio0,dirx,diry,ss_len,GL_QUADS);

				break;
			case 0:
				glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(base*ratio0,0,0);
				glEnd();
				//drawShadowSquare(base*ratio0,0,0,dirx,diry,ss_len,GL_QUADS);
				break;
			case 1:
				glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(0,base*ratio0,0);
				glEnd();
				//drawShadowSquare(0,base*ratio0,0,dirx,diry,ss_len,GL_QUADS);
				break;
			case 2:
				glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(0,0,base*ratio0);
				glEnd();
				//drawShadowSquare(0,0,base*ratio0,dirx,diry,ss_len,GL_QUADS);
				break;
			}
			glPopMatrix();
			*/
		}
		
	}
}

bool PositionControl::drag(SrCamera& cam,  float fx, float fy, float tx, float ty )
{
    SrVec p1, p2, x, inc;
	SrVec center = getWorldPt();
	//sr_out << "old center = " << center << "  ";
	
	SrVec eye = cam.eye;
	
	SrPlane plane ( center, eye-cam.center );

    cam.get_ray ( fx, fy, p1, x );
    p1 = plane.intersect ( p1, x );
    cam.get_ray ( tx, ty, p2, x );
    p2 = plane.intersect ( p2, x );

    inc = p2-p1;
	//sr_out << "inc = " << inc << "  ";
	//sr_out << "new center = " << center << srnl;
	center += inc;

	setWorldPt(center);
	return true;
}
