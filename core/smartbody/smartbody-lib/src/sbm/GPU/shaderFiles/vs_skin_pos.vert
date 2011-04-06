#extension GL_EXT_gpu_shader4 : enable
uniform samplerBuffer Transform;
attribute vec4 BoneID1,BoneID2;
attribute vec4 BoneWeight1,BoneWeight2;
//varying vec3 OutPos;
varying vec3 normal,lightDir,halfVector;

//fetch transformation matrix
mat3 GetTransformation(float id)
{
	mat3 rot;
	for (int i=0;i<3;i++)
	{		
		for (int j=0;j<3;j++)
			rot[j][i] = texelFetchBuffer(Transform,(int)(id*16+i*4+j)).x;		
	}	
	return rot;
}

vec3 GetTranslation(float id)
{
	vec3 tran;
	tran[0] = texelFetchBuffer(Transform,(int)(id*16+12)).x;
	tran[1] = texelFetchBuffer(Transform,(int)(id*16+13)).x;
	tran[2] = texelFetchBuffer(Transform,(int)(id*16+14)).x;
	return tran;	
}
vec3 TransformPos(vec3 position, vec4 boneid, vec4 boneweight)
{
	vec3 pos = 0;
	mat3 tempT;	
	vec3 tempt;	
	for (int i=0;i<4;i++)
	{
		tempT = GetTransformation(boneid[i]);
		tempt = GetTranslation(boneid[i]);
		pos += (position*tempT+tempt)*boneweight[i]; 		
	}	
	//tempT = GetTransformation(50);
	//pos = tempT[0].xyz;
	return pos;
}
void main()
{	
	// the following three lines provide the same result
	vec3 pos = vec3(gl_Vertex.xyz);
    vec3 tranPos = TransformPos(pos,BoneID1,BoneWeight1) + TransformPos(pos,BoneID2,BoneWeight2);	
	//gl_Position = vec4(tranPos,1.0);
	gl_Position = gl_ModelViewProjectionMatrix*vec4(tranPos,1.0);//gl_Vertex;//vec4(pos,1.0);
	//OutPos = tranPos.xyz;	
	lightDir = normalize(vec3(gl_LightSource[0].position));
	halfVector = normalize(gl_LightSource[0].halfVector.xyz);	
	normal = normalize(gl_NormalMatrix * gl_Normal);
	
}