/*
 *  sr_gl_render_funcs.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmarBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently UC Merced)
 */

# include <sr/sr_vec.h>
# include <sr/sr_mat.h>
# include <sr/sr_model.h>
# include <sr/sr_lines.h>
# include <sr/sr_points.h>
# include <sr/sr_box.h>
# include <sr/sr_sphere.h>
# include <sr/sr_cylinder.h>
# include <sr/sr_polygons.h>
#include <sbm/GPU/SbmTexture.h>
#include <sbm/GPU/SbmShader.h>

# include <sr/sr_sn.h>
# include <sr/sr_sn_shape.h>
# include <sr/sr_gl_render_funcs.h>

# include <sr/sr_gl.h>


//=============================== render_model ====================================

void SrGlRenderFuncs::render_model ( SrSnShapeBase* shape )
 {	
   SrModel& model = ((SrSnModel*)shape)->shape();

   //SR_TRACE1 ( "Render Model faces="<<model.F.size() );
   //initTex();

   SrArray<SrModel::Face>& F = model.F;
   SrArray<SrModel::Face>& Fn = model.Fn;
   SrArray<SrModel::Face>& Ft = model.Ft;
   SrArray<int>&           Fm = model.Fm;
   SrArray<SrVec>&         V = model.V;
   SrArray<SrVec>&         N = model.N;
   SrArray<SrPnt2>&        T = model.T;
   SrArray<SrMaterial>&    M = model.M;

   int fsize = F.size();
   int fmsize = Fm.size();
   if ( fsize==0 ) return;

   if ( shape->material_is_overriden() ) fmsize=0; // model materials are ignored
  
//   glEnable ( GL_LIGHTING );
   glShadeModel ( GL_SMOOTH );

   if ( model.culling )
    glEnable ( GL_CULL_FACE );
   else
    glDisable ( GL_CULL_FACE );

   bool flat = true;   
   SrVec fn(SrVec::i);
   int fmIndex = Fm.size() > 0? Fm[0] : -1;
   SrMaterial curmtl = fmsize>0 && fmIndex >= 0? M[Fm[0]]:shape->material();
   switch ( shape->render_mode() )
    { case srRenderModeDefault :
      case srRenderModeSmooth :
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           flat = false;
           break;
      case srRenderModeFlat :
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           break;
      case srRenderModeLines :
           glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
           break;
      case srRenderModePoints :
           glPolygonMode ( GL_FRONT_AND_BACK, GL_POINT );
           break;
    }
   
   glMaterial ( curmtl );    
   if (model.mtlnames.size() == 0)
   {
	   glBegin ( GL_TRIANGLES ); // some cards do require begin/end for each triangle!
	   for (int k=0; k<F.size(); k++ )
	   {	
		   int f = k;
		   glNormal ( N[Fn[f].a] ); glVertex ( V[F[f].a] );
		   glNormal ( N[Fn[f].b] ); glVertex ( V[F[f].b] );
		   glNormal ( N[Fn[f].c] ); glVertex ( V[F[f].c] );		   
	   }
	   glEnd ();
   }
   else
   {
	   for (int i=0;i<model.mtlnames.size();i++)
	   {
		   std::string mtlName = model.mtlnames[i];
		   if (model.mtlFaceIndices.find(mtlName) == model.mtlFaceIndices.end())
			   continue;
		   std::vector<int>& mtlFaces = model.mtlFaceIndices[mtlName];	   
		   std::string texName = "none";
		   if (model.mtlTextureNameMap.find(mtlName) != model.mtlTextureNameMap.end())
			   texName = model.mtlTextureNameMap[mtlName];

		   SbmTexture* tex = SbmTextureManager::singleton().findTexture(SbmTextureManager::TEXTURE_DIFFUSE,texName.c_str());
		   if ( fsize > Fn.size() || flat ) // no normal
		   {
			   glBegin ( GL_TRIANGLES ); // some cards do require begin/end for each triangle!
			   for (unsigned int k=0; k<mtlFaces.size(); k++ )
			   {  
				   int f = mtlFaces[k];			   
				   fn = model.face_normal(f);
				   glNormal ( fn );		   
				   glVertex ( V[F[f].a] );
				   glVertex ( V[F[f].b] );
				   glVertex ( V[F[f].c] );		   
			   }
			   glEnd ();
		   }
		   else if ( fsize > Ft.size() || !tex ) // no texture
		   {
			   glBegin ( GL_TRIANGLES );
			   for (unsigned int k=0; k<mtlFaces.size(); k++ )
			   {	
				   int f = mtlFaces[k];
				   glNormal ( N[Fn[f].a] ); glVertex ( V[F[f].a] );
				   glNormal ( N[Fn[f].b] ); glVertex ( V[F[f].b] );
				   glNormal ( N[Fn[f].c] ); glVertex ( V[F[f].c] );		   
			   }
			   glEnd (); 
		   }
		   else // has normal and texture
		   {   // to-do : figure out why texture does not work in the fixed-pipeline ?	  
			   //glDisable(GL_LIGHTING);	
			   
			   glEnable ( GL_ALPHA_TEST );
			   glEnable (GL_BLEND);
			   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			   //glAlphaFunc ( GL_GREATER, 0.3f ) ;
			   
			   glDisable(GL_COLOR_MATERIAL);	   
			   glActiveTexture(GL_TEXTURE0);
			   glEnable(GL_TEXTURE_2D);	 	
			   glBindTexture(GL_TEXTURE_2D,tex->getID());	   	   
			   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
			   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
			   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);//_MIPMAP_LINEAR);
			   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
			   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	  
			   glBegin ( GL_TRIANGLES );
			   //glColor3f(1.f,0.f,1.f);
			   for (unsigned int k=0; k<mtlFaces.size(); k++ )
			   {	
				   int f = mtlFaces[k];
				   //printf("texture coord = %f %f\n",T[Ft[f].a].x, T[Ft[f].a].y);
				   glNormal ( N[Fn[f].a] ); 
				   //glColor3f(T[Ft[f].a].x, T[Ft[f].a].y, 0.f);
				   glTexCoord2f(T[Ft[f].a].x, T[Ft[f].a].y); glVertex ( V[F[f].a] ); 
				   glNormal ( N[Fn[f].b] ); 
				   //glColor3f(T[Ft[f].b].x, T[Ft[f].b].y, 0.f);
				   glTexCoord2f(T[Ft[f].b].x, T[Ft[f].b].y); glVertex ( V[F[f].b] ); 
				   glNormal ( N[Fn[f].c] ); 
				   //glColor3f(T[Ft[f].b].x, T[Ft[f].b].y, 0.f);
				   glTexCoord2f(T[Ft[f].c].x, T[Ft[f].c].y); glVertex ( V[F[f].c] ); 		   
			   }
			   glEnd (); 	   
			   glBindTexture(GL_TEXTURE_2D, 0);	   
			   glDisable(GL_BLEND);
		   }

	   }
   }
   
   //if (tex)
	//	printf("texture name = %s, tex id = %d\n",textureName.c_str(), tex->getID());
   

