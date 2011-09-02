#include "SbmDeformableMeshGPU.h"
#include "SbmTexture.h"
#include <string>
#include <set>
#include <algorithm>

typedef std::pair<int,float> IntFloatPair;

static bool intFloatComp(const IntFloatPair& p1, const IntFloatPair& p2)
{
	return (p1.second > p2.second);
}


const char* ShaderDir = "../../smartbody-lib/src/sbm/GPU/shaderFiles/";
const char* VSName = "vs_skin_pos.vert";
const char* FSName = "fs_skin_render.frag";
const std::string shaderName = "MeshSkin";
bool SbmDeformableMeshGPU::initShader = false;
bool SbmDeformableMeshGPU::useGPUDeformableMesh = true;

std::string shaderVS = 
"#version 120 \n\
#extension GL_EXT_gpu_shader4 : require \n\
uniform samplerBuffer Transform; \n\
attribute vec4 BoneID1,BoneID2;   \n\
attribute vec4 BoneWeight1,BoneWeight2;\n \
attribute vec3 tangent, binormal;\n\
varying vec3 normal,lightDir[2],halfVector[2];\n\
varying vec3 tv,bv;\n\
varying float dist[2];\n\
mat3 GetTransformation(float id)\n \
{ \n\
	int idx = int(id);\n \
	mat3 rot;\n  \
	for (int i=0;i<3;i++)\n \
	{ \n  \
		for (int j=0;j<3;j++)\n \
			rot[j][i] = texelFetchBuffer(Transform,(idx*16+i*4+j)).x;\n		\
	}\n	\
	return rot;\n \
}\n \
vec3 GetTranslation(float id)\n \
{\n  \
	int idx = int(id);\n \
	vec3 tran;\n \
	tran[0] = texelFetchBuffer(Transform,(idx*16+12)).x;\n \
    tran[1] = texelFetchBuffer(Transform,(idx*16+13)).x;\n \
	tran[2] = texelFetchBuffer(Transform,(idx*16+14)).x;\n \
	return tran;\n	\
}\n  \
mat4 TransformPos(vec3 position, vec3 normal, vec3 tang, vec3 binorm, vec4 boneid, vec4 boneweight)\n\
{\n\
	vec3 pos = vec3(0,0,0);\n\
	vec3 n = vec3(0,0,0);\n\
	vec3 t = vec3(0,0,0);\n\
	vec3 b = vec3(0,0,0);\n\
	mat3 tempT;\n\
	vec3 tempt;\n\
	for (int i=0;i<4;i++)\n\
	{\n\
		tempT = GetTransformation(boneid[i]);\n\
		tempt = GetTranslation(boneid[i]);\n\
		pos += (position*tempT+tempt)*boneweight[i];\n\
		n   += (normal*tempT)*boneweight[i];\n\
		t   += (tang*tempT)*boneweight[i];\n\
		b   += (binorm*tempT)*boneweight[i];\n\
	}	\n\
	mat4 result;\n\
	result[0] = vec4(pos,1.0);\n\
	result[1] = vec4(n,1.0);\n\
	result[2] = vec4(t,1.0);\n\
	result[3] = vec4(b,1.0);\n\
	return result;\n\
}\n\
void main()\n \
{\n	\
	// the following three lines provide the same result\n \
	vec3 pos = vec3(gl_Vertex.xyz);\n \
	mat4 skin = TransformPos(pos,gl_Normal,tangent,binormal,BoneID1,BoneWeight1) + TransformPos(pos,gl_Normal,tangent,binormal,BoneID2,BoneWeight2);\n\
	gl_Position = gl_ModelViewProjectionMatrix*vec4(skin[0].xyz,1.0);\n\
	lightDir[0] = normalize(vec3(gl_LightSource[0].position));\n\
	halfVector[0] = normalize(gl_LightSource[0].halfVector.xyz);\n\
	dist[0] = 0.0;\n\
	vec3 posDir = vec3(gl_LightSource[1].position - gl_ModelViewMatrix * vec4(skin[0].xyz,1.0));\n\
	dist[1] = length(posDir);\n\
	lightDir[1] = normalize(posDir);\n\
	halfVector[1] = normalize(gl_LightSource[1].halfVector.xyz);\n\
	int colorIdx = int(gl_Vertex.w); \n\
	gl_TexCoord[0] = gl_MultiTexCoord0;\n\
	normal = normalize(gl_NormalMatrix * skin[1].xyz);\n\
	tv     = normalize(gl_NormalMatrix * tangent.xyz);\n\
	bv     = normalize(gl_NormalMatrix * binormal.xyz);\n\
}\n";

