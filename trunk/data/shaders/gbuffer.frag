/*******************************************************************************
 * GBuffer.frag
 *------------------------------------------------------------------------------
 * Writes final information to the GBuffers.
 ******************************************************************************/
#version 430
//#include "normal.glsl"
layout(binding=0) uniform sampler2D uDiffuseTex;

uniform float alphaThreshold = 0.8;

varying vec4 vPos;
varying vec3 vNormal;
varying vec2 vTexCoord;

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
layout(location = 3) out vec3 texCoord;

void main()
{
  // Translate clip [-1,1] -> homogenous [0,1]
  vec4 texColor = texture(uDiffuseTex, vTexCoord);  
  if (texColor.a < alphaThreshold) discard;
  viewPos = vPos.xyz;
  //viewNormal = encodeNormal(vNormal); 
  viewNormal = vNormal;
  texCoord = vec3(vTexCoord, 1.0);
  diffuse = texColor;
}