//    for ( f=0; f<fsize; f++ )
//    {    
//       if ( flat || f>=Fn.size() )
//        {
//          fn = model.face_normal(f);
//          glNormal ( fn );
//          glBegin ( GL_TRIANGLES ); // some cards do require begin/end for each triangle!
//          glVertex ( V[F[f].a] );
//          glVertex ( V[F[f].b] );
//          glVertex ( V[F[f].c] );
//          glEnd ();
//        }
//       else
//        {
//          glBegin ( GL_TRIANGLES );
//          glNormal ( N[Fn[f].a] ); glVertex ( V[F[f].a] );
//          glNormal ( N[Fn[f].b] ); glVertex ( V[F[f].b] );
//          glNormal ( N[Fn[f].c] ); glVertex ( V[F[f].c] );
//          glEnd (); 
//        }
//     }

   //SR_TRACE1 ( "End Render Model." );
 }

//============================= render_lines ====================================

void SrGlRenderFuncs::render_lines ( SrSnShapeBase* shape )
 {
   //SR_TRACE1 ( "Render lines" );

   SrLines& l = ((SrSnLines*)shape)->shape();
   SrArray<SrPnt>&   V = l.V;
   SrArray<SrColor>& C = l.C;
   SrArray<int>&     I = l.I;

   if ( V.size()<2 ) return;

   glDisable ( GL_LIGHTING );
   glColor ( shape->color() );

/*
   if ( shape->render_mode()==srRenderModeSmooth )
    { // render cylinders, with resolution as radius?
    }*/

   glLineWidth ( shape->resolution() ); // default is 1.0

   int v=0;               // current V index
   int i;                 // current I index
   int imax = I.size()-1; // max value for i
   int i1=-1, i2;         // pair I[i],I[i+1]

   if ( I.size()>1 ) { i=0; i1=I[i]; i2=I[i+1]; }

   while ( v<V.size() )
   {
	   if ( v==i1 )
	   {if ( i2<0 ) // new color
	   { /*if ( 0 )//shape->render_mode()==srRenderModeSmooth )
		 { SrMaterial mtl = shape->material();
		 mtl.diffuse = C[-i2-1];
		 glMaterial ( mtl );
		 }
		 else*/
		   { glColor ( C[-i2-1] );
		   }
	   }
	   else // new polyline
	   { glBegin ( GL_LINE_STRIP );
	   while ( v<V.size() && v<=i2 ) glVertex(V[v++]);
	   glEnd ();
	   }
	   i+=2; // update next I information
	   if ( i<imax ) { i1=I[i]; i2=I[i+1]; } else i1=-1;
	   }
	   else
	   { glBegin ( GL_LINES );
	   while ( v<V.size() && v!=i1 ) glVertex(V[v++]); 
	   glEnd ();
	   }
   }

   glLineWidth(1.0f);
}