std::string shaderVS_2 = 
"uniform mat4 Transform[140]; \n\
attribute vec4 BoneID1,BoneID2;   \n\
attribute vec4 BoneWeight1,BoneWeight2;\n \
attribute vec3 tangent, binormal;\n\
varying vec3 normal,lightDir[2],halfVector[2];\n\
varying vec3 tv,bv;\n\
varying float dist[2];\n\
mat4 GetTransformation(float id)\n \
{ \n\
int idx = int(id);\n \
return Transform[idx];\n \
}\n \
mat4 TransformPos(vec3 position, vec3 normal, vec3 tang, vec3 binorm, vec4 boneid, vec4 boneweight)\n\
{\n\
vec3 pos = vec3(0,0,0);\n\
vec3 n = vec3(0,0,0);\n\
vec3 t = vec3(0,0,0);\n\
vec3 b = vec3(0,0,0);\n\
mat4 tempT;\n\
vec3 tempt;\n\
for (int i=0;i<4;i++)\n\
{\n\
tempT = GetTransformation(boneid[i]);\n\
pos += (vec4(position,1.0)*tempT).xyz*boneweight[i];\n\
n   += (vec4(normal,0.0)*tempT).xyz*boneweight[i];\n\
t   += (vec4(tang,0.0)*tempT).xyz*boneweight[i];\n\
b   += (vec4(binorm,0.0)*tempT).xyz*boneweight[i];\n\
}	\n\
mat4 result;\n\
result[0] = vec4(pos,1.0);\n\
result[1] = vec4(n,1.0);\n\
result[2] = vec4(t,1.0);\n\
result[3] = vec4(b,1.0);\n\
return result;\n\
}\n\
void main()\n \
{\n	\
// the following three lines provide the same result\n \
vec3 pos = vec3(gl_Vertex.xyz);\n \
mat4 skin = TransformPos(pos,gl_Normal,tangent,binormal,BoneID1,BoneWeight1) + TransformPos(pos,gl_Normal,tangent,binormal,BoneID2,BoneWeight2);\n\
gl_Position = gl_ModelViewProjectionMatrix*vec4(skin[0].xyz,1.0);\n\
lightDir[0] = normalize(vec3(gl_LightSource[0].position));\n\
halfVector[0] = normalize(gl_LightSource[0].halfVector.xyz);\n\
dist[0] = 0.0;\n\
vec3 posDir = vec3(gl_LightSource[1].position - gl_ModelViewMatrix * vec4(skin[0].xyz,1.0));\n\
dist[1] = length(posDir);\n\
lightDir[1] = normalize(posDir);\n\
halfVector[1] = normalize(gl_LightSource[1].halfVector.xyz);\n\
int colorIdx = int(gl_Vertex.w); \n\
gl_TexCoord[0] = gl_MultiTexCoord0;\n\
normal = normalize(gl_NormalMatrix * skin[1].xyz);\n\
tv     = normalize(gl_NormalMatrix * tangent.xyz);\n\
bv     = normalize(gl_NormalMatrix * binormal.xyz);\n\
}\n";

std::string shaderFS =
"const vec3 ambient = vec3(0.0,0.0,0.0);//(vec3(255 + 127, 241, 0 + 2)/255.f)*(vec3(0.2,0.2,0.2));\n\
uniform sampler2D diffuseTexture;\n\
uniform sampler2D normalTexture;\n\
uniform int  useTexture;\n\
uniform int  useNormalMap;\n\
varying vec3 normal,lightDir[2],halfVector[2];\n\
varying vec3 tv,bv;\n\
uniform vec3 diffuseMaterial;\n\
uniform vec3 specularMaterial;\n\
uniform float  shineness;\n\
//uniform vec3 specularColors;\n\
varying float dist[2];\n\
void main (void)\n\
{  \n\
	vec3 n,halfV;\n\
	float NdotL,NdotHV;\n\
	float att;\n\
	vec4 color = vec4(ambient,1.0);	\n\
	vec3 newtv = normalize(tv - bv*dot(tv,bv));\n\
	vec3 newbv = normalize(bv);\n\
	vec3 newn  = normalize(cross(tv,bv));\n\
	vec4 texColor = texture2D(diffuseTexture,gl_TexCoord[0].st);\n\
	vec3 normalColor = normalize(texture2D(normalTexture,gl_TexCoord[0].st).xyz* 2.0 - 1.0);\n\
	vec3 normalMapN = normalize(newtv*normalColor.x+newbv*normalColor.y+newn*normalColor.z); \n\
	if (useTexture == 0) \n\
		texColor = vec4(diffuseMaterial,1.0);//vec4(matColor,1.0);\n\
	color.a = texColor.a;\n\
	n = normalize(normal);\n\
	if (useNormalMap == 1)\n\
		n = normalMapN;\n\
	for (int i=0;i<2;i++)\n\
	{\n\
		att = 1.0/(gl_LightSource[i].constantAttenuation + gl_LightSource[i].linearAttenuation * dist[i] + gl_LightSource[i].quadraticAttenuation * dist[i] * dist[i]);	\n\
		NdotL = max(dot(n,lightDir[i]),0.0);\n\
		if (NdotL > 0.0) {\n\
			color += vec4(texColor.xyz*NdotL,0)*att;\n\
			halfV = normalize(halfVector[i]);\n\
			NdotHV = max(dot(n,halfV),0.0);\n\
			color += vec4(specularMaterial*pow(NdotHV, shineness+1.0),0)*att;\n\
		}   \n\
	}\n\
	gl_FragColor = color;\n\
}";


