#include <stdio.h>
#include "VBOData.h"



VBOData::~VBOData(void)
{
	if (m_iVBO_ID)
		glDeleteBuffersARB(1,&m_iVBO_ID);
	printf("%10s (%d) is now destroied",m_Name,m_iVBO_ID);
}

void VBOData::BindBuffer()
{	
	if (data_Vec4f || data_Vec3f || data_float || data_Vec2f){
		//printf("Enable Client\n");
		//glEnableClientState(m_ArrayType);
		//printf("Array Type = %d\n",m_ArrayType);		
		EnableClient(m_ArrayType);	
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_iVBO_ID);
		
	}else
	{
		//printf("Enable element\n");
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_iVBO_ID);
	}
}

void VBOData::UnbindBuffer()
{
	if (data_Vec4f || data_Vec3f || data_float){
		//glDisableClientState(m_ArrayType);
		//printf("Array Type = %d\n",m_ArrayType);
		DisableClient(m_ArrayType);
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0);
	}else
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void VBOData::Update()
{
	m_bUpdate = true;

	if (m_bUpdate)
	{
		if (m_iVBO_ID<=0)
			// Get A Valid Name
			glGenBuffersARB( 1, &m_iVBO_ID);
		if (data_Vec3f){
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_iVBO_ID );
			glBufferDataARB( GL_ARRAY_BUFFER_ARB, data_Vec3f->size()*3*sizeof(float),
				getPtr(*data_Vec3f), GL_STATIC_DRAW_ARB );
			//printf("VBO vtx dim = %d\n",data_Vec3f->dim(0));
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0);
		}else if (data_Vec3i){
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,m_iVBO_ID);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,data_Vec3i->size()*3*sizeof(int),
				getPtr(*data_Vec3i),GL_STATIC_DRAW_ARB);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}else if (data_Vec4f) {
			//printf("Bind 4f\n");
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_iVBO_ID);
			//printf("VBO attrib dim = %d\n",data_Vec4f->dim(0));
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,data_Vec4f->size()*4*sizeof(float),
				getPtr(*data_Vec4f),GL_STATIC_DRAW_ARB);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
		else if (data_Vec2f) {
			//printf("Bind 4f\n");
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,m_iVBO_ID);
			//printf("VBO attrib dim = %d\n",data_Vec4f->dim(0));
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,data_Vec2f->size()*2*sizeof(float),
				getPtr(*data_Vec2f),GL_STATIC_DRAW_ARB);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
		m_bUpdate = false;
	}
}

VBOData::VBOData( char* name, int type, ublas::vector<Vec2f>& Data )
{
	m_ArrayType=type;
	strcpy(m_Name,name);

	data_Vec2f = &Data;	
	data_Vec3f = NULL;
	data_Vec3i = NULL;
	data_Vec4f = NULL;
	data_float = NULL;

	m_bUpdate = true;
	m_iVBO_ID = 0;
}

VBOData::VBOData( char* name, int type, ublas::vector<float>& Data )
{
	m_ArrayType=type;
	strcpy(m_Name,name);
	data_float = &Data;
	data_Vec3f = NULL;
	data_Vec3i = NULL;
	data_Vec4f = NULL;
	data_Vec2f = NULL;

	m_bUpdate = true;
	m_iVBO_ID = 0;
}

VBOData::VBOData( char* name, int type, ublas::vector<Vec3f>& Data )
{
	m_ArrayType=type;
	strcpy(m_Name,name);
	data_Vec2f = NULL;
	data_Vec3f = &Data;
	data_Vec3i = NULL;
	data_Vec4f = NULL;
	data_float = NULL;

	m_bUpdate = true;
	m_iVBO_ID = 0;
}

VBOData::VBOData( char* name, int type, ublas::vector<Vec3i>& Data )
{
	m_ArrayType=type;
	strcpy(m_Name,name);
	data_Vec2f = NULL;
	data_Vec3f = NULL;
	data_Vec3i = &Data;
	data_Vec4f = NULL;
	data_float = NULL;

	m_bUpdate = true;
	m_iVBO_ID = 0;
}

VBOData::VBOData( char* name, int type, ublas::vector<Vec4f>& Data )
{
	m_ArrayType=type;
	strcpy(m_Name,name);
	data_Vec2f = NULL;
	data_Vec3f = NULL;
	data_Vec3i = NULL;
	data_Vec4f = &Data;
	data_float = NULL;

	m_bUpdate = true;
	m_iVBO_ID = 0;

}


void VBOData::EnableClient( int ArrayType )
{
	if (ArrayType <= 3)
	{
		//printf("Enable Vertex Attributes %d\n",ArrayType);
		glEnableVertexAttribArray(ArrayType);
	}
	else
		glEnableClientState(ArrayType);
}

void VBOData::DisableClient( int ArrayType )
{
	if (ArrayType <= 3)
	{
		glDisableVertexAttribArray(ArrayType);
	}
	else
		glDisableClientState(ArrayType);
}

void VBOData::Debug(const char* tag/* = "TBO"*/)
{
	float testData[8];
	glBindBuffer(GL_ARRAY_BUFFER, m_iVBO_ID);	
	float* pData = (float*) glMapBuffer(GL_ARRAY_BUFFER,GL_READ_ONLY);
	for (int i=0;i<8;i++)
		testData[i] = pData[i];
	glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_TEXTURE_BUFFER_EXT,0);

	printf("%s : ",tag);
	for (int i=0;i<8;i++)
		printf("%f ",testData[i]);
	printf("\n");

}