//============================= render_points ====================================

void SrGlRenderFuncs::render_points ( SrSnShapeBase* shape )
{
	//SR_TRACE1 ( "Render points" );

	SrPoints& p = ((SrSnPoints*)shape)->shape();

	SrArray<SrPoints::Atrib>* A = p.A;
	SrArray<SrPnt>& P = p.P;

	if ( P.size()==0 ) return;

	glDisable ( GL_LIGHTING );
	glColor ( shape->material().diffuse );

	if ( shape->render_mode()==srRenderModeSmooth )
	{ // render shperes, with resolution as radius?
	}

	glPointSize ( shape->resolution() ); // default is 1.0

	int i;

	if ( A )
	{
		for ( i=0; i<P.size(); i++ )
		{ if ( i<A->size() )
		{ glPointSize ( A->get(i).s );
		glColor ( A->get(i).c );
		}

		glBegin ( GL_POINTS );
		glVertex ( P[i] );
		glEnd ();
		}
	}
	else
	{
		glBegin ( GL_POINTS );
		for ( i=0; i<P.size(); i++ ) glVertex ( P[i] );
		glEnd ();
	}

	glPointSize(1.0f);
}

//=============================== render_box ====================================

void SrGlRenderFuncs::render_box ( SrSnShapeBase* shape )
 {
   //SR_TRACE1 ( "Render box" );

   SrBox& b = ((SrSnBox*)shape)->shape();
   glColor ( shape->material().diffuse );
   glMaterial ( shape->material() );
   glShadeModel ( GL_SMOOTH );
//   glEnable ( GL_LIGHTING );
   glEnable ( GL_CULL_FACE );

   switch ( shape->render_mode() )
    { case srRenderModeDefault :
      case srRenderModeSmooth :
      case srRenderModeFlat :
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           glDrawBox ( b.a, b.b );
           break;
      case srRenderModeLines :
           glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
           glBegin ( GL_LINE_STRIP );
           glVertex ( b.a.x, b.a.y, b.a.z );
           glVertex ( b.a.x, b.a.y, b.b.z );
           glVertex ( b.a.x, b.b.y, b.b.z );
           glVertex ( b.a.x, b.b.y, b.a.z );
           glVertex ( b.a.x, b.a.y, b.a.z );
           glEnd ();
           glBegin ( GL_LINE_STRIP );
           glVertex ( b.b.x, b.b.y, b.b.z );
           glVertex ( b.b.x, b.a.y, b.b.z );
           glVertex ( b.b.x, b.a.y, b.a.z );
           glVertex ( b.b.x, b.b.y, b.a.z );
           glVertex ( b.b.x, b.b.y, b.b.z );
           glEnd ();
           glBegin ( GL_LINES );
           glVertex ( b.a.x, b.a.y, b.a.z, b.b.x, b.a.y, b.a.z );
           glVertex ( b.a.x, b.a.y, b.b.z, b.b.x, b.a.y, b.b.z );
           glVertex ( b.a.x, b.b.y, b.a.z, b.b.x, b.b.y, b.a.z );
           glVertex ( b.a.x, b.b.y, b.b.z, b.b.x, b.b.y, b.b.z );
           glEnd ();
           break;
      case srRenderModePoints :
           glBegin ( GL_POINTS );
           glVertex ( b.a.x, b.a.y, b.a.z );
           glVertex ( b.a.x, b.a.y, b.b.z );
           glVertex ( b.a.x, b.b.y, b.b.z );
           glVertex ( b.a.x, b.b.y, b.a.z );
           glVertex ( b.a.x, b.a.y, b.a.z );
           glVertex ( b.b.x, b.b.y, b.b.z );
           glVertex ( b.b.x, b.a.y, b.b.z );
           glVertex ( b.b.x, b.a.y, b.a.z );
           glVertex ( b.b.x, b.b.y, b.a.z );
           glVertex ( b.b.x, b.b.y, b.b.z );
           glEnd ();
           break;
    }
 }