typedef std::pair<int,int> IntPair;

SbmDeformableMeshGPU::SbmDeformableMeshGPU(void)
{
	useGPU = false;
	numTotalVtxs = 0;
	numTotalTris = 0;
}

SbmDeformableMeshGPU::~SbmDeformableMeshGPU(void)
{

}

void SbmDeformableMeshGPU::skinTransformGPU()
{
	GLuint program = SbmShaderManager::singleton().getShader(shaderName.c_str())->getShaderProgram();

	updateTransformBuffer();
	glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );	
 	
	glDisable(GL_POLYGON_SMOOTH);
	
 	glEnable ( GL_ALPHA_TEST );
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc ( GL_GREATER, 0.3f ) ;

	glEnable(GL_TEXTURE);
	glDisable(GL_COLOR_MATERIAL);
	glUseProgram(program);	

	int iActiveTex = 0;	
 	
	GLuint diffuse_sampler_location = glGetUniformLocation(program,"diffuseTexture");	
	GLuint normal_sampler_location = glGetUniformLocation(program,"normalTexture");	
	GLuint bone_loc1 = glGetAttribLocation(program,"BoneID1");
	GLuint weight_loc1 = glGetAttribLocation(program,"BoneWeight1");	
	GLuint bone_loc2 = glGetAttribLocation(program,"BoneID2");
	GLuint weight_loc2 = glGetAttribLocation(program,"BoneWeight2");
	GLuint tangent_loc = glGetAttribLocation(program,"tangent");
	GLuint binormal_loc = glGetAttribLocation(program,"binormal");
	GLuint diffuseLoc = glGetUniformLocation(program,"diffuseMaterial");	
	GLuint specularLoc = glGetUniformLocation(program,"specularMaterial");	
	GLuint shinenessLoc = glGetUniformLocation(program,"shineness");
	GLuint useTextureLoc = glGetUniformLocation(program,"useTexture");
	GLuint useNormalMapLoc = glGetUniformLocation(program,"useNormalMap");

	GLuint idQuery;
	GLuint count = 0;
	glGenQueries(1, &idQuery);

	if (SbmShaderManager::getShaderSupport() == SbmShaderManager::SUPPORT_OPENGL_3_0)
	{
		GLuint my_sampler_uniform_location = glGetUniformLocation(program,"Transform");	
		TBOTran->UpdateTBOData((float*)getPtr(transformBuffer));
 		glActiveTexture(GL_TEXTURE0_ARB);
 		TBOTran->BindBufferToTexture();
 		glUniform1i(my_sampler_uniform_location, iActiveTex);
	}
	else
	{
		GLuint transformLocation = glGetUniformLocation(program,"Transform");
		glUniformMatrix4fv(transformLocation,transformBuffer.size(),true,(GLfloat*)getPtr(transformBuffer));
	}
	glEnableClientState(GL_VERTEX_ARRAY);	
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	VBOPos->VBO()->BindBuffer();
	//SbmShaderProgram::printOglError("after bind buffer\n");
	glVertexPointer( 4, GL_FLOAT, 0, 0);
	//SbmShaderProgram::printOglError("after vertex pointer\n");
  	
	glEnableVertexAttribArray(weight_loc1);
   	VBOWeight1->VBO()->BindBuffer();
	glVertexAttribPointer(weight_loc1,4,GL_FLOAT,0,0,0);	
	//glBindAttribLocation(program,VBOWeight1->VBO()->m_ArrayType,"BoneWeight1");
   	//glVertexAttribPointer(VBOWeight1->VBO()->m_ArrayType,4,GL_FLOAT,0,0,0);	

	glEnableVertexAttribArray(bone_loc1);
	VBOBoneID1->VBO()->BindBuffer();
	glVertexAttribPointer(bone_loc1,4,GL_FLOAT,0,0,0);	
	//glBindAttribLocation(program,VBOBoneID1->VBO()->m_ArrayType,"BoneID1");
	//glVertexAttribPointer(VBOBoneID1->VBO()->m_ArrayType,4,GL_FLOAT,0,0,0);	

	glEnableVertexAttribArray(weight_loc2);
	VBOWeight2->VBO()->BindBuffer();
	glVertexAttribPointer(weight_loc2,4,GL_FLOAT,0,0,0);	
	//glBindAttribLocation(program,VBOWeight2->VBO()->m_ArrayType,"BoneWeight2");
	//glVertexAttribPointer(VBOWeight2->VBO()->m_ArrayType,4,GL_FLOAT,0,0,0);
