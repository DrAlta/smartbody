/*  sr_model_import_obj.cpp - part of Motion Engine and SmartBody-lib
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

#include <sr/sr_string_array.h>
#include <sr/sr_model.h>
#include <sb/SBTypes.h>

#if !defined (__ANDROID__) && !defined(SB_IPHONE)
#include <sbm/GPU/SbmTexture.h>
#endif

//# define SR_USE_TRACE1    // keyword tracking
//# include <sr/sr_trace.h>

# define GETID(n,A) in>>n; if (n>0) n--; else if (n<0) n+=A.size()
 
static void get_face ( SrInput& in, SrModel& m, int& vc, int& vt, int& vn )
 {
   vc = vt = vn = -1;
   GETID(vc,m.V);

   if ( in.get_token()!=SrInput::Delimiter ) // was only: vc
    { in.unget_token(); return; }

   if ( in.get_token()==SrInput::Delimiter ) // is: vc//vn
    { GETID(vn,m.N);
      return;
    }

   in.unget_token();

   GETID(vt,m.T); // is: vc/vt

   if ( in.get_token()==SrInput::Delimiter )
    { GETID(vn,m.N); // is: vc/vt/vn
      return;
    }

   if ( !in.finished() ) in.unget_token (); // is only: vc/vt
 }

static SrColor read_color ( SrInput& in )
 {
   float r, g, b;
   in >> r >> g >> b;
   SrColor c(r,g,b);
   return c;
 }

static void load_texture(int type, const char* file, const SrStringArray& paths)
{
#if !defined (__ANDROID__) && !defined(SB_IPHONE)
	SrString s;
	SrInput in;
	std::string imageFile = file;
	in.init( fopen(file,"r"));
	int i = 0;
	while ( !in.valid() && i < paths.size())
	{
		s = paths[i++];
		s << file;
		imageFile = s;
		in.init ( fopen(s,"r") );
	}
	if (!in.valid()) return;		
	SbmTextureManager& texManager = SbmTextureManager::singleton();
	texManager.loadTexture(type,file,s);	
#endif
}

static void read_materials ( SrArray<SrMaterial>& M,
                             SrStringArray& mnames,
							 std::map<std::string,std::string>& mtlTexMap,
							 std::map<std::string,std::string>& mtlTexBumpMap,
                             const SrString& file,
                             const SrStringArray& paths )
 {
   SrString s;
   SrInput in;

   in.lowercase_tokens(false);
   in.init ( fopen(file,"rt") );
   int i=0;
   while ( !in.valid() && i<paths.size() )
    { s = paths[i++];
      s << file;
      in.init ( fopen(s,"rt") );
    }
   if ( !in.valid() ) return; // could not get materials

   while ( !in.finished() )
    { in.get_token();
      if ( in.last_token()=="newmtl" )
       { 
	     SrMaterial material;
		 material.init();
		 //M.push().init();
		 M.push(material);        
         //SR_TRACE1 ( "new material: "<<in.last_token() );
		 SrString matName;
		 in.getline(matName);
		 matName.ltrim();
		 mnames.push ( matName );
       }	  
      else if ( in.last_token()=="Ka" )
       { M.top().ambient = read_color ( in );
       }
      else if ( in.last_token()=="Kd" )
       { M.top().diffuse = read_color ( in );
       }
      else if ( in.last_token()=="Ks" )
       { M.top().specular = read_color ( in );
       }
      else if ( in.last_token()=="Ke" ) // not sure if this one exists
       { M.top().emission = read_color ( in );
       }
      else if ( in.last_token()=="Ns" )
       { in >> i;
         M.top().shininess = i;
       }
	  else if ( in.last_token()=="map_Kd") // texture map
	  {
		  SrString mapKaName, dotstr, ext;			  
		  in.get_token(mapKaName);		  		  
		  in.get_token(dotstr);	
		  in.get_token(ext);
		  mapKaName.append(dotstr);
		  mapKaName.append(ext);
		  std::string texFile = (const char*) mapKaName;
		  std::string mtlName = mnames.top();
		  mtlTexMap[mtlName] = texFile;		  
	  }
	  else if ( in.last_token()=="map_bump") // texture map
	  {
		  SrString mapBump, dotstr, ext;
		  //in >> mapKaName;		  		 		  
		  in.get_token(mapBump);
		  in.get_token(dotstr);	
		  in.get_token(ext);
		  mapBump.append(dotstr);
		  mapBump.append(ext);
		  std::string texFile = (const char*) mapBump;
		  std::string mtlName = mnames.top();
		  mtlTexBumpMap[mtlName] = texFile;		  
	  }
      else if ( in.last_token()=="illum" ) // dont know what is this one
       { in >> i;
       }
    }
 }

static bool process_line ( const SrString& line,
                           SrModel& m,
                           SrStringArray& paths,
                           SrStringArray& mnames,
						   std::map<std::string,std::string>& mtlTexMap,
						   std::map<std::string,std::string>& mtlTexBumpMap,
                           int& curmtl )
 {
   SrInput in (line);
   in.get_token();

   if ( in.last_token().len()==0 ) return true;

   if ( in.last_token()=="v" ) // v x y z [w]
    { //SR_TRACE1 ( "v" );
      m.V.push();
      in >> m.V.top();	 
    }
   else if ( in.last_token()=="vn" ) // vn i j k
    { //SR_TRACE1 ( "vn" );
      m.N.push();
      in >> m.N.top();
    }
   else if ( in.last_token()=="vt" ) // vt u v [w]
    { //SR_TRACE1 ( "vt" );
      m.T.push();
      in >> m.T.top();
    }
   else if ( in.last_token()=="g" )
    { //SR_TRACE1 ( "g" );
    }
   else if ( in.last_token()=="f" ) // f v/t/n v/t/n v/t/n (or v/t or v//n or v)
    { //SR_TRACE1 ( "f" );
      int i=0;
      SrArray<int> v, t, n;
      v.capacity(8); t.capacity(8); n.capacity(8);
      while ( true )
       { if ( in.get_token()==SrInput::EndOfFile ) break;
         in.unget_token();
         get_face ( in, m, v.push(), t.push(), n.push() );
         if ( in.last_error()==SrInput::UnexpectedToken ) return false;
       }

      if ( v.size()<3 ) return false;

      for ( i=2; i<v.size(); i++ )
       { m.F.push().set ( v[0], v[i-1], v[i] );
         m.Fm.push() = curmtl;

         if ( t[0]>=0 && t[1]>=0 && t[i]>=0 )
          m.Ft.push().set( t[0], t[i-1], t[i] );

         if ( n[0]>=0 && n[1]>=0 && n[i]>=0 )
          m.Fn.push().set ( n[0], n[i-1], n[i] );
       }
    }
   else if ( in.last_token()=="s" ) // smoothing groups not supported
    { //SR_TRACE1 ( "s" );
      in.get_token();
    }
   else if ( in.last_token()=="usemap" ) // usemap name/off
    { //SR_TRACE1 ( "usemap" );
    }
   else if ( in.last_token()=="usemtl" ) // usemtl name
    { //SR_TRACE1 ( "usemtl" );
      //in.get_token ();
	  SrString matName;
	  in.getline(matName);
	  matName.ltrim();
      curmtl = mnames.lsearch ( matName );
      //SR_TRACE1 ( "curmtl = " << curmtl << " (" << in.last_token() << ")" );
    }
   else if ( in.last_token()=="mtllib" ) // mtllib file1 file2 ...
    { //SR_TRACE1 ( "mtllib" );
      SrString token, file;
      int pos = line.get_next_string ( token, 0 ); // parse again mtllib
      while ( true )
       { pos = line.get_next_string ( token, pos );
         if ( pos<0 ) break;
         token.extract_file_name ( file );
         token.replace ( "\\", "/" ); // avoid win-unix problems
         //SR_TRACE1 ( "new path: "<<token );
         paths.push ( token );
         read_materials ( m.M, mnames, mtlTexMap, mtlTexBumpMap, file, paths );
       }
    }

   return true;
 }

#if 0
/************************************************************************/
/* Import Ply mesh                                                      */
/************************************************************************/
bool SrModel::import_ply( const char* file )
{
	PlyFile *ply;
	int nelems;
	char **elist;
	int file_type;
	float version;
	int nprops;
	int num_elems;
	char *elem_name;
	PlyProperty **plist;
	
	
	int curmtl = -1;

	typedef struct PlyFace {
		unsigned char nverts;    /* number of vertex indices in list */
		int *verts;              /* vertex index list */
	} PlyFace;

	typedef struct PlyVertex {
		float x,y,z;             /* the usual 3-space position of a vertex */
	} PlyVertex;

	PlyProperty vert_props[] = { /* list of property information for a vertex */
		{"x", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,x), 0, 0, 0, 0},
		{"y", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,y), 0, 0, 0, 0},
		{"z", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,z), 0, 0, 0, 0},
	};

	PlyProperty face_props[] = { /* list of property information for a vertex */		
		{"vertex_indices", PLY_INT, PLY_INT, offsetof(PlyFace,verts),1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyFace,nverts)},
	};

	PlyVertex **vlist;
	char* filename = const_cast<char*>(file);
	/* open a PLY file for reading */
	ply = ply_open_for_reading(filename, &nelems, &elist, &file_type, &version);

	/* print what we found out about the file */
	printf("Ply version %f\n", version);
	//printf("Ply type %d\n", file_type);
	//printf("Ply nelems = %d\n",nelems);
	/* go through each kind of element that we learned is in the file */
	/* and read them */

	for (int i = 0; i < nelems; i++) {

		/* get the description of the first element */
		elem_name = elist[i];
		plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

		/* print the name of the element, for debugging */
		//printf ("element %s %d\n", elem_name, num_elems);

		/* if we're on vertex elements, read them in */
		if (equal_strings ("vertex", elem_name)) {			
			/* create a vertex list to hold all the vertices */
			vlist = (PlyVertex **) malloc (sizeof (PlyVertex *) * num_elems);

			/* set up for getting vertex elements */

			ply_get_property (ply, elem_name, &vert_props[0]);
			ply_get_property (ply, elem_name, &vert_props[1]);
			ply_get_property (ply, elem_name, &vert_props[2]);

			V.size(num_elems);
			/* grab all the vertex elements */
			for (int j = 0; j < num_elems; j++) {

				/* grab and element from the file */
				vlist[j] = (PlyVertex *) malloc (sizeof (PlyVertex));
				//PlyVertex tempPos;
				ply_get_element (ply, (void *) vlist[j]);
				V[j] = SrVec(vlist[j]->x,vlist[j]->y,vlist[j]->z);
				/* print out vertex x,y,z for debugging */
				if (j >= 229)
				{
					printf("vertex %d\n",j);
					printf ("vertex: %f %f %f\n",vlist[j]->x,vlist[j]->y,vlist[j]->z);
				}				
			}
		}

		/* if we're on face elements, read them in */
		if (equal_strings ("face", elem_name)) {	

			ply_get_property (ply, elem_name, &face_props[0]);			
			/* grab all the face elements */
			for (int j = 0; j < num_elems; j++) {				
				PlyFace tempFace;
				ply_get_element (ply, (void *) &tempFace);

				for (int k=2; k<tempFace.nverts; k++ )
				{ 
					F.push().set ( tempFace.verts[0], tempFace.verts[k-1], tempFace.verts[k] );
				    Fm.push() = curmtl;
				}
				//printf("number vtx = %d ", tempFace.nverts);
				/* print out face info, for debugging */				
				//for (int k = 0; k < tempFace.nverts; k++)
				//	printf ("%d ", tempFace.verts[k]);
				//printf ("\n");
			}
		}

		/* print out the properties we got, for debugging */
		//for (int j = 0; j < nprops; j++)
		//	printf ("property %s\n", plist[j]->name);
	}
	
	/* close the PLY file */
	ply_close (ply);
	validate ();
	remove_redundant_materials ();
	//   remove_redundant_normals ();
	compress ();

	return true;
}
#endif