//=============================== render_sphere ====================================

void SrGlRenderFuncs::render_sphere ( SrSnShapeBase* shape )
 {
   //SR_TRACE1 ( "Render sphere" );

   SrSphere& sphere = ((SrSnSphere*)shape)->shape();

   srRenderMode rm = shape->render_mode();
   glEnable ( GL_CULL_FACE );
//   glEnable ( GL_LIGHTING );
   glMaterial ( shape->material() );

   switch ( rm )
    { case srRenderModeFlat :
           glShadeModel ( GL_FLAT );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           break;
      case srRenderModeLines :
           glShadeModel ( GL_SMOOTH );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
           break;
      case srRenderModePoints :
           glShadeModel ( GL_SMOOTH );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_POINTS );
           break;
      default:
           glShadeModel ( GL_SMOOTH );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           break;
    }

   int i, j, k, s_x, s_y, s_z, order, octant;
   float botWidth, topWidth, yTop, yBot, tmp;
   SrVec vec;

   tmp = shape->resolution()*5.0f;
   int depth = SR_ROUND(tmp);
   if ( depth<1 ) depth=1;
   SrArray<SrPnt> array(0,depth*2);

   float rad = sphere.radius;

   glPushMatrix ();
   glTranslate ( sphere.center );

   for ( octant=0; octant<8; octant++ )
    { s_x = -(((octant & 01) << 1) - 1);
      s_y = -( (octant & 02)       - 1);
      s_z = -(((octant & 04) >> 1) - 1);
      //sr_out<<"Octant:"<<octant<<": "<<s_x<<srspc<<s_y<<srspc<<s_z<<srnl;
	  order = s_x * s_y * s_z;

	  for ( i=0; i<depth-1; i++ )
       { yBot = (float) i      / depth;
	     yTop = (float)(i + 1) / depth;
         botWidth = 1 - yBot;
         topWidth = 1 - yTop;

         array.size(0);
         for ( j=0; j<depth-i; j++ )
          { // First vertex
            k = order > 0 ? depth - i - j : j;
		    tmp = (botWidth * k) / (depth - i);
		    array.push().set(s_x * tmp, s_y * yBot, s_z * (botWidth - tmp));
            array.top().normalize();

            // Second vertex
		    k = order > 0 ? (depth - i - 1) - j : j;
		    tmp = (topWidth * k) / (depth - i - 1);
		    array.push().set(s_x * tmp, s_y * yTop, s_z * (topWidth - tmp));
            array.top().normalize();
          }

         // Last vertex
	     k = order > 0 ? depth - i - j : j;
	     tmp = (botWidth * k) / (depth - i);
	     array.push().set(s_x * tmp, s_y * yBot, s_z * (botWidth - tmp));
         array.top().normalize();

         if ( rm==srRenderModePoints )
          { glBegin ( GL_POINTS );
            for ( j=0; j<array.size(); j++ ) glVertex(array[j]*rad);
          }
         else if ( rm==srRenderModeFlat )
          { glBegin ( GL_TRIANGLES );
            for ( j=2; j<array.size(); j++ )
             { if ( j%2 ) swap(array[j-2],array[j-1]);
               glNormal ( triangle_normal(array[j-2],array[j-1],array[j]) );
               glVertex(array[j-2]*rad);
               glVertex(array[j-1]*rad);
               glVertex(array[j]*rad);
               if ( j%2 ) swap(array[j-2],array[j-1]);
             }
          }
         else
          { glBegin ( GL_TRIANGLE_STRIP );
            for ( j=0; j<array.size(); j++ ) { glNormal(array[j]); glVertex(array[j]*rad); }
          }
         glEnd ();
       }

      // Handle the top/bottom polygons specially, to avoid divide by zero

      array.size(3);
      yBot = (float) i / depth;
	  yTop = 1.0;
	  botWidth = 1 - yBot;

	  // First cap vertex
	  if (order > 0)
	    array[0].set(0.0, s_y * yBot, s_z * botWidth);
	  else
	    array[0].set(s_x * botWidth, s_y * yBot, 0);
	  array[0].normalize();

       // Second cap vertex
      if (order > 0)
       array[1].set(s_x * botWidth, s_y * yBot, 0);
      else
       array[1].set(0, s_y * yBot, s_z * botWidth);
	  array[1].normalize();

       // Third cap vertex
	  array[2].set(0, float(s_y), 0);

      if ( rm==srRenderModePoints )
       { glBegin ( GL_POINTS );
         for ( j=0; j<3; j++ ) glVertex(array[j]*rad);
       }
      else if ( rm==srRenderModeFlat )
       { glBegin ( GL_TRIANGLES );
         glNormal ( triangle_normal(array[0],array[1],array[2]) );
         glVertex(array[0]*rad);
         glVertex(array[1]*rad);
         glVertex(array[2]*rad);
       }
      else
       { glBegin ( GL_TRIANGLE_STRIP );
         for ( j=0; j<3; j++ ) { glNormal(array[j]); glVertex(array[j]*rad); }
       }
      glEnd ();
    }
   glPopMatrix ();
 }