// 
	glEnableVertexAttribArray(bone_loc2);
	VBOBoneID2->VBO()->BindBuffer();
	glVertexAttribPointer(bone_loc2,4,GL_FLOAT,0,0,0);	

	glEnableVertexAttribArray(tangent_loc);
	VBOTangent->VBO()->BindBuffer();
	glVertexAttribPointer(tangent_loc,3,GL_FLOAT,0,0,0);	

	glEnableVertexAttribArray(binormal_loc);
	VBOBiNormal->VBO()->BindBuffer();
	glVertexAttribPointer(binormal_loc,3,GL_FLOAT,0,0,0);	

	SbmTextureManager& texManager = SbmTextureManager::singleton();
	VBONormal->VBO()->BindBuffer();
	glNormalPointer(GL_FLOAT,0,0);
	VBOTexCoord->VBO()->BindBuffer();
	glTexCoordPointer(3,GL_FLOAT,0,0);
	for (unsigned int i=0;i<meshSubset.size();i++)
	{	
		MeshSubset* mesh = meshSubset[i];
		float color[4];
		mesh->material.diffuse.get(color);		
		glUniform3f(diffuseLoc,color[0],color[1],color[2]);
		mesh->material.specular.get(color);		
		glUniform3f(specularLoc,color[0],color[1],color[2]);	
		//printf("shineness = %d\n",mesh->material.shininess);
		glUniform1f(shinenessLoc,mesh->material.shininess);
		//LOG("mat color = %f %f %f\n",color[0],color[1],color[2]);
		SbmTexture* tex = texManager.findTexture(SbmTextureManager::TEXTURE_DIFFUSE,mesh->texName.c_str());
		if (tex)
		{
			glActiveTexture(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D,tex->getID());
			glUniform1i(diffuse_sampler_location, 1);
			glUniform1i(useTextureLoc,1);
		}		
		else
		{
			glUniform1i(useTextureLoc,0);
		}

		SbmTexture* texNormal = texManager.findTexture(SbmTextureManager::TEXTURE_NORMALMAP,mesh->normalMapName.c_str());		
		if (texNormal)
		{
			glActiveTexture(GL_TEXTURE2_ARB);
			glBindTexture(GL_TEXTURE_2D,texNormal->getID());
			glUniform1i(normal_sampler_location, 2);
			glUniform1i(useNormalMapLoc,1);
		}		
		else
		{
			glUniform1i(useNormalMapLoc,0);
		}
		mesh->VBOTri->VBO()->BindBuffer();
		glDrawElements(GL_TRIANGLES,3*mesh->numTri,GL_UNSIGNED_INT,0);
		mesh->VBOTri->VBO()->UnbindBuffer();
		glBindTexture(GL_TEXTURE_2D,0);
	}	

	VBOTexCoord->VBO()->UnbindBuffer();
	VBONormal->VBO()->UnbindBuffer();
	VBOBiNormal->VBO()->UnbindBuffer();
	VBOTangent->VBO()->UnbindBuffer();
 	VBOBoneID2->VBO()->UnbindBuffer();
 	VBOWeight2->VBO()->UnbindBuffer();
 	VBOBoneID1->VBO()->UnbindBuffer();
 	VBOWeight1->VBO()->UnbindBuffer();
	VBOPos->VBO()->UnbindBuffer();

 	glUseProgram(0);		
	glDisable(GL_TEXTURE);
	glDisable(GL_ALPHA_TEST) ;
	glDisable(GL_BLEND);
}

void SbmDeformableMeshGPU::initShaderProgram()
{
	std::string vsPathName = ShaderDir;
	vsPathName += VSName;

	std::string fsPathName = ShaderDir;
	fsPathName += FSName;

	initShader = true;
	//shaderProgram.initShaderProgram(vsPathName.c_str(),NULL);	
	//SbmShaderManager::singleton().addShader(shaderName.c_str(),vsPathName.c_str(),fsPathName.c_str());
	if (SbmShaderManager::getShaderSupport() == SbmShaderManager::SUPPORT_OPENGL_3_0)
	{
		SbmShaderManager::singleton().addShader(shaderName.c_str(),shaderVS.c_str(),shaderFS.c_str(),false);
	}
	else if (SbmShaderManager::getShaderSupport() == SbmShaderManager::SUPPORT_OPENGL_2_0)
	{
		SbmShaderManager::singleton().addShader(shaderName.c_str(),shaderVS_2.c_str(),shaderFS.c_str(),false);
	}
	else
	{
		initShader = false;	
	}
}



