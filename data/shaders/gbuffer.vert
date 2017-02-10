/*******************************************************************************
 * GBuffer.vert
 *------------------------------------------------------------------------------
 * Calculates previous and current positions, as well as material attributes.
 ******************************************************************************/
// Framebuffer Outputs
#version 430
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec3 tangent;
//layout(location = 4) in vec3 binormal;

out vec4 vPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBiNormal;
out vec2 vTexCoord;

uniform mat4 modelViewMat;
uniform mat4 modelViewProjMat;

void main()
{
  // Calculations
  vPos = modelViewMat*vec4(position, 1.0);
  gl_Position = modelViewProjMat*vec4(position, 1.0);


  vNormal = normalize(modelViewMat*vec4(normal,0.0)).xyz;
  vTangent = normalize(modelViewMat*vec4(tangent,0.0)).xyz;
  vBiNormal = cross(vNormal, vTangent);//normalize(modelViewMat*vec4(binormal,0.0)).xyz;
  vTexCoord = texcoord.xy;  
}
