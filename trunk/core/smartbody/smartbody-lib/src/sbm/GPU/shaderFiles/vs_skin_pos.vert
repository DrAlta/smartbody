#extension GL_EXT_gpu_shader4 : enable
//#extension NV_vertex_program4 : enable
uniform samplerBuffer Offset;
uniform samplerBuffer Transform;
attribute vec4 BoneID;
attribute vec4 BoneWeight;
varying vec3 OutPos;

// fetch transformation matrix
mat3 GetTransformation(float id)
{
	mat3 rot;
	for (int i=0;i<3;i++)
	{		
		for (int j=0;j<3;j++)
			rot[j][i] = texelFetchBuffer(Transform,(int)(id*9+i*3+j)).x;		
	}	
	return rot;
}

vec3 GetTranslation(float id)
{
	vec3 tran;
	tran[0] = texelFetchBuffer(Offset,(int)(id*3+0)).x;
	tran[1] = texelFetchBuffer(Offset,(int)(id*3+1)).x;
	tran[2] = texelFetchBuffer(Offset,(int)(id*3+2)).x;
	return tran;	
}

vec3 TransformPos(vec3 position, float4 boneid, float4 boneweight)
{
	vec3 pos = 0;
	mat3 tempT;	
	vec3 tempt;	
	for (int i=0;i<4;i++)
	{
		tempT = GetTransformation(boneid[i]);
		tempt = GetTranslation(boneid[i]);
		pos += (mul(tempT,position)+tempt)*boneweight[i]; 		
	}	
	//tempT = GetTransformation(50);
	//pos = tempT[0].xyz;
	return pos;
}

void main()
{	
	// the following three lines provide the same result
	vec3 pos = vec3(gl_Vertex.xyz);
	vec3 tranPos = TransformPos(pos,BoneID,BoneWeight);	
	//gl_Position = vec4(tranPos,1.0);
	OutPos = tranPos.xyz;	
}