bool SbmDeformableMeshGPU::initBuffer()
{
	// feng : the CPU version of deformable mesh consists of some mesh segments, with their corresponding bone weights loosely stored.
	// this is very bad for GPU processing. Thus I reorganize the data into a single array, to avoid redundancy in memory storage.

	if (skinWeights.size() == 0 )
		return false;

	int nTotalVtxs=0, nTotalTris = 0, nTotalBones = 0;
	std::map<std::string,int> boneIdxMap;
	std::vector<std::set<IntPair> > vtxNormalIdxMap;
	std::vector<std::set<int> > vtxMaterialIdxMap;
	std::map<IntPair,int> ntNewVtxIdxMap;	
	std::map<int,std::vector<int> > vtxNewVtxIdxMap;
	std::map<int,std::vector<int> > meshSubsetMap;

	GLuint program = SbmShaderManager::singleton().getShader(shaderName.c_str())->getShaderProgram();	
	
	SrColor colorArray[6] = { SrColor::blue, SrColor::red, SrColor::green, SrColor::magenta, SrColor::gray, SrColor::yellow};
	// setup deformable mesh color	
	int nMaterial = 1;	
	std::vector<SrMaterial> allMatList;
	//std::vector<SrColor> allMatList;
	std::vector<std::string> allTexNameList;
	std::vector<std::string> allNormalTexNameList;
	SrMaterial defaultMaterial;
	defaultMaterial.diffuse = SrColor(0.6f,0.6f,0.6f);//SrColor::gray;	
	defaultMaterial.specular = SrColor(101,101,101);//SrColor::gray;
	defaultMaterial.shininess = 29;

	allMatList.push_back(defaultMaterial);
	allTexNameList.push_back("");
	allNormalTexNameList.push_back("");
	meshSubsetMap[0] = std::vector<int>(); // default material group : gray color
	for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
	{
		SkinWeight* skinWeight = skinWeights[skinCounter];		
		int pos;
		int globalCounter = 0;
		pos = this->getMesh(skinWeight->sourceMesh);
		if (pos != -1)
		{
			SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
			SrSnModel* dMeshStatic = dMeshStatic_p[pos];
			dMeshStatic->shape().computeTangentBiNormal();
			SrArray<SrMaterial>& matList = dMeshDynamic->shape().M; 	
			std::map<std::string,std::string> mtlTexMap = dMeshDynamic->shape().mtlTextureNameMap;
			std::map<std::string,std::string> mtlNormalTexMap = dMeshDynamic->shape().mtlNormalTexNameMap;
			for (int j=0;j<matList.size();j++)
			{			
				SrMaterial& mat = matList[j];	
				std::string mtlName = dMeshDynamic->shape().mtlnames[j];
				if (mtlTexMap.find(mtlName) != mtlTexMap.end())
				{
					allTexNameList.push_back(mtlTexMap[mtlName]);
				}
				else
				{
					allTexNameList.push_back("");
				}	

				if (mtlNormalTexMap.find(mtlName) != mtlNormalTexMap.end())
				{
					allNormalTexNameList.push_back(mtlNormalTexMap[mtlName]);
				}
				else
				{
					allNormalTexNameList.push_back("");
				}
				allMatList.push_back(mat);
				//colorArray[j%6].get(fcolor);				
				meshSubsetMap[nMaterial] = std::vector<int>(); 
				nMaterial++;
			}
		}				
	}	

	//printf("num of mesh subset =  %d\n",meshSubsetMap.size());

	int iMaterialOffset = 1;
	boneJointList.clear();
	int iFaceIdxOffset = 0, iNormalIdxOffset = 0, iTextureIdxOffset = 0;
	int iFace = 0;
	SrModel::Face defaultIdx;
	defaultIdx.a = defaultIdx.b = defaultIdx.c = -1;
	for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
	{
		SkinWeight* skinWeight = skinWeights[skinCounter];		
		int pos;
		int globalCounter = 0;
		pos = this->getMesh(skinWeight->sourceMesh);
		if (pos != -1)
		{
			SrSnModel* dMeshStatic = dMeshStatic_p[pos];
			SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];			
			int nVtx = dMeshStatic->shape().V.size();	
			int nFace = dMeshStatic->shape().F.size();
			int nNormal = dMeshStatic->shape().N.size();	
			int nTexture = dMeshStatic->shape().T.size();
			for (int i=0;i<nVtx;i++)
			{
				vtxNormalIdxMap.push_back(std::set<IntPair>());				
			}

			nTotalVtxs += nVtx;				
			nTotalTris += nFace;
			for (unsigned int k=0;k<skinWeight->infJointName.size();k++)
			{
				std::string& jointName = skinWeight->infJointName[k];
				SkJoint* curJoint = skinWeight->infJoint[k];
				if (boneIdxMap.find(jointName) == boneIdxMap.end()) // new joint
				{
					boneIdxMap[jointName] = nTotalBones++;
					boneJointList.push_back(curJoint);
					bindPoseMatList.push_back(skinWeight->bindPoseMat[k]);
				}
			}
			int numTris = dMeshStatic->shape().F.size();
			for (int i=0; i < numTris ; i++)
			{
				SrModel& model = dMeshStatic->shape();
				if (dMeshStatic->shape().F.size() == 0)
					continue;
				SrModel::Face& faceIdx = dMeshStatic->shape().F[i];			
				if (dMeshStatic->shape().Fn.size() == 0)
					continue;
				SrModel::Face& nIdx = dMeshStatic->shape().Fn[i];
				SrModel::Face& tIdx = defaultIdx;
				if (model.Ft.size() > i)
					tIdx = model.Ft[i];
								
				vtxNormalIdxMap[faceIdx.a + iFaceIdxOffset].insert(IntPair(nIdx.a+iNormalIdxOffset,tIdx.a+iTextureIdxOffset));
				vtxNormalIdxMap[faceIdx.b + iFaceIdxOffset].insert(IntPair(nIdx.b+iNormalIdxOffset,tIdx.b+iTextureIdxOffset));
				vtxNormalIdxMap[faceIdx.c + iFaceIdxOffset].insert(IntPair(nIdx.c+iNormalIdxOffset,tIdx.c+iTextureIdxOffset));
				
				int nMatIdx = 0; // if no corresponding materials, push into the default gray material group
				if (i < dMeshStatic->shape().Fm.size())
					nMatIdx = dMeshStatic->shape().Fm[i] + iMaterialOffset;		
				meshSubsetMap[nMatIdx].push_back(iFace);			
				iFace++;
			}
			iFaceIdxOffset += nVtx;
			iNormalIdxOffset += nNormal;
			iMaterialOffset += dMeshDynamic->shape().M.size();
			iTextureIdxOffset += nTexture;
			//printf("iMaterial Offset = %d\n",iMaterialOffset);
		}			
	}

	if (nTotalVtxs == 0 || nTotalTris ==0)
		return false;

	//printf("orig vtxs = %d\n",nTotalVtxs);

	for (unsigned int i=0;i<vtxNormalIdxMap.size();i++)
	{
		std::set<IntPair>& idxMap = vtxNormalIdxMap[i];
		if (idxMap.size() > 1)
		{
			vtxNewVtxIdxMap[i] = std::vector<int>();
			std::set<IntPair>::iterator si = idxMap.begin();
			si++;
			for ( ;
				  si != idxMap.end();
				  si++)
			{
				vtxNewVtxIdxMap[i].push_back(nTotalVtxs);
				ntNewVtxIdxMap[*si] = nTotalVtxs;
				nTotalVtxs++;
			}
		}
	}

	//printf("new vtxs = %d\n",nTotalVtxs);

	// temporary storage 
	ublas::vector<Vec4f> posBuffer(nTotalVtxs);
	ublas::vector<Vec3f> normalBuffer(nTotalVtxs),texCoordBuffer(nTotalVtxs), tangentBuffer(nTotalVtxs), binormalBuffer(nTotalVtxs);
	ublas::vector<Vec3i> triBuffer(nTotalTris);
	ublas::vector<Vec4f> boneID1(nTotalVtxs),boneID2(nTotalVtxs),weight1(nTotalVtxs),weight2(nTotalVtxs);
	transformBuffer.resize(nTotalBones);	
	
	int iVtx = 0;
	iFace = 0;
	iFaceIdxOffset = 0;
	iNormalIdxOffset = 0;
	iTextureIdxOffset = 0;
	for (unsigned int skinCounter = 0; skinCounter < skinWeights.size(); skinCounter++)
	{
		SkinWeight* skinWeight = skinWeights[skinCounter];		
		int pos;
		int globalCounter = 0;
		pos = this->getMesh(skinWeight->sourceMesh);
		if (pos != -1)
		{
			SrSnModel* dMeshStatic = dMeshStatic_p[pos];
			SrSnModel* dMeshDynamic = dMeshDynamic_p[pos];
			dMeshDynamic->visible(false);
			int numVertices = dMeshStatic->shape().V.size();
			int numNormals = dMeshStatic->shape().N.size();
			int numTexCoords = dMeshStatic->shape().T.size();
			for (int i = 0; i < numVertices; i++)
			{
				if (i >= (int) skinWeight->numInfJoints.size())
					continue;
				int numOfInfJoints = skinWeight->numInfJoints[i];				
				SrVec& lv = dMeshStatic->shape().V[i];					
				posBuffer(iVtx) = Vec4f(lv[0],lv[1],lv[2],1.f);

				SrVec& lt =	dMeshStatic->shape().Tangent[i];		
				SrVec  lb = dMeshStatic->shape().BiNormal[i];
				tangentBuffer(iVtx) = Vec3f(lt[0],lt[1],lt[2]);
				binormalBuffer(iVtx) = Vec3f(lb[0],lb[1],lb[2]);
				//normalBuffer(iVtx) = Vec3f(ln[0],ln[1],ln[2]);
				boneID1(iVtx) = Vec4f(0,0,0,0);
				boneID2(iVtx) = Vec4f(0,0,0,0);
				weight1(iVtx) = Vec4f(0,0,0,0);
				weight2(iVtx) = Vec4f(0,0,0,0);
				
				std::vector<IntFloatPair> weightList;
// 				if (numOfInfJoints > 8)
// 					printf("vtx %d : \n",iVtx);
				for (int j = 0; j < numOfInfJoints; j++)
				{
					const std::string curJointName = skinWeight->infJointName[skinWeight->jointNameIndex[globalCounter]];					
					float jointWeight = skinWeight->bindWeight[skinWeight->weightIndex[globalCounter]];
					int    jointIndex  = boneIdxMap[curJointName];
// 					if (numOfInfJoints > 8)
// 						printf("w = %d : %f\n",jointIndex,jointWeight);
					weightList.push_back(IntFloatPair(jointIndex,jointWeight));							
					globalCounter ++;									
				}
				std::sort(weightList.begin(),weightList.end(),intFloatComp);				
				int numWeight = numOfInfJoints > 8 ? 8 : numOfInfJoints;
				float weightSum = 0.f;
				for (int j=0;j<numWeight;j++)
				{
					IntFloatPair& w = weightList[j];
					weightSum += w.second;
					if ( j < 4)
					{
						boneID1(iVtx)[j] = (float)w.first;
						weight1(iVtx)[j] = w.second;
					}
					else if (j < 8)
					{
						boneID2(iVtx)[j-4] = (float)w.first;
						weight2(iVtx)[j-4] = w.second;
					}	
				}
				for (int j=0;j<4;j++)
				{
					weight1(iVtx)[j] /= weightSum;
					weight2(iVtx)[j] /= weightSum;
				}	
				
				if (vtxNewVtxIdxMap.find(iVtx) != vtxNewVtxIdxMap.end())
				{
					std::vector<int>& idxMap = vtxNewVtxIdxMap[iVtx];
					// copy related vtx components 
					for (unsigned int k=0;k<idxMap.size();k++)
					{
						posBuffer(idxMap[k]) = posBuffer(iVtx);
 						tangentBuffer(idxMap[k]) = tangentBuffer(iVtx);
 						binormalBuffer(idxMap[k]) = binormalBuffer(iVtx);
						boneID1(idxMap[k]) = boneID1(iVtx);
						boneID2(idxMap[k]) = boneID2(iVtx);
						weight1(idxMap[k]) = weight1(iVtx);
						weight2(idxMap[k]) = weight2(iVtx);
					}
				}
				iVtx++;
			}

			int numTris = dMeshStatic->shape().F.size();
			for (int i=0; i < numTris ; i++)
			{
				if (dMeshStatic->shape().F.size() <= i)
					continue;
				if (dMeshStatic->shape().Fn.size() <= i)
					continue;
				SrModel::Face& faceIdx = dMeshStatic->shape().F[i];
				SrModel::Face& normalIdx = dMeshStatic->shape().Fn[i];
				SrModel::Face& texCoordIdx = defaultIdx;
				if (dMeshStatic->shape().Ft.size() > i)
					texCoordIdx = dMeshStatic->shape().Ft[i];
				int fIdx[3] = { faceIdx.a, faceIdx.b, faceIdx.c};
				int nIdx[3] = { normalIdx.a, normalIdx.b, normalIdx.c};
				int tIdx[3] = { texCoordIdx.a, texCoordIdx.b, texCoordIdx.c};

				for (int k=0;k<3;k++)
				{
					SrVec nvec;
					SrPnt2 tvec = SrPnt2(0,0);
					nvec = dMeshStatic->shape().N[nIdx[k]];
					if (dMeshStatic->shape().T.size() > tIdx[k])
						tvec = dMeshStatic->shape().T[tIdx[k]];
					int newNIdx = nIdx[k] + iNormalIdxOffset;
					int newTIdx = tIdx[k] + iTextureIdxOffset;
					int vIdx = fIdx[k] + iFaceIdxOffset;
					if (ntNewVtxIdxMap.find(IntPair(newNIdx,newTIdx)) != ntNewVtxIdxMap.end())
						vIdx = ntNewVtxIdxMap[IntPair(newNIdx,newTIdx)];
					normalBuffer(vIdx) = Vec3f(nvec.x,nvec.y,nvec.z);
					texCoordBuffer(vIdx) = Vec3f(tvec.x,1.f-tvec.y,0.f);
					triBuffer(iFace)[k] = vIdx;
				}			
				iFace++;
			}
			iFaceIdxOffset += numVertices;
			iNormalIdxOffset += numNormals;
			iTextureIdxOffset += numTexCoords;
		}			
	}		

	numTotalVtxs = iVtx;
	numTotalTris = iFace;

	std::map<int,std::vector<int> >::iterator vi;
	for (vi  = meshSubsetMap.begin();
		 vi != meshSubsetMap.end();
		 vi++)
	{
		int iMaterial = vi->first;

		std::vector<int>& faceIdxList = vi->second;	
		if (faceIdxList.size() == 0)
			continue;
		ublas::vector<Vec3i> subsetBuffer(faceIdxList.size());
		for (unsigned int k=0;k<faceIdxList.size();k++)
		{
			subsetBuffer(k) = triBuffer(faceIdxList[k]);
		}
		MeshSubset* mesh = new MeshSubset();
		mesh->material = allMatList[iMaterial];
		mesh->texName  = allTexNameList[iMaterial];
		mesh->normalMapName = allNormalTexNameList[iMaterial];
		mesh->VBOTri = new VBOVec3i((char*)"TriIdx",GL_ELEMENT_ARRAY_BUFFER,subsetBuffer);
		mesh->numTri = faceIdxList.size();
		meshSubset.push_back(mesh);
	}
	// initial GPU buffer memory
	// Vertex Buffer Object	
	VBOPos = new VBOVec4f((char*)"RestPos",VERTEX_POSITION,posBuffer);		
	VBOTangent = new VBOVec3f((char*)"Tangent",VERTEX_TANGENT, tangentBuffer);
	VBOBiNormal = new VBOVec3f((char*)"BiNormal",VERTEX_BINORMAL, binormalBuffer);
	VBONormal  =  new VBOVec3f((char*)"Normal",VERTEX_VBONORMAL, normalBuffer);
	VBOTexCoord = new VBOVec3f((char*)"TexCoord",VERTEX_TEXCOORD, texCoordBuffer);
	VBOWeight1 = new VBOVec4f((char*)"Weight1",VERTEX_BONE_WEIGHT_1,weight1);
	VBOWeight2 = new VBOVec4f((char*)"Weight2",VERTEX_BONE_WEIGHT_2,weight2);
	//VBOOutPos  = new VBOVec4f((char*)"OutPos",VERTEX_POSITION,posBuffer);
	VBOTri     = new VBOVec3i((char*)"TriIdx",GL_ELEMENT_ARRAY_BUFFER,triBuffer);
	VBOBoneID1 = new VBOVec4f((char*)"BoneID1",VERTEX_BONE_ID_1,boneID1);
	VBOBoneID2 = new VBOVec4f((char*)"BoneID2",VERTEX_BONE_ID_2,boneID2);

	// Texture Buffer Object
	if (SbmShaderManager::getShaderSupport() == SbmShaderManager::SUPPORT_OPENGL_3_0)
	{
		int tranSize = boneJointList.size()*16;
		int colorSize = DeformableMesh::dMeshDynamic_p.size()*3;
		TBOTran    = new TBOData((char*)"BoneTran",tranSize); 
	}
	return true;
}

