#version 430
#define MAX_LIGHTS 20
layout(binding=0) uniform sampler2D uNormalTex;
layout(binding=1) uniform sampler2D uEnvLightTex;
layout(binding=2) uniform sampler2D uEnvDiffuseTex;
layout(binding=3) uniform sampler2D uDiffuseTex;
layout(binding=4) uniform sampler2D uSSAOTex;

varying vec2 texCoord;
varying vec3 viewRay;
uniform mat4 uViewToWorldMatrix;

layout(location=0) out vec4 fResult;

const float PI = 3.1415926535897932384626433832795;
const float TwoPI = 3.1415926535897932384626433832795*2.0;

vec2 envMapEquirect(vec3 wcNormal, float flipEnvMap) {
  //I assume envMap texture has been flipped the WebGL way (pixel 0,0 is a the bottom)
  //therefore we flip wcNorma.y as acos(1) = 0
  float phi = acos(-wcNormal.y);
  float theta = atan(flipEnvMap * wcNormal.x, wcNormal.z) + PI;
  return vec2(theta / TwoPI, phi / PI);
}

vec2 envMapEquirect(vec3 wcNormal) {
    //-1.0 for left handed coordinate system oriented texture (usual case)
    return envMapEquirect(wcNormal, -1.0);
}

/*----------------------------------------------------------------------------*/
void main() {	
	
	//vec3 normal = decodeNormal(texture(uNormalTex, texCoord).rgb);	
	vec3 normal = texture(uNormalTex, texCoord).rgb;	

	if (length(normal) == 0.0)
	{
		vec3 wray = normalize(mat3(uViewToWorldMatrix)*viewRay);
		vec3 lcolor = texture(uEnvLightTex, envMapEquirect(wray)).rgb;
		fResult = vec4(lcolor, 1.0);
	}
	else
	{
		vec3 wnormal = normalize(mat3(uViewToWorldMatrix)*normal);
		vec3 lcolor = texture(uEnvDiffuseTex, envMapEquirect(wnormal)).rgb;	
		//vec3 envMapColor = texture(uEnvDiffuseTex, texCoord).rgb;
		vec4 diffuseColor = texture(uDiffuseTex, texCoord);
		float ssaoVal = texture(uSSAOTex, texCoord).r;

		//fResult = vec4(envMapColor, 1.0);
		//fResult = vec4(lcolor/PI*diffuseColor.rgb, 1.0);
		fResult = vec4(lcolor*diffuseColor.rgb*ssaoVal, 1.0);		
		//fResult = vec4(lcolor*ssaoVal, 1.0);
	}
	
	//fResult = vec4(1,0,0,1);
	//fResult = vec4(wnormal, 1.0);
	//fResult = vec4(lcolor*diffuseColor.xyz*ssaoVal, 1.f);
	//fResult = vec4(lcolor, 1.0);
}