//=============================== render_cylinder ====================================

void SrGlRenderFuncs::render_cylinder ( SrSnShapeBase* shape )
 {
   //SR_TRACE1 ( "Render cylinder" );

   SrCylinder& cyl = ((SrSnCylinder*)shape)->shape();

   srRenderMode rm = shape->render_mode();
   glEnable ( GL_CULL_FACE );
//   glEnable ( GL_LIGHTING );

   glMaterial ( shape->material() );

   switch ( rm )
    { case srRenderModeFlat :
           glShadeModel ( GL_FLAT );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           break;
      case srRenderModeLines :
           glShadeModel ( GL_SMOOTH );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
           break;
      case srRenderModePoints :
           glShadeModel ( GL_SMOOTH );
           break;
      default:
           glShadeModel ( GL_SMOOTH );
           glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
           break;
    }

   int nfaces = int(shape->resolution()*10.0f);
   if ( nfaces<3 ) nfaces = 3;
   
   float dang = sr2pi/float(nfaces);
   SrVec va = cyl.b-cyl.a; 
   va.normalize(); // axial vector
   SrVec minus_va = va * -1.0f;

   SrVec vr;
   float deg = SR_TODEG ( angle(SrVec::i,va) );

   if ( deg<10 || deg>170 )
     vr = cross ( SrVec::j, va );
   else
     vr = cross ( SrVec::i, va );
   
   vr.len ( cyl.radius ); // radial vector

   SrMat rot;
   rot.rot ( va, dang );

   SrPnt a1 = cyl.a+vr;
   SrPnt b1 = cyl.b+vr;
   SrPnt a2 = a1 * rot;
   SrPnt b2 = b1 * rot;

   if ( rm==srRenderModePoints )
     glBegin ( GL_POINTS );
   else
     glBegin ( GL_QUADS );

   int i=1;
   SrArray<SrPnt> vlist(0,2*(nfaces+1));
   SrVec n1, n2; // normals
   do { n1=(a1-cyl.a);  n2=(a2-cyl.a);
        n1/=cyl.radius; n2/=cyl.radius; // normalize
        vlist.push()=a1; vlist.push()=b1;
        glNormal (n1); glVertex (b1,a1);
        glNormal (n2); glVertex (a2,b2);
        if ( i==nfaces ) break;
        a1=a2; b1=b2; a2=a1*rot; b2=b1*rot;
        i++;
      } while ( true );

   glEnd ();

   if ( rm!=srRenderModePoints )
    { glBegin ( GL_POLYGON );
      glNormal ( minus_va );
      for ( i=vlist.size()-2; i>=0; i-=2 ) glVertex ( vlist[i] );
      glEnd ();
      glNormal ( va );
      glBegin ( GL_POLYGON );
      for ( i=1; i<vlist.size(); i+=2 ) glVertex ( vlist[i] );
      glEnd ();
    }
 }