void SbmDeformableMeshGPU::update()
{	
//#define USE_GPU_TRANSFORM 1
//#if USE_GPU_TRANSFORM
	if (!useGPUDeformableMesh)
	{
		DeformableMesh::update();
		return;
	}

	if (!binding)	return; 

	if (!initShader)
		initShaderProgram();

	SbmShaderProgram* program = SbmShaderManager::singleton().getShader(shaderName.c_str());	
	bool hasGLContext = SbmShaderManager::singleton().initOpenGL() && SbmShaderManager::singleton().initGLExtension();
	if (!useGPU && hasGLContext && program && program->finishBuild())
	{
		// initialize 
		useGPU = initBuffer();
	}

	if (!useGPU || !hasGLContext)
	{	
		for (unsigned int i=0;i<dMeshDynamic_p.size();i++)
		{
			dMeshDynamic_p[i]->visible(true);
		}
		DeformableMesh::update();		
	}	
	else
	{
		// GPU update and rendering
		//printf("GPU Deformable Model Update\n");
		for (unsigned int i=0;i<dMeshDynamic_p.size();i++)
		{
			dMeshDynamic_p[i]->visible(false);
		}
		skeleton->update_global_matrices();
		skinTransformGPU();
	}
//#else
	
//#endif
}

void SbmDeformableMeshGPU::updateTransformBuffer()
{
	if (transformBuffer.size() != boneJointList.size())
		transformBuffer.resize(boneJointList.size());
	for (unsigned int i=0;i<boneJointList.size();i++)
	{
		SkJoint* joint = boneJointList[i];
		if (!joint)
			continue;
		transformBuffer(i) = bindPoseMatList[i]*joint->gmat();		
	}
}
