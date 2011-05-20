#include "SbmDeformableMeshGPU.h"
#include <string>
#include <set>
#include <algorithm>

typedef std::pair<int,float> IntFloatPair;

static bool intFloatComp(IntFloatPair& p1, IntFloatPair& p2)
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
"#extension GL_EXT_gpu_shader4 : enable \n\
uniform samplerBuffer Transform; \n\
attribute vec4 BoneID1,BoneID2;   \n\
attribute vec4 BoneWeight1,BoneWeight2;\n \
varying vec3 normal,lightDir[2],halfVector[2];\n\
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
mat3 TransformPos(vec3 position, vec3 normal, vec4 boneid, vec4 boneweight)\n\
{\n\
	vec3 pos = vec3(0,0,0);\n\
	vec3 n = vec3(0,0,0);\n\
	mat3 tempT;\n\
	vec3 tempt;\n\
	for (int i=0;i<4;i++)\n\
	{\n\
		tempT = GetTransformation(boneid[i]);\n\
		tempt = GetTranslation(boneid[i]);\n\
		pos += (position*tempT+tempt)*boneweight[i]; 		\n\
		n   += (normal*tempT)*boneweight[i];\n\
	}	\n\
	mat3 result;\n\
	result[0] = pos;\n\
	result[1] = n;\n\
	return result;\n\
}\n\
void main()\n \
{\n	\
	// the following three lines provide the same result\n \
	vec3 pos = vec3(gl_Vertex.xyz);\n \
	mat3 skin = TransformPos(pos,gl_Normal,BoneID1,BoneWeight1) + TransformPos(pos,gl_Normal,BoneID2,BoneWeight2);\n\
	gl_Position = gl_ModelViewProjectionMatrix*vec4(skin[0],1.0);\n\
	lightDir[0] = normalize(vec3(gl_LightSource[0].position));\n\
	halfVector[0] = normalize(gl_LightSource[0].halfVector.xyz);\n\
	dist[0] = 0.0;\n\
	vec3 posDir = vec3(gl_LightSource[1].position - gl_ModelViewMatrix * vec4(skin[0],1.0));\n\
	dist[1] = length(posDir);\n\
	lightDir[1] = normalize(posDir);\n\
	halfVector[1] = normalize(gl_LightSource[1].halfVector.xyz);\n\
	normal = normalize(gl_NormalMatrix * skin[1]);\n\
}\n";

std::string shaderFS =
"const vec3 diffuseColor = vec3(1,1,1)*0.6;//vec3(255.f,252.f,181.f)/255.f*vec3(0.8,0.8,0.8)*0.8;\n\
const vec3 specularColor = vec3(101.0/255.0, 101.0/255.0, 101.0/255.0);\n\
const vec3 ambient = vec3(0.0,0.0,0.0);//(vec3(255 + 127, 241, 0 + 2)/255.f)*(vec3(0.2,0.2,0.2));\n\
varying vec3 normal,lightDir[2],halfVector[2];\n\
varying float dist[2];\n\
void main (void)\n\
{  \n\
	vec3 n,halfV;\n\
	float NdotL,NdotHV;\n\
	float att;\n\
	vec4 color = vec4(ambient,1.0);	\n\
	n = normalize(normal);	\n\
	for (int i=0;i<2;i++)\n\
	{\n\
		att = 1.0/(gl_LightSource[i].constantAttenuation + gl_LightSource[i].linearAttenuation * dist[i] + gl_LightSource[i].quadraticAttenuation * dist[i] * dist[i]);	\n\
		NdotL = max(dot(n,lightDir[i]),0.0);\n\
		if (NdotL > 0.0) {\n\
			color += vec4(diffuseColor*NdotL,0)*att;\n\
			halfV = normalize(halfVector[i]);\n\
			NdotHV = max(dot(n,halfV),0.0);\n\
			color += vec4(specularColor*pow(NdotHV, 30.0),0)*att;			\n\
		}   \n\
	}\n\
	gl_FragColor = color;\n\
}";

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
	glUseProgram(program);	

	int iActiveTex = 0;	
 	GLuint my_sampler_uniform_location = glGetUniformLocation(program,"Transform");	
	GLuint bone_loc1 = glGetAttribLocation(program,"BoneID1");
	GLuint weight_loc1 = glGetAttribLocation(program,"BoneWeight1");	
	GLuint bone_loc2 = glGetAttribLocation(program,"BoneID2");
	GLuint weight_loc2 = glGetAttribLocation(program,"BoneWeight2");

	GLuint idQuery;
	GLuint count = 0;
	glGenQueries(1, &idQuery);

    TBOTran->UpdateTBOData((float*)getPtr(transformBuffer));

 	glActiveTexture(GL_TEXTURE0_ARB);
 	TBOTran->BindBufferToTexture();
 	glUniform1i(my_sampler_uniform_location, iActiveTex);	

	glEnableClientState(GL_VERTEX_ARRAY);	
	glEnableClientState(GL_NORMAL_ARRAY);
	VBOPos->VBO()->BindBuffer();
	//SbmShaderProgram::printOglError("after bind buffer\n");
	glVertexPointer( 3, GL_FLOAT, 0, 0);
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
	//glBindAttribLocation(program,VBOBoneID2->VBO()->m_ArrayType,"BoneID2");
	//glVertexAttribPointer(VBOBoneID2->VBO()->m_ArrayType,4,GL_FLOAT,0,0,0);	
	