//============================= render_polygon ====================================

static void render_polygon ( SrPolygon& p, srRenderMode rm, float res )
 {
   //SR_TRACE1 ( "Render polygon" );

   int i;

   if ( p.open() )
    { glBegin ( GL_LINE_STRIP );
      for ( i=0; i<p.size(); i++ ) glVertex ( p[i] );
      glEnd ();
    }
   else
    { if ( rm==srRenderModeSmooth || rm==srRenderModeFlat || rm==srRenderModeDefault )
       { SrArray<SrPnt2> tris;
         p.ear_triangulation ( tris );
         glBegin ( GL_TRIANGLES );
         for ( i=0; i<tris.size(); i++ ) glVertex ( tris[i] );
         glEnd ();
       }
      else
       { glBegin ( GL_LINE_STRIP );
         for ( i=0; i<p.size(); i++ ) glVertex ( p[i] );
         if ( !p.open() ) glVertex ( p[0] );
         glEnd ();
       }
    }

   if ( rm==srRenderModePoints || rm==srRenderModeFlat )
    { glPointSize ( res*4 );
      glBegin ( GL_POINTS );
      for ( i=0; i<p.size(); i++ ) glVertex ( p[i] );
      glEnd ();
    }
 }

void SrGlRenderFuncs::render_polygon ( SrSnShapeBase* shape )
 {
   SrPolygon& p = ((SrSnPolygon*)shape)->shape();
   if ( p.size()==0 ) return;

   float resolution = shape->resolution();

   glDisable ( GL_LIGHTING );
   glColor ( shape->material().diffuse );
   glLineWidth ( resolution ); // default is 1.0

   ::render_polygon ( p, shape->render_mode(), resolution );
 }

//============================= render_polygons ====================================

void SrGlRenderFuncs::render_polygons ( SrSnShapeBase* shape )
 {
   SrPolygons& p = ((SrSnPolygons*)shape)->shape();

   if ( p.size()==0 ) return;

   float resolution = shape->resolution();
   glDisable ( GL_LIGHTING );
   glColor ( shape->material().diffuse );
   glLineWidth ( resolution ); // default is 1.0

   int i;
   for ( i=0; i<p.size(); i++ )
    ::render_polygon ( p[i], shape->render_mode(), resolution );
 }

//======================================= EOF ====================================