/*! This function import the .obj format. If the import
    is succesfull, true is returned
    and the model m will contain the imported object, otherwise false
    is returned. */
bool SrModel::import_obj ( const char* file )
{
  SrInput in ( fopen(file,"rt") );
  if ( !in.valid() ) return false;
   in.comment_style ( '#' );
   in.lowercase_tokens ( false );

   SrString path=file;
   SrString filename;
   path.extract_file_name(filename);
   SrStringArray paths;
   paths.push ( path );
  
   //SR_TRACE1 ( "First path:" << path );

   int curmtl = -1;

   init ();
   name = filename;
   name.remove_file_extension();

   SrString line;
   while ( in.getline(line)!=EOF )
    { if ( !process_line(line,*this,paths,mtlnames,mtlTextureNameMap,mtlNormalTexNameMap,curmtl) ) return false;
    }

   validate ();
   remove_redundant_materials ();
//   remove_redundant_normals ();
   compress ();

   // after remove all redundant materials, load the corresponding textures   
#if !defined (__ANDROID__) && !defined(SB_IPHONE)
   for (int i=0;i<M.size();i++)
   {
	   std::string matName = mtlnames[i];
	   if (mtlTextureNameMap.find(matName) != mtlTextureNameMap.end())
	   {
		   load_texture(SbmTextureManager::TEXTURE_DIFFUSE,mtlTextureNameMap[matName].c_str(),paths);	   
	   }	
	   if (mtlNormalTexNameMap.find(matName) != mtlNormalTexNameMap.end())
	   {
		   load_texture(SbmTextureManager::TEXTURE_NORMALMAP,mtlNormalTexNameMap[matName].c_str(),paths);	   
	   }
   }
#endif
   //SR_TRACE1("Ok!");
   return true;
 }

//============================ EOF ===============================
