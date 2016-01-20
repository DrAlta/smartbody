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

#include <vhcl.h>
#include <sr/sr_string_array.h>
#include <sr/sr_model.h>
#include <sb/SBTypes.h>
#include <external/rply/rply.h>

#if !defined (__ANDROID__) && !defined(SB_IPHONE) && !defined(EMSCRIPTEN)
#include <sbm/GPU/SbmTexture.h>
#endif

#include <sstream>

#define TEST_TEXTURE 1

static int vertex_cb(p_ply_argument argument) {
	static int count = 0;
	long idx;	
	SrModel* model;
	ply_get_argument_user_data(argument, (void**)&model, &idx);
	if (idx == 0)
	{
		model->V.push_back(SrVec());
		model->T.push_back(SrVec2());
	}
	double argumentValue = ply_get_argument_value(argument);
	model->V.back()[(int)idx] = (float) argumentValue;	
	return 1;
}

static int vertex_color_cb(p_ply_argument argument) {
	static int count = 0;
	long idx;	
	SrModel* model;
	ply_get_argument_user_data(argument, (void**)&model, &idx);
	if (idx == 0)
		model->Vc.push_back(SrVec());
	double argumentValue = ply_get_argument_value(argument);
	model->Vc.back()[(int)idx] = (float) argumentValue/255.0f;	
	return 1;
}

static int face_cb(p_ply_argument argument) {	
	long length, value_index;
	long idx;
	SrModel* model;
	ply_get_argument_user_data(argument, (void**)&model, &idx);
	ply_get_argument_property(argument, NULL, &length, &value_index);
	
	if (value_index == -1) // first entry in the list
	{
		model->F.push_back(SrVec3i());
		int mtlIdx = model->M.size()-1;
		model->Fm.push_back(mtlIdx);
	}
	else if (value_index >= 0 && value_index <= 2) // a triangle face
	{
		double argumentValue = ply_get_argument_value(argument);
		model->F.back()[value_index] = (int)(float) argumentValue;		
	}		
	return 1;
}

static int texCoord_cb(p_ply_argument argument) {	
	long length, value_index;
	long idx;
	SrModel* model;
	ply_get_argument_user_data(argument, (void**)&model, &idx);
	ply_get_argument_property(argument, NULL, &length, &value_index);

	if (value_index == -1) // first entry in the list
	{
		model->Ft.push_back(SrVec3i());
		//SrModel::Face& fid = model->F[model->Ft.size()-1];
		//model->Ft.top().set(fid[0],fid[1],fid[2]);
		int tsize = (model->Ft.size()-1)*3;
		model->Ft.back() = SrVec3i(tsize+0,tsize+1,tsize+2); 
		for (int i=0;i<3;i++)
			model->T.push_back(SrVec2());
	}
	else if (value_index >= 0 && value_index <= 5) // a triangle face
	{
		double argumentValue = ply_get_argument_value(argument);
		int faceIndex = model->Ft.size() - 1;
		int faceNumber = (value_index/2);
		if (value_index == 0 || value_index == 2 || value_index == 4)
			model->T[faceIndex * 3 + faceNumber].x = (float) argumentValue;
		else
			model->T[faceIndex * 3 + faceNumber].y = (float) argumentValue;

	}		
	return 1;
}

static int texNumber_cb(p_ply_argument argument) {	
	long length, value_index;
	long idx;
	SrModel* model;
	ply_get_argument_user_data(argument, (void**)&model, &idx);
	ply_get_argument_property(argument, NULL, &length, &value_index);

	if (value_index == -1) // first entry in the list
	{
	}
	else if (value_index >= 0 && value_index <= 5) // a triangle face
	{
		double argumentValue = ply_get_argument_value(argument);
		int materialIndex = (int) argumentValue;
		model->Fm[model->Fm.size() - 1] = materialIndex + 1;
	}		
	return 1;
}



#if 1

static void load_texture(int type, const char* file, const SrStringArray& paths)
{
#if !defined (__ANDROID__) && !defined(SB_IPHONE) && !defined(EMSCRIPTEN)
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
	LOG("loading texture file = %s", imageFile.c_str());
	texManager.loadTexture(type,file,s);		
#endif
}

/************************************************************************/
/* Import Ply mesh                                                      */
/************************************************************************/
bool SrModel::import_ply( const char* file )
{

	long nvertices, ntriangles;
	M.push_back(SrMaterial());
	M.back().diffuse = SrColor::gray;

	mtlnames.push_back("noname");
	p_ply ply = ply_open(file, NULL, 0, NULL);
	if (!ply) return false;
	if (!ply_read_header(ply)) return false;

#if TEST_TEXTURE
	const char* comment = ply_get_next_comment(ply, NULL);
	int materialNum =  1;
	while (comment)
	{
		std::string commentStr = comment;
		std::vector<std::string> tokens;
		vhcl::Tokenize(commentStr,tokens);
		if (tokens.size() > 1 && tokens[0] == "TextureFile")
		{
			std::string texFile = tokens[1];
			std::stringstream strstr; 
			strstr << "mat" << materialNum;
			materialNum++;
			std::string mtlName = strstr.str();	
			SrMaterial material;
			material.init();
			material.diffuse = SrColor::white;
			//M.push().init();
			M.push_back(material);        
			//SR_TRACE1 ( "new material: "<<in.last_token() );	
			mtlnames.push_back ( mtlName.c_str() );
			LOG("texture found : %s", texFile.c_str());
			mtlTextureNameMap[mtlName] = texFile;	
		}
		comment = ply_get_next_comment(ply,comment);
	}
#endif

	SrString path=file;
	SrString filename;
	path.extract_file_name(filename);
	
	SrStringArray paths;
	paths.push ( path );

	name = filename;
	name.remove_file_extension();
	nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, this, 0);
	ply_set_read_cb(ply, "vertex", "y", vertex_cb, this, 1);
	ply_set_read_cb(ply, "vertex", "z", vertex_cb, this, 2);
	ply_set_read_cb(ply, "vertex", "red", vertex_color_cb, this, 0);
	ply_set_read_cb(ply, "vertex", "green", vertex_color_cb, this, 1);
	ply_set_read_cb(ply, "vertex", "blue", vertex_color_cb, this, 2);
	ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, this, 0);
#if TEST_TEXTURE
	ntriangles = ply_set_read_cb(ply, "face", "texcoord", texCoord_cb, this, 0);
	ply_set_read_cb(ply, "face", "texnumber", texNumber_cb, this, 0);

#endif
	LOG("%ld\n%ld\n", nvertices, ntriangles);
	if (!ply_read(ply))
		return false;
	ply_close(ply);

	validate ();
	remove_redundant_materials ();
	remove_redundant_texcoord();
	//   remove_redundant_normals ();
	compress ();

// 	for (int i=0;i<T.size();i++)
// 	{
// 		if (i%100 == 0)
// 			LOG("Tex Coord %d = %.4f %.4f", i, T[i].x, T[i].y);
// 	}

#if TEST_TEXTURE
#if !defined (__ANDROID__) && !defined(SB_IPHONE) && !defined(EMSCRIPTEN)
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
#endif

	return true;
}
#endif


//============================ EOF ===============================
