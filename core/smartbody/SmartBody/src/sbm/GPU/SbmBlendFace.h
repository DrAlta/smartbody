#pragma once
#include <sb/SBTypes.h>
#include "SbmShader.h"
#include "VBOData.h"
#include "TBOData.h"
#include <sbm/sbm_deformable_mesh.h>

#if !defined(__FLASHPLAYER__)
#include "external/glew/glew.h"
#endif

class SbmDeformableMeshGPU;

class SbmBlendFace: public DeformableMesh
{
	public:
		SbmBlendFace();
		~SbmBlendFace();

		bool			buildVertexBufferGPU();
		void			addFace(SbmDeformableMeshGPU*);
		void			initShaderProgram();
		void			initShaderProgram_Dan();
		void			initShader();
		
		void			setDeformableMesh(DeformableMesh*);
		DeformableMesh* getDeformableMesh();
		
		VBOVec3f*		getVBOPos(int);
		VBOVec3f*		getVBONormal();
		VBOVec2f*		getVBOTexCoord();
		VBOVec3i*		getVBOTri();
		
		std::vector<VBOVec3i*> subMeshTris;

		GLuint			_vsID;
		GLuint			_fsID;
		GLuint			_programID;

	private:
		DeformableMesh*			_mesh;
		std::vector<VBOVec3f*>	_VBOPos;	
		VBOVec3f*				_VBONormal;
		VBOVec2f*				_VBOTexCoord;
		VBOVec3i*				_VBOTri;

		bool			_initGPUVertexBuffer;
		
		std::string		_shaderName;

		unsigned int	_faceCounter;				// How many faces have been loaded
	

};


class SbmBlendTextures
{
	public:
		SbmBlendTextures();
		~SbmBlendTextures();

		static GLuint getShader(const std::string);
		static void BlendTwoFBO(GLuint, GLuint, GLuint, GLuint, float, GLuint, int, int);
		static void BlendAllAppearances(GLuint, GLuint, std::vector<float>, std::vector<GLuint>, GLuint, int, int);

		GLuint			_vsID;
		GLuint			_fsID;
		GLuint			_programID;

	private:
		std::string		_shaderName;
};