// 	glBindBufferOffsetNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, VBOOutPos->VBO()->m_iVBO_ID, 0);	
// 
// 	//SbmShaderProgram::printOglError("After feed back\n");
// 	glBeginTransformFeedbackNV(GL_POINTS);
// 	
// 	glEnable(GL_RASTERIZER_DISCARD_NV);
// 	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV, idQuery);

	//glDrawArrays(GL_POINTS,0,numTotalVtxs);
	VBONormal->VBO()->BindBuffer();
	glNormalPointer(GL_FLOAT,0,0);
	VBOTri->VBO()->BindBuffer();
	glDrawElements(GL_TRIANGLES,3*numTotalTris,GL_UNSIGNED_INT,0);
	VBOTri->VBO()->UnbindBuffer();
	VBONormal->VBO()->UnbindBuffer();
	

// 	SbmShaderProgram::printOglError("after draw array\n");
// 
// 	glDisable(GL_RASTERIZER_DISCARD_NV);
// 	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV);
// 	glEndTransformFeedbackNV();

	//m_pSkinVtx->Debug("TBO SkinVtx",100,9000);

 	VBOBoneID2->VBO()->UnbindBuffer();
 	VBOWeight2->VBO()->UnbindBuffer();
 	VBOBoneID1->VBO()->UnbindBuffer();
 	VBOWeight1->VBO()->UnbindBuffer();
	VBOPos->VBO()->UnbindBuffer();

// 	GLuint primitives_written;
// 	glGetQueryObjectuiv(idQuery, GL_QUERY_RESULT, &primitives_written);	
 	glUseProgram(0);	
// 	printf("Written Vertices = %u\n",primitives_written);
	
}

void SbmDeformableMeshGPU::initShaderProgram()
{
	std::string vsPathName = ShaderDir;
	vsPathName += VSName;

	std::string fsPathName = ShaderDir;
	fsPathName += FSName;
	//shaderProgram.initShaderProgram(vsPathName.c_str(),NULL);	
	//SbmShaderManager::singleton().addShader(shaderName.c_str(),vsPathName.c_str(),fsPathName.c_str());
	SbmShaderManager::singleton().addShader(shaderName.c_str(),shaderVS.c_str(),shaderFS.c_str(),false);
	initShader = true;
}

