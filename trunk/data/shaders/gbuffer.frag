/*******************************************************************************
 * GBuffer.frag
 *------------------------------------------------------------------------------
 * Writes final information to the GBuffers.
 ******************************************************************************/
#version 430
//#include "normal.glsl"
layout(binding=0) uniform sampler2D uDiffuseTex;
layout(binding=1) uniform sampler2D uNormalTex;
layout(binding=2) uniform sampler2D uSpecularTex;

uniform float alphaThreshold = 0.8;

in vec4 vPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBiNormal;
in vec2 vTexCoord;

vec3 encodeNormal(in vec3 normal) {
	vec3 result;
	result = normal;
	result = result * 0.5 + 0.5;
	return result;
}

// Framebuffer Outputs
layout(location = 0) out vec3 viewPos;
layout(location = 1) out vec3 viewNormal;
layout(location = 2) out vec4 diffuse;
layout(location = 3) out vec4 specular;
layout(location = 4) out vec3 glossy;


void main()
{
  // Translate clip [-1,1] -> homogenous [0,1]
  vec4 texColor = texture(uDiffuseTex, vTexCoord);  
  if (texColor.a < alphaThreshold) discard;    
  vec3 normalColor = normalize(texture2D(uNormalTex,vTexCoord).xyz* 2.0 - 1.0);
  vec3 newtv = normalize(vTangent - vBiNormal*dot(vTangent,vBiNormal));
  vec3 newbv = normalize(vBiNormal);
  vec3 newn  = vNormal;//normalize(cross(newtv,newbv));
  vec3 normalMapN = normalize(-newtv*normalColor.x-newbv*normalColor.y+newn*normalColor.z);

  vec3 specularColor = texture(uSpecularTex, vTexCoord).rgb;  
  float glossyColor = texture(uSpecularTex, vTexCoord).a;  

  viewPos = vPos.xyz;
  //viewNormal = encodeNormal(vNormal); 
  viewNormal = normalMapN;
  specular = vec4(specularColor, glossyColor);//vec3(vTexCoord, 1.0);
  glossy = vec3(glossyColor, glossyColor, glossyColor);
  diffuse = texColor;
}
