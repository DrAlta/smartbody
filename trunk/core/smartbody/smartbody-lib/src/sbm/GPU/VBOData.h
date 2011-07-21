#pragma once
#include "SbmShader.h"
#include <sbm/me_ct_ublas.hpp>
#include "gfx/vec3.h"
#include "gfx/vec4.h"

using namespace gfx;

enum
{	
	VERTEX_ATTRIBUTE_0 = 0,
	VERTEX_ATTRIBUTE_1 = 1,
	VERTEX_ATTRIBUTE_2 = 2,
	VERTEX_ATTRIBUTE_3 = 3,
	VERTEX_ATTRIBUTE_4 = 4,
	VERTEX_ATTRIBUTE_5 = 5,
	VERTEX_POSITION = GL_VERTEX_ARRAY,
	VERTEX_VBONORMAL = GL_NORMAL_ARRAY,
	VERTEX_TEXCOORD = GL_TEXTURE_COORD_ARRAY,
	VERTEX_COLOR = GL_COLOR_ARRAY,
};

enum
{
	VERTEX_BONE_ID_1 = 0,
	VERTEX_BONE_ID_2 = 1,
	VERTEX_BONE_WEIGHT_1 = 2,
	VERTEX_BONE_WEIGHT_2 = 3,
	VERTEX_TANGENT= 4,
	VERTEX_BINORMAL = 5,
};



class VBOData
{
public:
	unsigned int m_iVBO_ID;
	bool m_bUpdate;
	GLuint m_ArrayType;

	char m_Name[20];		
	ublas::vector<Vec2f> *data_Vec2f;
	ublas::vector<Vec3f> *data_Vec3f;
	ublas::vector<Vec3i> *data_Vec3i;
	ublas::vector<Vec4f> *data_Vec4f;	
	ublas::vector<float> *data_float;
public:	
	VBOData(char* name, int type, ublas::vector<Vec2f>& Data);
	VBOData(char* name, int type, ublas::vector<Vec3f>& Data);
	VBOData(char* name, int type, ublas::vector<Vec3i>& Data);
	VBOData(char* name, int type, ublas::vector<Vec4f>& Data);	
	VBOData(char* name, int type, ublas::vector<float>& Data);
public:
	~VBOData(void);
	
	void BindBuffer();
	void UnbindBuffer();
	void Update();

	void Debug(const char* tag = "VBO");

protected:
	void EnableClient(int ArrayType);
	void DisableClient(int ArrayType);
};

template<class S>
class VBODataArray
{	
protected:
	ublas::vector<S> m_Data;
	VBOData*   m_pVBO;	
public:
	VBODataArray(char* name, int type, ublas::vector<S>& data);
	VBODataArray(char* name, int type, int nSize);
	~VBODataArray();
	VBOData* VBO() const { return m_pVBO; }
};

template<class S>
VBODataArray<S>::VBODataArray( char* name, int type, int nSize )
{
	m_Data.resize(nSize);
	m_pVBO = new VBOData(name,type,m_Data);
	m_pVBO->Update();
}

template<class S>
VBODataArray<S>::~VBODataArray()
{
	if (m_pVBO)
		delete m_pVBO;
}

template<class S>
VBODataArray<S>::VBODataArray( char* name, int type, ublas::vector<S>& data )
{
	m_Data.resize(data.size());
	//copy(data,m_Data);
	m_Data = data;
	m_pVBO = new VBOData(name,type,m_Data);
	m_pVBO->Update();
}

typedef VBODataArray<Vec3f> VBOVec3f;
typedef VBODataArray<Vec4f> VBOVec4f;
typedef VBODataArray<Vec3i> VBOVec3i;