bool SbmDeformableMeshGPU::initBuffer()
{
	// feng : the CPU version of deformable mesh consists of some mesh segments, with their corresponding bone weights loosely stored.
	// this is very bad for GPU processing. Thus I reorganize the data into a single array, to avoid redundancy in memory storage.

	if (skinWeights.size() == 0 )
		return false;

	int nTotalVtxs=0, nTotalTris = 0, nTotalBones = 0;
	std::map<std::string,int> boneIdxMap;
	std::vector<std::set<int> > vtxNormalIdxMap;
	std::map<int,int> normalNewVtxIdxMap;
	std::map<int,std::vector<int> > vtxNewVtxIdxMap;

	boneJointList.clear();
	int iFaceIdxOffset = 0, iNormalIdxOffset = 0;
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
			for (int i=0;i<nVtx;i++)
				vtxNormalIdxMap.push_back(std::set<int>());

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
				SrModel::Face& faceIdx = dMeshStatic->shape().F[i];				
				SrModel::Face& nIdx = dMeshStatic->shape().Fn[i];
				vtxNormalIdxMap[faceIdx.a + iFaceIdxOffset].insert(nIdx.a+iNormalIdxOffset);
				vtxNormalIdxMap[faceIdx.b + iFaceIdxOffset].insert(nIdx.b+iNormalIdxOffset);
				vtxNormalIdxMap[faceIdx.c + iFaceIdxOffset].insert(nIdx.c+iNormalIdxOffset);
			}
			iFaceIdxOffset += nVtx;
			iNormalIdxOffset += nNormal;
		}			
	}

	if (nTotalVtxs == 0 || nTotalTris ==0)
		return false;

	//printf("orig vtxs = %d\n",nTotalVtxs);

	for (unsigned int i=0;i<vtxNormalIdxMap.size();i++)
	{
		std::set<int>& idxMap = vtxNormalIdxMap[i];
		if (idxMap.size() > 1)
		{
			vtxNewVtxIdxMap[i] = std::vector<int>();
			std::set<int>::iterator si = idxMap.begin();
			si++;
			for ( ;
				  si != idxMap.end();
				  si++)
			{
				vtxNewVtxIdxMap[i].push_back(nTotalVtxs);
				normalNewVtxIdxMap[*si] = nTotalVtxs;
				nTotalVtxs++;
			}
		}
	}

	//printf("new vtxs = %d\n",nTotalVtxs);

	// temporary storage 
	ublas::vector<Vec3f> posBuffer(nTotalVtxs), normalBuffer(nTotalVtxs);
	ublas::vector<Vec3i> triBuffer(nTotalTris);
	ublas::vector<Vec4f> boneID1(nTotalVtxs),boneID2(nTotalVtxs),weight1(nTotalVtxs),weight2(nTotalVtxs);
	transformBuffer.resize(nTotalBones);
	
	int iVtx = 0, iFace = 0;
	iFaceIdxOffset = 0;
	iNormalIdxOffset = 0;
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
			for (int i = 0; i < numVertices; i++)
			{
				int numOfInfJoints = skinWeight->numInfJoints[i];				
				SrVec& lv = dMeshStatic->shape().V[i];	
				//SrVec& ln = dMeshStatic->shape().N[i];
				posBuffer(iVtx) = Vec3f(lv[0],lv[1],lv[2]);
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
				SrModel::Face& faceIdx = dMeshStatic->shape().F[i];
				SrModel::Face& normalIdx = dMeshStatic->shape().Fn[i];
				int fIdx[3] = { faceIdx.a, faceIdx.b, faceIdx.c};
				int nIdx[3] = { normalIdx.a, normalIdx.b, normalIdx.c};

				for (int k=0;k<3;k++)
				{
					SrVec nvec;
					nvec = dMeshStatic->shape().N[nIdx[k]];
					int newIdx = nIdx[k] + iNormalIdxOffset;
					int vIdx = fIdx[k] + iFaceIdxOffset;
					if (normalNewVtxIdxMap.find(newIdx) != normalNewVtxIdxMap.end())
						vIdx = normalNewVtxIdxMap[newIdx];

					normalBuffer(vIdx) = Vec3f(nvec.x,nvec.y,nvec.z);
					triBuffer(iFace)[k] = vIdx;
				}			
				iFace++;
			}
			iFaceIdxOffset += numVertices;
			iNormalIdxOffset += numNormals;
		}			
	}	

	numTotalVtxs = iVtx;
	numTotalTris = iFace;
	// initial GPU buffer memory

	// Vertex Buffer Object	
	VBOPos = new VBOVec3f("RestPos",VERTEX_POSITION,posBuffer);	
	VBONormal  =  new VBOVec3f("Normal",VERTEX_VBONORMAL, normalBuffer);
	VBOWeight1 = new VBOVec4f("Weight1",VERTEX_BONE_WEIGHT_1,weight1);
	VBOWeight2 = new VBOVec4f("Weight2",VERTEX_BONE_WEIGHT_2,weight2);
	VBOOutPos  = new VBOVec3f("OutPos",VERTEX_POSITION,posBuffer);
	VBOTri     = new VBOVec3i("TriIdx",GL_ELEMENT_ARRAY_BUFFER,triBuffer);
	VBOBoneID1 = new VBOVec4f("BoneID1",VERTEX_BONE_ID_1,boneID1);
	VBOBoneID2 = new VBOVec4f("BoneID2",VERTEX_BONE_ID_2,boneID2);
	

	// Texture Buffer Object
	int tranSize = boneJointList.size()*16;
	TBOTran    = new TBOData("BoneTran",tranSize); 
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
		transformBuffer(i) = bindPoseMatList[i]*boneJointList[i]->gmat();		
	}
}